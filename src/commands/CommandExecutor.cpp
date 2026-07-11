#include "CommandExecutor.h"

namespace aidaw
{
void CommandHistory::record(juce::String commandJson, juce::String preview, CommandResult result)
{
    history.push_back(CommandHistoryEntry {
        std::move(commandJson),
        std::move(preview),
        std::move(result),
        juce::Time::getCurrentTime() });
}

juce::String CommandHistory::toDisplayString() const
{
    juce::String text;

    for (const auto& entry : history)
    {
        text << entry.timestamp.formatted("%H:%M:%S")
             << " "
             << (entry.result.ok ? "OK" : "FAIL")
             << " | "
             << entry.preview
             << " -> "
             << entry.result.message
             << "\n";
    }

    return text;
}

namespace
{
CommandResult fail(juce::String message)
{
    return CommandResult { false, std::move(message), {} };
}

CommandResult ok(juce::String message, juce::var data = {})
{
    return CommandResult { true, std::move(message), std::move(data) };
}

TrackType trackTypeFromString(const juce::String& value)
{
    return value == "audio" ? TrackType::audio : TrackType::midi;
}

EffectType effectTypeFromString(const juce::String& value)
{
    if (value == "delay")
        return EffectType::delay;

    if (value == "saturation")
        return EffectType::saturation;

    return EffectType::lowPass;
}

AutomationTarget automationTargetFromString(const juce::String& value)
{
    return value == "trackPan" ? AutomationTarget::trackPan : AutomationTarget::trackGain;
}

juce::String requireString(const juce::var& object, const juce::Identifier& key, const juce::String& fallback = {})
{
    return object.getProperty(key, fallback).toString();
}

double requireDouble(const juce::var& object, const juce::Identifier& key, double fallback)
{
    return static_cast<double>(object.getProperty(key, fallback));
}

int requireInt(const juce::var& object, const juce::Identifier& key, int fallback)
{
    return static_cast<int>(object.getProperty(key, fallback));
}

float requireFloat(const juce::var& object, const juce::Identifier& key, float fallback)
{
    return static_cast<float>(static_cast<double>(object.getProperty(key, fallback)));
}

CommandResult validateCommand(const Project& project, const juce::var& command)
{
    const auto type = requireString(command, "type");
    if (type.isEmpty())
        return fail("Command is missing type");

    if (type == "create_track")
        return ok("Will create " + requireString(command, "trackType", "midi") + " track");

    if (type == "create_midi_clip")
    {
        const auto trackId = requireInt(command, "trackId", 0);
        const auto* track = project.findTrack(trackId);
        if (track == nullptr)
            return fail("Track does not exist");

        if (track->type != TrackType::midi)
            return fail("Target track is not MIDI");

        return ok("Will create MIDI clip on track " + juce::String(trackId));
    }

    if (type == "add_midi_notes")
    {
        const auto trackId = requireInt(command, "trackId", 0);
        const auto clipId = requireInt(command, "clipId", 0);
        if (project.findClip(trackId, clipId) == nullptr)
            return fail("Target clip does not exist");

        const auto* notes = command.getProperty("notes", juce::var {}).getArray();
        if (notes == nullptr || notes->isEmpty())
            return fail("No notes provided");

        return ok("Will add " + juce::String(notes->size()) + " MIDI note(s)");
    }

    if (type == "set_track_gain")
        return project.findTrack(requireInt(command, "trackId", 0)) != nullptr ? ok("Will set track gain") : fail("Track does not exist");

    if (type == "add_effect")
        return project.findTrack(requireInt(command, "trackId", 0)) != nullptr ? ok("Will add track effect") : fail("Track does not exist");

    if (type == "add_automation_point")
        return project.findTrack(requireInt(command, "trackId", 0)) != nullptr ? ok("Will add automation point") : fail("Track does not exist");

    if (type == "summarize_project")
        return ok("Will summarize project");

    return fail("Unknown command type: " + type);
}

juce::var makeIdData(EntityId id)
{
    auto object = new juce::DynamicObject();
    object->setProperty("id", id);
    return object;
}
}

CommandResult CommandExecutor::previewJson(const Project& project, const juce::String& json)
{
    const auto parsed = juce::JSON::parse(json);
    if (parsed.isVoid())
        return fail("Command JSON is invalid");

    return validateCommand(project, parsed);
}

CommandResult CommandExecutor::executeJson(Project& project, const juce::String& json)
{
    const auto parsed = juce::JSON::parse(json);
    if (parsed.isVoid())
        return fail("Command JSON is invalid");

    const auto validation = validateCommand(project, parsed);
    if (! validation.ok)
        return validation;

    const auto type = requireString(parsed, "type");

    if (type == "create_track")
    {
        auto& track = project.createTrack(trackTypeFromString(requireString(parsed, "trackType", "midi")),
                                          requireString(parsed, "name", "AI Track"));
        return ok("Created track " + track.name, makeIdData(track.id));
    }

    if (type == "create_midi_clip")
    {
        auto& clip = project.createClip(requireInt(parsed, "trackId", 0),
                                        requireString(parsed, "name", "AI Clip"),
                                        requireDouble(parsed, "startBeat", 0.0),
                                        requireDouble(parsed, "lengthBeats", 4.0));
        return ok("Created MIDI clip " + clip.name, makeIdData(clip.id));
    }

    if (type == "add_midi_notes")
    {
        const auto trackId = requireInt(parsed, "trackId", 0);
        const auto clipId = requireInt(parsed, "clipId", 0);
        const auto* notes = parsed.getProperty("notes", juce::var {}).getArray();
        jassert(notes != nullptr);

        for (const auto& note : *notes)
        {
            [[maybe_unused]] auto& addedNote = project.addMidiNote(trackId,
                                                                    clipId,
                                                                    requireInt(note, "pitch", 60),
                                                                    requireDouble(note, "startBeat", 0.0),
                                                                    requireDouble(note, "lengthBeats", 0.25),
                                                                    requireFloat(note, "velocity", 0.8f));
        }

        return ok("Added MIDI notes");
    }

    if (type == "set_track_gain")
    {
        auto* track = project.findTrack(requireInt(parsed, "trackId", 0));
        jassert(track != nullptr);
        track->gain = juce::jlimit(0.0f, 1.25f, requireFloat(parsed, "gain", track->gain));
        return ok("Set track gain");
    }

    if (type == "add_effect")
    {
        project.addTrackEffect(requireInt(parsed, "trackId", 0),
                               effectTypeFromString(requireString(parsed, "effect", "lowPass")),
                               requireFloat(parsed, "amount", 0.5f));
        return ok("Added effect");
    }

    if (type == "add_automation_point")
    {
        project.addAutomationPoint(requireInt(parsed, "trackId", 0),
                                   automationTargetFromString(requireString(parsed, "target", "trackGain")),
                                   requireDouble(parsed, "beat", 0.0),
                                   requireFloat(parsed, "value", 0.0f));
        return ok("Added automation point");
    }

    if (type == "summarize_project")
        return ok("Project summarized", summarizeProject(project));

    return fail("Unknown command type");
}

CommandResult CommandExecutor::executeJson(Project& project, const juce::String& json, CommandHistory& history)
{
    const auto preview = previewJson(project, json);
    auto result = preview.ok ? executeJson(project, json) : preview;
    history.record(json, preview.message, result);
    return result;
}

juce::String CommandExecutor::summarizeProject(const Project& project)
{
    juce::String summary;
    summary << "Project: " << project.getName() << "\n";
    summary << "Tempo: " << juce::String(project.getTransport().bpm, 1) << " BPM\n";
    summary << "Tracks: " << juce::String(project.getTracks().size()) << "\n";

    for (const auto& track : project.getTracks())
    {
        summary << "- [" << track.id << "] " << track.name
                << " (" << (track.type == TrackType::midi ? "MIDI" : "Audio") << ")"
                << ", clips=" << juce::String(track.clips.size())
                << ", effects=" << juce::String(track.effects.size())
                << ", automation=" << juce::String(track.automation.size())
                << "\n";
    }

    return summary;
}

juce::String CommandExecutor::toolManifestJson()
{
    auto root = new juce::DynamicObject();
    root->setProperty("name", "ai_powered_daw");
    root->setProperty("description", "Agent-callable DAW project mutation tools");

    juce::Array<juce::var> tools;
    const auto addTool = [&tools](const juce::String& name, const juce::String& description, const juce::String& schema)
    {
        auto tool = new juce::DynamicObject();
        tool->setProperty("name", name);
        tool->setProperty("description", description);
        tool->setProperty("inputSchema", juce::JSON::parse(schema));
        tools.add(tool);
    };

    addTool("create_track",
            "Create a MIDI or audio track",
            R"({"type":"object","required":["type","trackType","name"],"properties":{"type":{"const":"create_track"},"trackType":{"enum":["midi","audio"]},"name":{"type":"string"}}})");
    addTool("create_midi_clip",
            "Create a MIDI clip on a MIDI track",
            R"({"type":"object","required":["type","trackId","name","startBeat","lengthBeats"],"properties":{"type":{"const":"create_midi_clip"},"trackId":{"type":"integer"},"name":{"type":"string"},"startBeat":{"type":"number"},"lengthBeats":{"type":"number"}}})");
    addTool("add_midi_notes",
            "Add notes to an existing MIDI clip",
            R"({"type":"object","required":["type","trackId","clipId","notes"],"properties":{"type":{"const":"add_midi_notes"},"trackId":{"type":"integer"},"clipId":{"type":"integer"},"notes":{"type":"array","items":{"type":"object","required":["pitch","startBeat","lengthBeats","velocity"]}}}})");
    addTool("set_track_gain",
            "Set track gain from 0.0 to 1.25",
            R"({"type":"object","required":["type","trackId","gain"],"properties":{"type":{"const":"set_track_gain"},"trackId":{"type":"integer"},"gain":{"type":"number"}}})");
    addTool("add_effect",
            "Add a built-in effect to a track",
            R"({"type":"object","required":["type","trackId","effect","amount"],"properties":{"type":{"const":"add_effect"},"trackId":{"type":"integer"},"effect":{"enum":["lowPass","saturation","delay"]},"amount":{"type":"number"}}})");
    addTool("add_automation_point",
            "Add a track gain or pan automation point",
            R"({"type":"object","required":["type","trackId","target","beat","value"],"properties":{"type":{"const":"add_automation_point"},"trackId":{"type":"integer"},"target":{"enum":["trackGain","trackPan"]},"beat":{"type":"number"},"value":{"type":"number"}}})");
    addTool("summarize_project",
            "Return a compact text summary of the current project",
            R"({"type":"object","required":["type"],"properties":{"type":{"const":"summarize_project"}}})");

    root->setProperty("tools", tools);
    return juce::JSON::toString(root, true);
}
}
