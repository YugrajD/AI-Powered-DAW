#pragma once

#include "../core/DiagnosticLog.h"
#include "../core/Project.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>

namespace aidaw
{
class AudioEngine final : private juce::AudioIODeviceCallback
{
public:
    explicit AudioEngine(DiagnosticLog& log);
    ~AudioEngine() override;

    bool initialise();
    void shutdown();

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate.load(); }
    [[nodiscard]] int getBlockSize() const noexcept { return blockSize.load(); }
    [[nodiscard]] bool isDeviceOpen() const noexcept { return deviceOpen.load(); }

private:
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;

    DiagnosticLog& log;
    juce::AudioDeviceManager deviceManager;
    std::atomic<double> sampleRate { 0.0 };
    std::atomic<int> blockSize { 0 };
    std::atomic<bool> deviceOpen { false };
};
}
