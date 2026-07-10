#include "TrackProcessingGraph.h"

namespace aidaw
{
namespace
{
double midiPitchToFrequency(int pitch) noexcept
{
    return 440.0 * std::pow(2.0, (static_cast<double>(pitch) - 69.0) / 12.0);
}
}

void TrackProcessingGraph::configureFromProject(const Project& project)
{
    tracks.clear();
    tracks.reserve(project.getTracks().size());

    for (size_t index = 0; index < project.getTracks().size(); ++index)
    {
        const auto& track = project.getTracks()[index];
        TrackProcessor processor;
        processor.id = track.id;
        processor.gain = track.gain;
        processor.pan = track.pan;

        if (track.type == TrackType::midi)
        {
            for (const auto& clip : track.clips)
            {
                for (const auto& note : clip.notes)
                {
                    if (clip.loopEnabled)
                    {
                        for (double loopStart = clip.startBeat; loopStart < clip.startBeat + 64.0; loopStart += clip.lengthBeats)
                        {
                            processor.notes.push_back(TrackProcessor::SequencedNote {
                                note.pitch,
                                loopStart + note.startBeat,
                                loopStart + note.startBeat + note.lengthBeats,
                                note.velocity });
                        }
                    }
                    else
                    {
                        processor.notes.push_back(TrackProcessor::SequencedNote {
                            note.pitch,
                            clip.startBeat + note.startBeat,
                            clip.startBeat + note.startBeat + note.lengthBeats,
                            note.velocity });
                    }
                }
            }
        }

        tracks.push_back(std::move(processor));
    }
}

void TrackProcessingGraph::prepare(double sampleRate, int maxBlockSize, int numOutputChannels)
{
    currentSampleRate = sampleRate;
    trackScratch.setSize(numOutputChannels, maxBlockSize, false, false, true);
    trackScratch.clear();
}

void TrackProcessingGraph::render(juce::AudioBuffer<float>& output,
                                  int numSamples,
                                  double startBeat,
                                  double beatsPerSample) noexcept
{
    output.clear();

    if (currentSampleRate <= 0.0 || tracks.empty() || trackScratch.getNumSamples() < numSamples)
        return;

    for (auto& track : tracks)
    {
        trackScratch.clear(0, numSamples);

        if (trackScratch.getNumChannels() > 0)
        {
            const auto secondsPerBeat = (beatsPerSample > 0.0 && currentSampleRate > 0.0)
                                            ? 1.0 / (beatsPerSample * currentSampleRate)
                                            : 0.5;
            auto* mono = trackScratch.getWritePointer(0);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                const auto beat = startBeat + (static_cast<double>(sample) * beatsPerSample);
                float sampleValue = 0.0f;

                for (const auto& note : track.notes)
                {
                    if (beat < note.startBeat || beat >= note.endBeat)
                        continue;

                    const auto noteBeat = beat - note.startBeat;
                    const auto seconds = noteBeat * secondsPerBeat;
                    const auto phase = juce::MathConstants<double>::twoPi
                                       * midiPitchToFrequency(note.pitch)
                                       * seconds;
                    sampleValue += static_cast<float>(std::sin(phase) * note.velocity * 0.16);
                }

                mono[sample] = juce::jlimit(-0.8f, 0.8f, sampleValue);
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
