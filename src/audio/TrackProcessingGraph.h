#pragma once

#include "../core/Project.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <vector>

namespace aidaw
{
class TrackProcessingGraph
{
public:
    void configureFromProject(const Project& project);
    void prepare(double sampleRate, int maxBlockSize, int numOutputChannels);
    void render(juce::AudioBuffer<float>& output, int numSamples) noexcept;

    [[nodiscard]] int getTrackCount() const noexcept { return static_cast<int>(tracks.size()); }

private:
    struct TrackProcessor
    {
        EntityId id = 0;
        float gain = 1.0f;
        float pan = 0.0f;
    };

    std::vector<TrackProcessor> tracks;
    juce::AudioBuffer<float> trackScratch;
    double currentSampleRate = 0.0;
};
}
