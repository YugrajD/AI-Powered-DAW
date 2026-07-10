#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace aidaw
{
using EntityId = int;

enum class TrackType
{
    audio,
    midi
};

struct MidiNote
{
    int pitch = 60;
    double startBeat = 0.0;
    double lengthBeats = 1.0;
    float velocity = 0.8f;
};

struct Clip
{
    EntityId id = 0;
    juce::String name;
    double startBeat = 0.0;
    double lengthBeats = 4.0;
    bool loopEnabled = true;
    std::vector<MidiNote> notes;
};

struct Track
{
    EntityId id = 0;
    juce::String name;
    TrackType type = TrackType::midi;
    bool muted = false;
    bool soloed = false;
    bool armed = false;
    float gain = 1.0f;
    float pan = 0.0f;
    std::vector<Clip> clips;
};

struct Transport
{
    bool playing = false;
    double bpm = 120.0;
    double positionBeats = 0.0;
};

class Project
{
public:
    Project();

    [[nodiscard]] const juce::String& getName() const noexcept { return name; }
    void setName(juce::String newName);

    [[nodiscard]] Transport& getTransport() noexcept { return transport; }
    [[nodiscard]] const Transport& getTransport() const noexcept { return transport; }

    [[nodiscard]] Track& createTrack(TrackType type, juce::String trackName);
    [[nodiscard]] Clip& createClip(EntityId trackId, juce::String clipName, double startBeat, double lengthBeats);

    [[nodiscard]] Track* findTrack(EntityId trackId) noexcept;
    [[nodiscard]] const Track* findTrack(EntityId trackId) const noexcept;

    [[nodiscard]] const std::vector<Track>& getTracks() const noexcept { return tracks; }

private:
    [[nodiscard]] EntityId allocateId() noexcept;

    juce::String name;
    Transport transport;
    std::vector<Track> tracks;
    EntityId nextId = 1;
};
}
