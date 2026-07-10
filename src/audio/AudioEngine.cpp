#include "AudioEngine.h"

namespace aidaw
{
AudioEngine::AudioEngine(DiagnosticLog& diagnosticLog)
    : log(diagnosticLog)
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::initialise()
{
    const auto result = deviceManager.initialiseWithDefaultDevices(0, 2);
    if (result.isNotEmpty())
    {
        log.error("Audio device init failed: " + result);
        return false;
    }

    deviceManager.addAudioCallback(this);
    log.info("Audio device manager initialized");
    return true;
}

void AudioEngine::shutdown()
{
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
    deviceOpen.store(false);
    sampleRate.store(0.0);
    blockSize.store(0);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;

    sampleRate.store(device->getCurrentSampleRate());
    blockSize.store(device->getCurrentBufferSizeSamples());
    deviceOpen.store(true);
}

void AudioEngine::audioDeviceStopped()
{
    deviceOpen.store(false);
    sampleRate.store(0.0);
    blockSize.store(0);
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
}
}
