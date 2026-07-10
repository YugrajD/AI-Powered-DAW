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
    void render(juce::AudioBuffer<float>& output,
                int numSamples,
                double startBeat,
                double beatsPerSample) noexcept;

    [[nodiscard]] int getTrackCount() const noexcept { return static_cast<int>(tracks.size()); }

private:
    struct TrackProcessor
    {
        struct SequencedNote
        {
            int pitch = 60;
            double startBeat = 0.0;
            double endBeat = 1.0;
            float velocity = 0.8f;
        };

        EntityId id = 0;
        InstrumentType instrument = InstrumentType::sineSynth;
        float gain = 1.0f;
        float pan = 0.0f;
        std::vector<SequencedNote> notes;
        std::vector<EffectSlot> effects;
        float lowPassState = 0.0f;
        std::vector<float> effectLowPassState;
        juce::AudioBuffer<float> delayBuffer;
        int delayWritePosition = 0;
    };

    std::vector<TrackProcessor> tracks;
    juce::AudioBuffer<float> trackScratch;
    double currentSampleRate = 0.0;
    int currentOutputChannels = 0;
};
}
