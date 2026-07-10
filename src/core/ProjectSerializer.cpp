#include "ProjectSerializer.h"

namespace aidaw
{
namespace
{
juce::String trackTypeToString(TrackType type)
{
    return type == TrackType::audio ? "audio" : "midi";
}

TrackType trackTypeFromString(const juce::String& value)
{
    return value == "audio" ? TrackType::audio : TrackType::midi;
}

juce::String instrumentTypeToString(InstrumentType type)
{
    switch (type)
    {
        case InstrumentType::sineSynth:
            return "sineSynth";
        case InstrumentType::subtractiveSynth:
            return "subtractiveSynth";
        case InstrumentType::drumSynth:
            return "drumSynth";
    }

    return "sineSynth";
}

InstrumentType instrumentTypeFromString(const juce::String& value)
{
    if (value == "subtractiveSynth")
        return InstrumentType::subtractiveSynth;

    if (value == "drumSynth")
        return InstrumentType::drumSynth;

    return InstrumentType::sineSynth;
}

juce::String effectTypeToString(EffectType type)
{
    switch (type)
    {
        case EffectType::lowPass:
            return "lowPass";
        case EffectType::delay:
            return "delay";
        case EffectType::saturation:
            return "saturation";
    }

    return "lowPass";
}

EffectType effectTypeFromString(const juce::String& value)
{
    if (value == "delay")
        return EffectType::delay;

    if (value == "saturation")
        return EffectType::saturation;

    return EffectType::lowPass;
}

juce::var noteToVar(const MidiNote& note)
{
    auto object = new juce::DynamicObject();
    object->setProperty("pitch", note.pitch);
    object->setProperty("startBeat", note.startBeat);
    object->setProperty("lengthBeats", note.lengthBeats);
    object->setProperty("velocity", note.velocity);
    return object;
}

juce::var clipToVar(const Clip& clip)
{
    auto object = new juce::DynamicObject();
    object->setProperty("id", clip.id);
    object->setProperty("name", clip.name);
    object->setProperty("startBeat", clip.startBeat);
    object->setProperty("lengthBeats", clip.lengthBeats);
    object->setProperty("loopEnabled", clip.loopEnabled);
    object->setProperty("audioFilePath", clip.audioFilePath);
    object->setProperty("sourceStartSeconds", clip.sourceStartSeconds);
    object->setProperty("clipGain", clip.clipGain);
    object->setProperty("fadeInBeats", clip.fadeInBeats);
    object->setProperty("fadeOutBeats", clip.fadeOutBeats);

    juce::Array<juce::var> notes;
    for (const auto& note : clip.notes)
        notes.add(noteToVar(note));

    object->setProperty("notes", notes);
    return object;
}

juce::var trackToVar(const Track& track)
{
    auto object = new juce::DynamicObject();
    object->setProperty("id", track.id);
    object->setProperty("name", track.name);
    object->setProperty("type", trackTypeToString(track.type));
    object->setProperty("instrument", instrumentTypeToString(track.instrument));
    object->setProperty("muted", track.muted);
    object->setProperty("soloed", track.soloed);
    object->setProperty("armed", track.armed);
    object->setProperty("gain", track.gain);
    object->setProperty("pan", track.pan);

    juce::Array<juce::var> clips;
    for (const auto& clip : track.clips)
        clips.add(clipToVar(clip));

    object->setProperty("clips", clips);

    juce::Array<juce::var> effects;
    for (const auto& effect : track.effects)
    {
        auto effectObject = new juce::DynamicObject();
        effectObject->setProperty("type", effectTypeToString(effect.type));
        effectObject->setProperty("enabled", effect.enabled);
        effectObject->setProperty("amount", effect.amount);
        effects.add(effectObject);
    }

    object->setProperty("effects", effects);
    return object;
}

MidiNote noteFromVar(const juce::var& value)
{
    MidiNote note;
    note.pitch = static_cast<int>(value.getProperty("pitch", 60));
    note.startBeat = static_cast<double>(value.getProperty("startBeat", 0.0));
    note.lengthBeats = static_cast<double>(value.getProperty("lengthBeats", 1.0));
    note.velocity = static_cast<float>(static_cast<double>(value.getProperty("velocity", 0.8)));
    return note;
}
}

juce::var ProjectSerializer::toVar(const Project& project)
{
    auto object = new juce::DynamicObject();
    object->setProperty("schemaVersion", currentSchemaVersion);
    object->setProperty("name", project.getName());

    auto transportObject = new juce::DynamicObject();
    const auto& transport = project.getTransport();
    transportObject->setProperty("playing", transport.playing);
    transportObject->setProperty("bpm", transport.bpm);
    transportObject->setProperty("positionBeats", transport.positionBeats);
    object->setProperty("transport", transportObject);

    juce::Array<juce::var> tracks;
    for (const auto& track : project.getTracks())
        tracks.add(trackToVar(track));

    object->setProperty("tracks", tracks);
    return object;
}

juce::String ProjectSerializer::toJson(const Project& project)
{
    return juce::JSON::toString(toVar(project), true);
}

Project ProjectSerializer::fromJson(const juce::String& json)
{
    auto parsed = juce::JSON::parse(json);
    Project project;
    project.setName(parsed.getProperty("name", "Untitled Project").toString());

    const auto transportValue = parsed.getProperty("transport", juce::var {});
    auto& transport = project.getTransport();
    transport.playing = static_cast<bool>(transportValue.getProperty("playing", false));
    transport.bpm = static_cast<double>(transportValue.getProperty("bpm", 120.0));
    transport.positionBeats = static_cast<double>(transportValue.getProperty("positionBeats", 0.0));

    const auto* tracks = parsed.getProperty("tracks", juce::var {}).getArray();
    if (tracks == nullptr)
        return project;

    for (const auto& trackValue : *tracks)
    {
        auto& track = project.createTrack(
            trackTypeFromString(trackValue.getProperty("type", "midi").toString()),
            trackValue.getProperty("name", "Track").toString());

        track.instrument = instrumentTypeFromString(trackValue.getProperty("instrument", "sineSynth").toString());
        track.muted = static_cast<bool>(trackValue.getProperty("muted", false));
        track.soloed = static_cast<bool>(trackValue.getProperty("soloed", false));
        track.armed = static_cast<bool>(trackValue.getProperty("armed", false));
        track.gain = static_cast<float>(static_cast<double>(trackValue.getProperty("gain", 1.0)));
        track.pan = static_cast<float>(static_cast<double>(trackValue.getProperty("pan", 0.0)));

        const auto* clips = trackValue.getProperty("clips", juce::var {}).getArray();
        if (clips == nullptr)
            clips = nullptr;

        if (clips != nullptr)
        {
            for (const auto& clipValue : *clips)
            {
                auto& clip = project.createClip(track.id,
                                                clipValue.getProperty("name", "Clip").toString(),
                                                static_cast<double>(clipValue.getProperty("startBeat", 0.0)),
                                                static_cast<double>(clipValue.getProperty("lengthBeats", 4.0)));
                clip.loopEnabled = static_cast<bool>(clipValue.getProperty("loopEnabled", true));
                clip.audioFilePath = clipValue.getProperty("audioFilePath", "").toString();
                clip.sourceStartSeconds = static_cast<double>(clipValue.getProperty("sourceStartSeconds", 0.0));
                clip.clipGain = static_cast<float>(static_cast<double>(clipValue.getProperty("clipGain", 1.0)));
                clip.fadeInBeats = static_cast<double>(clipValue.getProperty("fadeInBeats", 0.0));
                clip.fadeOutBeats = static_cast<double>(clipValue.getProperty("fadeOutBeats", 0.0));

                const auto* notes = clipValue.getProperty("notes", juce::var {}).getArray();
                if (notes == nullptr)
                    continue;

                for (const auto& noteValue : *notes)
                    clip.notes.push_back(noteFromVar(noteValue));
            }
        }

        const auto* effects = trackValue.getProperty("effects", juce::var {}).getArray();
        if (effects == nullptr)
            continue;

        for (const auto& effectValue : *effects)
        {
            track.effects.push_back(EffectSlot {
                effectTypeFromString(effectValue.getProperty("type", "lowPass").toString()),
                static_cast<bool>(effectValue.getProperty("enabled", true)),
                static_cast<float>(static_cast<double>(effectValue.getProperty("amount", 0.5))) });
        }
    }

    return project;
}
}
