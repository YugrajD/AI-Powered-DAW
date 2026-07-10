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

enum class InstrumentType
{
    sineSynth,
    subtractiveSynth,
    drumSynth
};

enum class EffectType
{
    lowPass,
    delay,
    saturation
};

struct EffectSlot
{
    EffectType type = EffectType::lowPass;
    bool enabled = true;
    float amount = 0.5f;
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
    juce::String audioFilePath;
    double sourceStartSeconds = 0.0;
    float clipGain = 1.0f;
    double fadeInBeats = 0.0;
    double fadeOutBeats = 0.0;
    std::vector<MidiNote> notes;
};

struct Track
{
    EntityId id = 0;
    juce::String name;
    TrackType type = TrackType::midi;
    InstrumentType instrument = InstrumentType::sineSynth;
    bool muted = false;
    bool soloed = false;
    bool armed = false;
    float gain = 1.0f;
    float pan = 0.0f;
    std::vector<Clip> clips;
    std::vector<EffectSlot> effects;
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
    void setTrackInstrument(EntityId trackId, InstrumentType instrument);
    bool addTrackEffect(EntityId trackId, EffectType effect, float amount);
    [[nodiscard]] Clip& createClip(EntityId trackId, juce::String clipName, double startBeat, double lengthBeats);
    [[nodiscard]] Clip& createAudioClip(EntityId trackId,
                                        juce::String clipName,
                                        juce::File audioFile,
                                        double startBeat,
                                        double lengthBeats);
    [[nodiscard]] MidiNote& addMidiNote(EntityId trackId,
                                        EntityId clipId,
                                        int pitch,
                                        double startBeat,
                                        double lengthBeats,
                                        float velocity);
    [[nodiscard]] Clip& duplicateClip(EntityId trackId, EntityId clipId, double newStartBeat);

    bool quantizeClip(EntityId trackId, EntityId clipId, double gridBeats);
    bool transposeClip(EntityId trackId, EntityId clipId, int semitones);

    [[nodiscard]] Track* findTrack(EntityId trackId) noexcept;
    [[nodiscard]] const Track* findTrack(EntityId trackId) const noexcept;
    [[nodiscard]] Clip* findClip(EntityId trackId, EntityId clipId) noexcept;
    [[nodiscard]] const Clip* findClip(EntityId trackId, EntityId clipId) const noexcept;

    [[nodiscard]] const std::vector<Track>& getTracks() const noexcept { return tracks; }

private:
    [[nodiscard]] EntityId allocateId() noexcept;

    juce::String name;
    Transport transport;
    std::vector<Track> tracks;
    EntityId nextId = 1;
};
}
