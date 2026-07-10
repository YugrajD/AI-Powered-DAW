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

    void play() noexcept;
    void pause() noexcept;
    void stop() noexcept;
    void setTempo(double bpm) noexcept;

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate.load(); }
    [[nodiscard]] int getBlockSize() const noexcept { return blockSize.load(); }
    [[nodiscard]] bool isDeviceOpen() const noexcept { return deviceOpen.load(); }
    [[nodiscard]] bool isPlaying() const noexcept { return playing.load(); }
    [[nodiscard]] double getTempo() const noexcept { return tempoBpm.load(); }
    [[nodiscard]] double getPositionBeats() const noexcept { return positionBeats.load(); }

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
    std::atomic<double> tempoBpm { 120.0 };
    std::atomic<double> positionBeats { 0.0 };
    std::atomic<int> blockSize { 0 };
    std::atomic<bool> deviceOpen { false };
    std::atomic<bool> playing { false };
};
}
