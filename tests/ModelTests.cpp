#include "core/Project.h"
#include "core/ProjectSerializer.h"

#include <iostream>
#include <string_view>

namespace
{
int failures = 0;

void expect(bool condition, std::string_view message)
{
    if (condition)
        return;

    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

aidaw::Project makeProject()
{
    aidaw::Project project;
    project.setName("Serialization Test");
    project.getTransport().bpm = 128.0;
    project.getTransport().positionBeats = 16.0;

    auto& track = project.createTrack(aidaw::TrackType::midi, "Bass");
    track.gain = 0.75f;
    track.pan = -0.2f;

    auto& clip = project.createClip(track.id, "Bass Loop", 8.0, 4.0);
    clip.notes.push_back(aidaw::MidiNote { 36, 0.0, 0.5, 0.9f });
    clip.notes.push_back(aidaw::MidiNote { 43, 1.0, 0.5, 0.8f });
    return project;
}

void testProjectModel()
{
    auto project = makeProject();

    expect(project.getName() == "Serialization Test", "project name is stored");
    expect(project.getTracks().size() == 1, "track is created");
    expect(project.getTracks().front().clips.size() == 1, "clip is created");
    expect(project.findTrack(project.getTracks().front().id) != nullptr, "track lookup works");
    expect(project.getTracks().front().clips.front().notes.size() == 2, "notes are stored");
}

void testProjectSerializationRoundTrip()
{
    auto source = makeProject();
    const auto json = aidaw::ProjectSerializer::toJson(source);
    const auto restored = aidaw::ProjectSerializer::fromJson(json);

    expect(json.contains("\"schemaVersion\": 1"), "schema version is written");
    expect(restored.getName() == source.getName(), "project name round-trips");
    expect(restored.getTransport().bpm == source.getTransport().bpm, "transport bpm round-trips");
    expect(restored.getTracks().size() == source.getTracks().size(), "track count round-trips");

    const auto& restoredTrack = restored.getTracks().front();
    expect(restoredTrack.name == "Bass", "track name round-trips");
    expect(restoredTrack.type == aidaw::TrackType::midi, "track type round-trips");
    expect(restoredTrack.clips.size() == 1, "clip count round-trips");
    expect(restoredTrack.clips.front().notes.size() == 2, "note count round-trips");
    expect(restoredTrack.clips.front().notes.front().pitch == 36, "note pitch round-trips");
}
}

int main()
{
    testProjectModel();
    testProjectSerializationRoundTrip();

    if (failures != 0)
        return 1;

    std::cout << "All model tests passed\n";
    return 0;
}
