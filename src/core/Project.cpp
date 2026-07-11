#include "Project.h"

namespace aidaw
{
Project::Project()
    : name("Untitled Project")
{
}

void Project::setName(juce::String newName)
{
    name = newName.isNotEmpty() ? std::move(newName) : juce::String("Untitled Project");
}

Track& Project::createTrack(TrackType type, juce::String trackName)
{
    auto& track = tracks.emplace_back();
    track.id = allocateId();
    track.type = type;
    track.name = trackName.isNotEmpty() ? std::move(trackName) : juce::String("Track");
    return track;
}

void Project::setTrackInstrument(EntityId trackId, InstrumentType instrument)
{
    if (auto* track = findTrack(trackId))
        track->instrument = instrument;
}

bool Project::addTrackEffect(EntityId trackId, EffectType effect, float amount)
{
    auto* track = findTrack(trackId);
    if (track == nullptr)
        return false;

    track->effects.push_back(EffectSlot { effect, true, juce::jlimit(0.0f, 1.0f, amount) });
    return true;
}

bool Project::addAutomationPoint(EntityId trackId, AutomationTarget target, double beat, float value)
{
    auto* track = findTrack(trackId);
    if (track == nullptr)
        return false;

    auto lane = std::find_if(track->automation.begin(),
                             track->automation.end(),
                             [target](const AutomationLane& candidate)
                             {
                                 return candidate.target == target;
                             });

    if (lane == track->automation.end())
    {
        track->automation.push_back(AutomationLane { target, {} });
        lane = std::prev(track->automation.end());
    }

    lane->points.push_back(AutomationPoint { juce::jmax(0.0, beat), value });
    std::sort(lane->points.begin(),
              lane->points.end(),
              [](const AutomationPoint& left, const AutomationPoint& right)
              {
                  return left.beat < right.beat;
              });
    return true;
}

float Project::getAutomationValue(EntityId trackId,
                                  AutomationTarget target,
                                  double beat,
                                  float fallback) const noexcept
{
    const auto* track = findTrack(trackId);
    if (track == nullptr)
        return fallback;

    const auto lane = std::find_if(track->automation.begin(),
                                   track->automation.end(),
                                   [target](const AutomationLane& candidate)
                                   {
                                       return candidate.target == target;
                                   });

    if (lane == track->automation.end() || lane->points.empty())
        return fallback;

    if (beat <= lane->points.front().beat)
        return lane->points.front().value;

    if (beat >= lane->points.back().beat)
        return lane->points.back().value;

    for (size_t index = 1; index < lane->points.size(); ++index)
    {
        const auto& right = lane->points[index];
        if (beat > right.beat)
            continue;

        const auto& left = lane->points[index - 1];
        const auto span = right.beat - left.beat;
        if (span <= 0.0)
            return right.value;

        const auto alpha = static_cast<float>((beat - left.beat) / span);
        return left.value + ((right.value - left.value) * alpha);
    }

    return fallback;
}

Clip& Project::createClip(EntityId trackId, juce::String clipName, double startBeat, double lengthBeats)
{
    auto* track = findTrack(trackId);
    jassert(track != nullptr);

    auto& clip = track->clips.emplace_back();
    clip.id = allocateId();
    clip.name = clipName.isNotEmpty() ? std::move(clipName) : juce::String("Clip");
    clip.startBeat = juce::jmax(0.0, startBeat);
    clip.lengthBeats = juce::jmax(0.25, lengthBeats);
    return clip;
}

Clip& Project::createAudioClip(EntityId trackId,
                               juce::String clipName,
                               juce::File audioFile,
                               double startBeat,
                               double lengthBeats)
{
    auto& clip = createClip(trackId, std::move(clipName), startBeat, lengthBeats);
    clip.loopEnabled = false;
    clip.audioFilePath = audioFile.getFullPathName();
    clip.clipGain = 1.0f;
    clip.sourceStartSeconds = 0.0;
    clip.fadeInBeats = 0.05;
    clip.fadeOutBeats = 0.05;
    return clip;
}

MidiNote& Project::addMidiNote(EntityId trackId,
                               EntityId clipId,
                               int pitch,
                               double startBeat,
                               double lengthBeats,
                               float velocity)
{
    auto* clip = findClip(trackId, clipId);
    jassert(clip != nullptr);

    auto& note = clip->notes.emplace_back();
    note.pitch = juce::jlimit(0, 127, pitch);
    note.startBeat = juce::jlimit(0.0, clip->lengthBeats, startBeat);
    note.lengthBeats = juce::jlimit(0.03125, clip->lengthBeats, lengthBeats);
    note.velocity = juce::jlimit(0.0f, 1.0f, velocity);
    return note;
}

Clip& Project::duplicateClip(EntityId trackId, EntityId clipId, double newStartBeat)
{
    auto* track = findTrack(trackId);
    auto* source = findClip(trackId, clipId);
    jassert(track != nullptr);
    jassert(source != nullptr);

    auto duplicated = *source;
    duplicated.id = allocateId();
    duplicated.name = source->name + " Copy";
    duplicated.startBeat = juce::jmax(0.0, newStartBeat);
    track->clips.push_back(std::move(duplicated));
    return track->clips.back();
}

bool Project::quantizeClip(EntityId trackId, EntityId clipId, double gridBeats)
{
    auto* clip = findClip(trackId, clipId);
    if (clip == nullptr || gridBeats <= 0.0)
        return false;

    for (auto& note : clip->notes)
        note.startBeat = std::round(note.startBeat / gridBeats) * gridBeats;

    return true;
}

bool Project::transposeClip(EntityId trackId, EntityId clipId, int semitones)
{
    auto* clip = findClip(trackId, clipId);
    if (clip == nullptr)
        return false;

    for (auto& note : clip->notes)
        note.pitch = juce::jlimit(0, 127, note.pitch + semitones);

    return true;
}

Track* Project::findTrack(EntityId trackId) noexcept
{
    for (auto& track : tracks)
        if (track.id == trackId)
            return &track;

    return nullptr;
}

const Track* Project::findTrack(EntityId trackId) const noexcept
{
    for (const auto& track : tracks)
        if (track.id == trackId)
            return &track;

    return nullptr;
}

Clip* Project::findClip(EntityId trackId, EntityId clipId) noexcept
{
    auto* track = findTrack(trackId);
    if (track == nullptr)
        return nullptr;

    for (auto& clip : track->clips)
        if (clip.id == clipId)
            return &clip;

    return nullptr;
}

const Clip* Project::findClip(EntityId trackId, EntityId clipId) const noexcept
{
    const auto* track = findTrack(trackId);
    if (track == nullptr)
        return nullptr;

    for (const auto& clip : track->clips)
        if (clip.id == clipId)
            return &clip;

    return nullptr;
}

EntityId Project::allocateId() noexcept
{
    return nextId++;
}
}
