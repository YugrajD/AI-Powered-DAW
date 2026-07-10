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

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;

    sampleRate.store(device->getCurrentSampleRate());
    blockSize.store(device->getCurrentBufferSizeSamples());
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
    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (auto* output = outputChannelData[channel])
            juce::FloatVectorOperations::clear(output, numSamples);

    juce::AudioBuffer<float> outputBuffer(outputChannelData, numOutputChannels, numSamples);
    trackGraph.render(outputBuffer, numSamples);

    const auto currentSampleRate = sampleRate.load();
    const auto currentlyPlaying = playing.load();

    if (! currentlyPlaying || currentSampleRate <= 0.0)
    {
        wasPlayingLastBlock = false;
        return;
    }

    auto currentPosition = positionBeats.load();
    const auto beatsPerSample = tempoBpm.load() / 60.0 / currentSampleRate;

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
}
}
