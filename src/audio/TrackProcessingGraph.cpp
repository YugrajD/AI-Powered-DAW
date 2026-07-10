#include "TrackProcessingGraph.h"

namespace aidaw
{
namespace
{
double midiPitchToFrequency(int pitch) noexcept
{
    return 440.0 * std::pow(2.0, (static_cast<double>(pitch) - 69.0) / 12.0);
}

float renderSine(double phase, float velocity) noexcept
{
    return static_cast<float>(std::sin(phase) * velocity * 0.16);
}

float renderSubtractive(double phase, float velocity, float& lowPassState) noexcept
{
    const auto saw = static_cast<float>((phase / juce::MathConstants<double>::pi) - 1.0);
    const auto folded = std::fmod(saw + 3.0f, 2.0f) - 1.0f;
    lowPassState += 0.08f * ((folded * velocity * 0.24f) - lowPassState);
    return lowPassState;
}

float renderDrum(double seconds, int pitch, float velocity) noexcept
{
    const auto decay = std::exp(-seconds * 18.0);
    if (pitch < 40)
    {
        const auto sweep = 42.0 + (110.0 * std::exp(-seconds * 24.0));
        return static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * sweep * seconds) * decay * velocity * 0.55);
    }

    if (pitch < 46)
    {
        const auto tone = std::sin(juce::MathConstants<double>::twoPi * 190.0 * seconds);
        const auto snap = std::sin(juce::MathConstants<double>::twoPi * 910.0 * seconds)
                          * std::exp(-seconds * 38.0);
        return static_cast<float>(((tone * 0.65) + (snap * 0.35)) * decay * velocity * 0.38);
    }

    const auto metallic = std::sin(juce::MathConstants<double>::twoPi * 6200.0 * seconds)
                          + std::sin(juce::MathConstants<double>::twoPi * 7310.0 * seconds);
    return static_cast<float>(metallic * 0.5 * std::exp(-seconds * 55.0) * velocity * 0.18);
}

float fadeGainForBeat(double beat, double startBeat, double endBeat, double fadeInBeats, double fadeOutBeats) noexcept
{
    auto gain = 1.0;

    if (fadeInBeats > 0.0)
        gain = juce::jmin(gain, juce::jlimit(0.0, 1.0, (beat - startBeat) / fadeInBeats));

    if (fadeOutBeats > 0.0)
        gain = juce::jmin(gain, juce::jlimit(0.0, 1.0, (endBeat - beat) / fadeOutBeats));

    return static_cast<float>(gain);
}
}

void TrackProcessingGraph::configureFromProject(const Project& project)
{
    tracks.clear();
    tracks.reserve(project.getTracks().size());

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    for (size_t index = 0; index < project.getTracks().size(); ++index)
    {
        const auto& track = project.getTracks()[index];
        TrackProcessor processor;
        processor.id = track.id;
        processor.instrument = track.instrument;
        processor.gain = track.gain;
        processor.pan = track.pan;
        processor.effects = track.effects;

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
        else if (track.type == TrackType::audio)
        {
            for (const auto& clip : track.clips)
            {
                if (clip.audioFilePath.isEmpty())
                    continue;

                auto reader = std::unique_ptr<juce::AudioFormatReader>(
                    formatManager.createReaderFor(juce::File(clip.audioFilePath)));
                if (reader == nullptr)
                    continue;

                TrackProcessor::AudioRegion region;
                region.startBeat = clip.startBeat;
                region.endBeat = clip.startBeat + clip.lengthBeats;
                region.sourceStartSeconds = clip.sourceStartSeconds;
                region.gain = clip.clipGain;
                region.fadeInBeats = clip.fadeInBeats;
                region.fadeOutBeats = clip.fadeOutBeats;
                region.sampleRate = reader->sampleRate;
                region.audio.setSize(static_cast<int>(reader->numChannels),
                                     static_cast<int>(reader->lengthInSamples));

                reader->read(&region.audio,
                             0,
                             region.audio.getNumSamples(),
                             0,
                             true,
                             true);

                processor.audioRegions.push_back(std::move(region));
            }
        }

        tracks.push_back(std::move(processor));
    }
}

