#pragma once

#include "../core/Project.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

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

        struct AudioRegion
        {
            double startBeat = 0.0;
            double endBeat = 0.0;
            double sourceStartSeconds = 0.0;
            float gain = 1.0f;
            double fadeInBeats = 0.0;
            double fadeOutBeats = 0.0;
            double sampleRate = 0.0;
            juce::AudioBuffer<float> audio;
        };

        EntityId id = 0;
        InstrumentType instrument = InstrumentType::sineSynth;
        float gain = 1.0f;
        float pan = 0.0f;
        std::vector<AutomationPoint> gainAutomation;
        std::vector<AutomationPoint> panAutomation;
        std::vector<SequencedNote> notes;
        std::vector<AudioRegion> audioRegions;
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
