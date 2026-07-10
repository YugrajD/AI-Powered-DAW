#include "AudioEngine.h"

namespace aidaw
{
AudioEngine::AudioEngine(Project& projectToRender, DiagnosticLog& diagnosticLog)
    : log(diagnosticLog),
      project(projectToRender)
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::initialise()
{
    trackGraph.configureFromProject(project);

    const auto result = deviceManager.initialiseWithDefaultDevices(0, 2);
    if (result.isNotEmpty())
    {
        log.error("Audio device init failed: " + result);
        return false;
    }

    deviceManager.addAudioCallback(this);
    log.info("Audio device manager initialized with "
             + juce::String(trackGraph.getTrackCount())
             + " track processor(s)");
    return true;
}

void AudioEngine::shutdown()
{
    playing.store(false);
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
    deviceOpen.store(false);
    sampleRate.store(0.0);
    blockSize.store(0);
}

void AudioEngine::play() noexcept
{
    playing.store(true);
}

void AudioEngine::pause() noexcept
{
    playing.store(false);
}

void AudioEngine::stop() noexcept
{
    playing.store(false);
    positionBeats.store(0.0);
}

void AudioEngine::setTempo(double bpm) noexcept
{
    tempoBpm.store(juce::jlimit(40.0, 240.0, bpm));
}

void AudioEngine::setMetronomeEnabled(bool enabled) noexcept
{
    metronomeEnabled.store(enabled);
}

void AudioEngine::refreshProjectGraph()
{
    pause();
    trackGraph.configureFromProject(project);
    if (sampleRate.load() > 0.0 && blockSize.load() > 0)
        trackGraph.prepare(sampleRate.load(), blockSize.load(), 2);
    log.info("Audio graph refreshed from project MIDI");
}

double AudioEngine::getLastCallbackMilliseconds() const noexcept
{
    return static_cast<double>(lastCallbackMicros.load()) / 1000.0;
}

double AudioEngine::getMaxCallbackMilliseconds() const noexcept
{
    return static_cast<double>(maxCallbackMicros.load()) / 1000.0;
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;

    sampleRate.store(device->getCurrentSampleRate());
    blockSize.store(device->getCurrentBufferSizeSamples());
    callbackCount.store(0);
    overrunCount.store(0);
    lastCallbackMicros.store(0);
    maxCallbackMicros.store(0);
    trackGraph.prepare(sampleRate.load(), blockSize.load(), device->getActiveOutputChannels().countNumberOfSetBits());
    deviceOpen.store(true);
}

void AudioEngine::audioDeviceStopped()
{
    deviceOpen.store(false);
    sampleRate.store(0.0);
    blockSize.store(0);
    wasPlayingLastBlock = false;
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*,
                                                   int,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext&)
{
    const auto callbackStartTicks = juce::Time::getHighResolutionTicks();

    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (auto* output = outputChannelData[channel])
            juce::FloatVectorOperations::clear(output, numSamples);

    const auto currentSampleRate = sampleRate.load();
    const auto currentlyPlaying = playing.load();

    if (! currentlyPlaying || currentSampleRate <= 0.0)
    {
        wasPlayingLastBlock = false;
        recordCallbackTiming(callbackStartTicks, numSamples, currentSampleRate);
        return;
    }

    auto currentPosition = positionBeats.load();
    const auto beatsPerSample = tempoBpm.load() / 60.0 / currentSampleRate;
    juce::AudioBuffer<float> outputBuffer(outputChannelData, numOutputChannels, numSamples);
    trackGraph.render(outputBuffer, numSamples, currentPosition, beatsPerSample);

    if (! wasPlayingLastBlock)
    {
        nextMetronomeBeat = std::ceil(currentPosition);
        if (juce::approximatelyEqual(nextMetronomeBeat, currentPosition))
            nextMetronomeBeat = currentPosition;

        wasPlayingLastBlock = true;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (metronomeEnabled.load() && currentPosition >= nextMetronomeBeat)
        {
            const auto beatNumber = static_cast<int>(std::floor(nextMetronomeBeat));
            clickFrequency = (beatNumber % 4 == 0) ? 1320.0 : 880.0;
            clickSamplesRemaining = static_cast<int>(currentSampleRate * 0.035);
            clickPhase = 0.0;
            nextMetronomeBeat += 1.0;
        }

        float clickSample = 0.0f;
        if (clickSamplesRemaining > 0)
        {
            const auto envelope = static_cast<double>(clickSamplesRemaining)
                                  / juce::jmax(1.0, currentSampleRate * 0.035);
            clickSample = static_cast<float>(std::sin(clickPhase) * envelope * 0.25);
            clickPhase += juce::MathConstants<double>::twoPi * clickFrequency / currentSampleRate;
            --clickSamplesRemaining;
        }

        for (int channel = 0; channel < numOutputChannels; ++channel)
            if (auto* output = outputChannelData[channel])
                output[sample] += clickSample;

        currentPosition += beatsPerSample;
    }

    positionBeats.store(currentPosition);
    recordCallbackTiming(callbackStartTicks, numSamples, currentSampleRate);
}

void AudioEngine::recordCallbackTiming(int64_t startTicks, int numSamples, double currentSampleRate) noexcept
{
    const auto elapsedSeconds = juce::Time::highResolutionTicksToSeconds(
        juce::Time::getHighResolutionTicks() - startTicks);
    const auto elapsedMicros = static_cast<int64_t>(elapsedSeconds * 1000000.0);

    callbackCount.fetch_add(1);
    lastCallbackMicros.store(elapsedMicros);

    auto previousMax = maxCallbackMicros.load();
    while (elapsedMicros > previousMax
           && ! maxCallbackMicros.compare_exchange_weak(previousMax, elapsedMicros))
    {
    }

    if (currentSampleRate > 0.0)
    {
        const auto budgetMicros = (static_cast<double>(numSamples) / currentSampleRate) * 1000000.0;
        if (static_cast<double>(elapsedMicros) > budgetMicros * 0.8)
            overrunCount.fetch_add(1);
    }
}
}
