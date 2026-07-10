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

EntityId Project::allocateId() noexcept
{
    return nextId++;
}
}