void TrackProcessingGraph::prepare(double sampleRate, int maxBlockSize, int numOutputChannels)
{
    currentSampleRate = sampleRate;
    currentOutputChannels = numOutputChannels;
    trackScratch.setSize(numOutputChannels, maxBlockSize, false, false, true);
    trackScratch.clear();

    const auto maxDelaySamples = juce::jmax(1, static_cast<int>(sampleRate * 0.5));
    for (auto& track : tracks)
    {
        track.effectLowPassState.assign(static_cast<size_t>(numOutputChannels), 0.0f);
        track.delayBuffer.setSize(numOutputChannels, maxDelaySamples, false, false, true);
        track.delayBuffer.clear();
        track.delayWritePosition = 0;
    }
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
                    switch (track.instrument)
                    {
                        case InstrumentType::sineSynth:
                            sampleValue += renderSine(phase, note.velocity);
                            break;
                        case InstrumentType::subtractiveSynth:
                            sampleValue += renderSubtractive(phase, note.velocity, track.lowPassState);
                            break;
                        case InstrumentType::drumSynth:
                            sampleValue += renderDrum(seconds, note.pitch, note.velocity);
                            break;
                    }
                }

                mono[sample] = juce::jlimit(-0.8f, 0.8f, sampleValue);
            }

            for (int channel = 1; channel < trackScratch.getNumChannels(); ++channel)
                trackScratch.copyFrom(channel, 0, trackScratch, 0, 0, numSamples);
        }

        if (! track.audioRegions.empty() && trackScratch.getNumChannels() > 0)
        {
            const auto secondsPerBeat = (beatsPerSample > 0.0 && currentSampleRate > 0.0)
                                            ? 1.0 / (beatsPerSample * currentSampleRate)
                                            : 0.5;

            for (int sample = 0; sample < numSamples; ++sample)
            {
                const auto beat = startBeat + (static_cast<double>(sample) * beatsPerSample);

                for (const auto& region : track.audioRegions)
                {
                    if (beat < region.startBeat || beat >= region.endBeat || region.sampleRate <= 0.0)
                        continue;

                    const auto localSeconds = (beat - region.startBeat) * secondsPerBeat;
                    const auto sourceSeconds = region.sourceStartSeconds + localSeconds;
                    const auto sourceSample = static_cast<int>(sourceSeconds * region.sampleRate);

                    if (sourceSample < 0 || sourceSample >= region.audio.getNumSamples())
                        continue;

                    const auto fadeGain = fadeGainForBeat(beat,
                                                          region.startBeat,
                                                          region.endBeat,
                                                          region.fadeInBeats,
                                                          region.fadeOutBeats);
                    for (int channel = 0; channel < trackScratch.getNumChannels(); ++channel)
                    {
                        const auto sourceChannel = juce::jmin(channel, region.audio.getNumChannels() - 1);
                        auto* channelData = trackScratch.getWritePointer(channel);
                        channelData[sample] += region.audio.getSample(sourceChannel, sourceSample)
                                               * region.gain
                                               * fadeGain;
                    }
                }
            }
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

        for (const auto& effect : track.effects)
        {
            if (! effect.enabled)
                continue;

            const auto amount = juce::jlimit(0.0f, 1.0f, effect.amount);
            switch (effect.type)
            {
                case EffectType::lowPass:
                {
                    const auto coefficient = juce::jlimit(0.01f, 0.35f, 0.02f + (amount * 0.18f));
                    for (int channel = 0; channel < trackScratch.getNumChannels(); ++channel)
                    {
                        auto* data = trackScratch.getWritePointer(channel);
                        auto state = track.effectLowPassState[static_cast<size_t>(channel)];
                        for (int sample = 0; sample < numSamples; ++sample)
                        {
                            state += coefficient * (data[sample] - state);
                            data[sample] = state;
                        }
                        track.effectLowPassState[static_cast<size_t>(channel)] = state;
                    }
                    break;
                }

                case EffectType::saturation:
                {
                    const auto drive = 1.0f + (amount * 8.0f);
                    const auto trim = 1.0f / std::tanh(drive);
                    for (int channel = 0; channel < trackScratch.getNumChannels(); ++channel)
                    {
                        auto* data = trackScratch.getWritePointer(channel);
                        for (int sample = 0; sample < numSamples; ++sample)
                            data[sample] = std::tanh(data[sample] * drive) * trim * 0.85f;
                    }
                    break;
                }

                case EffectType::delay:
                {
                    if (track.delayBuffer.getNumSamples() <= 1)
                        break;

                    const auto delaySamples = juce::jlimit(1,
                                                           track.delayBuffer.getNumSamples() - 1,
                                                           static_cast<int>(currentSampleRate * (0.08 + (amount * 0.22))));
                    const auto feedback = 0.18f + (amount * 0.34f);
                    const auto wet = 0.12f + (amount * 0.28f);

                    for (int sample = 0; sample < numSamples; ++sample)
                    {
                        const auto readPosition = (track.delayWritePosition + track.delayBuffer.getNumSamples() - delaySamples)
                                                  % track.delayBuffer.getNumSamples();

                        for (int channel = 0; channel < trackScratch.getNumChannels(); ++channel)
                        {
                            auto* data = trackScratch.getWritePointer(channel);
                            auto* delayData = track.delayBuffer.getWritePointer(channel);
                            const auto delayed = delayData[readPosition];
                            const auto input = data[sample];
                            data[sample] = input + (delayed * wet);
                            delayData[track.delayWritePosition] = input + (delayed * feedback);
                        }

                        track.delayWritePosition = (track.delayWritePosition + 1) % track.delayBuffer.getNumSamples();
                    }
                    break;
                }
            }
        }

        for (int channel = 0; channel < output.getNumChannels(); ++channel)
            output.addFrom(channel, 0, trackScratch, channel, 0, numSamples);
    }
}
}
