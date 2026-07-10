#include "TrackProcessingGraph.h"

namespace aidaw
{
void TrackProcessingGraph::configureFromProject(const Project& project)
{
    tracks.clear();
    tracks.reserve(project.getTracks().size());

    for (size_t index = 0; index < project.getTracks().size(); ++index)
    {
        const auto& track = project.getTracks()[index];
        tracks.push_back(TrackProcessor {
            track.id,
            track.gain,
            track.pan,
            110.0 + (55.0 * static_cast<double>(index)),
            0.0 });
    }
}

void TrackProcessingGraph::prepare(double sampleRate, int maxBlockSize, int numOutputChannels)
{
    currentSampleRate = sampleRate;
    trackScratch.setSize(numOutputChannels, maxBlockSize, false, false, true);
    trackScratch.clear();
}

void TrackProcessingGraph::render(juce::AudioBuffer<float>& output, int numSamples) noexcept
{
    output.clear();

    if (currentSampleRate <= 0.0 || tracks.empty() || trackScratch.getNumSamples() < numSamples)
        return;

    for (auto& track : tracks)
    {
        trackScratch.clear(0, numSamples);

        if (trackScratch.getNumChannels() > 0)
        {
            auto* mono = trackScratch.getWritePointer(0);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                mono[sample] = static_cast<float>(std::sin(track.phase) * 0.12);
                track.phase += juce::MathConstants<double>::twoPi * track.frequency / currentSampleRate;

                if (track.phase >= juce::MathConstants<double>::twoPi)
                    track.phase -= juce::MathConstants<double>::twoPi;
            }

            for (int channel = 1; channel < trackScratch.getNumChannels(); ++channel)
                trackScratch.copyFrom(channel, 0, trackScratch, 0, 0, numSamples);
        }

        const auto clampedPan = juce::jlimit(-1.0f, 1.0f, track.pan);
        const auto leftGain = track.gain * (clampedPan <= 0.0f ? 1.0f : 1.0f - clampedPan);
        const auto rightGain = track.gain * (clampedPan >= 0.0f ? 1.0f : 1.0f + clampedPan);

        if (trackScratch.getNumChannels() == 1)
        {
            trackScratch.applyGain(0, 0, numSamples, track.gain);
        }
        else if (trackScratch.getNumChannels() >= 2)
        {
            trackScratch.applyGain(0, 0, numSamples, leftGain);
            trackScratch.applyGain(1, 0, numSamples, rightGain);

            for (int channel = 2; channel < trackScratch.getNumChannels(); ++channel)
                trackScratch.applyGain(channel, 0, numSamples, track.gain);
        }

        for (int channel = 0; channel < output.getNumChannels(); ++channel)
            output.addFrom(channel, 0, trackScratch, channel, 0, numSamples);
    }
}
}
