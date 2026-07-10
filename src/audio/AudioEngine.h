#pragma once

#include "../core/DiagnosticLog.h"
#include "../core/Project.h"
#include "TrackProcessingGraph.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>
#include <cstdint>

namespace aidaw
{
class AudioEngine final : private juce::AudioIODeviceCallback
{
public:
    AudioEngine(Project& project, DiagnosticLog& log);
    ~AudioEngine() override;

    bool initialise();
    void shutdown();

    void play() noexcept;
    void pause() noexcept;
    void stop() noexcept;
    void setTempo(double bpm) noexcept;
    void setMetronomeEnabled(bool enabled) noexcept;
    void refreshProjectGraph();

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate.load(); }
    [[nodiscard]] int getBlockSize() const noexcept { return blockSize.load(); }
    [[nodiscard]] bool isDeviceOpen() const noexcept { return deviceOpen.load(); }
    [[nodiscard]] bool isPlaying() const noexcept { return playing.load(); }
    [[nodiscard]] bool isMetronomeEnabled() const noexcept { return metronomeEnabled.load(); }
    [[nodiscard]] double getTempo() const noexcept { return tempoBpm.load(); }
    [[nodiscard]] double getPositionBeats() const noexcept { return positionBeats.load(); }
    [[nodiscard]] int64_t getCallbackCount() const noexcept { return callbackCount.load(); }
    [[nodiscard]] int64_t getOverrunCount() const noexcept { return overrunCount.load(); }
    [[nodiscard]] double getLastCallbackMilliseconds() const noexcept;
    [[nodiscard]] double getMaxCallbackMilliseconds() const noexcept;

private:
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void recordCallbackTiming(int64_t startTicks, int numSamples, double currentSampleRate) noexcept;

    DiagnosticLog& log;
    Project& project;
    juce::AudioDeviceManager deviceManager;
    TrackProcessingGraph trackGraph;
    std::atomic<double> sampleRate { 0.0 };
    std::atomic<double> tempoBpm { 120.0 };
    std::atomic<double> positionBeats { 0.0 };
    std::atomic<int> blockSize { 0 };
    std::atomic<bool> deviceOpen { false };
    std::atomic<bool> playing { false };
    std::atomic<bool> metronomeEnabled { true };
    std::atomic<int64_t> callbackCount { 0 };
    std::atomic<int64_t> overrunCount { 0 };
    std::atomic<int64_t> lastCallbackMicros { 0 };
    std::atomic<int64_t> maxCallbackMicros { 0 };

    double nextMetronomeBeat = 0.0;
    double clickPhase = 0.0;
    double clickFrequency = 880.0;
    int clickSamplesRemaining = 0;
    bool wasPlayingLastBlock = false;
};
}
