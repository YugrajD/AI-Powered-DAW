#include "TrackProcessingGraph.h"

namespace aidaw
{
void TrackProcessingGraph::configureFromProject(const Project& project)
{
    tracks.clear();
    tracks.reserve(project.getTracks().size());

    for (const auto& track : project.getTracks())
        tracks.push_back(TrackProcessor { track.id, track.gain, track.pan });
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

    for (const auto& track : tracks)
    {
        juce::ignoreUnused(track);
        trackScratch.clear(0, numSamples);

        for (int channel = 0; channel < output.getNumChannels(); ++channel)
            output.addFrom(channel, 0, trackScratch, channel, 0, numSamples);
    }
}
}
