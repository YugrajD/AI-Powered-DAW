#include "core/Project.h"
#include "core/ProjectFileService.h"
#include "core/ProjectSerializer.h"
#include "audio/OfflineRenderer.h"
#include "audio/TrackProcessingGraph.h"
#include "commands/CommandExecutor.h"

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
    track.instrument = aidaw::InstrumentType::subtractiveSynth;
    track.gain = 0.75f;
    track.pan = -0.2f;
    project.addTrackEffect(track.id, aidaw::EffectType::saturation, 0.4f);
    project.addAutomationPoint(track.id, aidaw::AutomationTarget::trackGain, 0.0, 0.25f);
    project.addAutomationPoint(track.id, aidaw::AutomationTarget::trackGain, 4.0, 0.75f);

    auto& clip = project.createClip(track.id, "Bass Loop", 8.0, 4.0);
    clip.notes.push_back(aidaw::MidiNote { 36, 0.0, 0.5, 0.9f });
    clip.notes.push_back(aidaw::MidiNote { 43, 1.0, 0.5, 0.8f });

    auto& audioTrack = project.createTrack(aidaw::TrackType::audio, "Loop");
    auto& audioClip = project.createAudioClip(audioTrack.id,
                                              "Break",
                                              juce::File("C:/Samples/break.wav"),
                                              0.0,
                                              8.0);
    audioClip.clipGain = 0.65f;
    audioClip.fadeInBeats = 0.25;
    audioClip.fadeOutBeats = 0.5;
    return project;
}

void testProjectModel()
{
    auto project = makeProject();

    expect(project.getName() == "Serialization Test", "project name is stored");
    expect(project.getTracks().size() == 2, "tracks are created");
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
    expect(restoredTrack.instrument == aidaw::InstrumentType::subtractiveSynth, "instrument round-trips");
    expect(restoredTrack.effects.size() == 1, "effect count round-trips");
    expect(restoredTrack.effects.front().type == aidaw::EffectType::saturation, "effect type round-trips");
    expect(restoredTrack.automation.size() == 1, "automation lane count round-trips");
    expect(restored.getAutomationValue(restoredTrack.id, aidaw::AutomationTarget::trackGain, 2.0, 1.0f) == 0.5f,
           "automation interpolates after round-trip");
    expect(restoredTrack.clips.size() == 1, "clip count round-trips");
    expect(restoredTrack.clips.front().notes.size() == 2, "note count round-trips");
    expect(restoredTrack.clips.front().notes.front().pitch == 36, "note pitch round-trips");

    const auto& restoredAudioTrack = restored.getTracks().back();
    expect(restoredAudioTrack.type == aidaw::TrackType::audio, "audio track type round-trips");
    expect(restoredAudioTrack.clips.front().audioFilePath.endsWith("break.wav"), "audio file path round-trips");
    expect(restoredAudioTrack.clips.front().clipGain == 0.65f, "audio clip gain round-trips");
    expect(restoredAudioTrack.clips.front().fadeOutBeats == 0.5, "audio clip fade round-trips");
}

void testProjectFileService()
{
    auto source = makeProject();
    const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                               .getChildFile("aidaw_project_service_test");
    directory.createDirectory();
    const auto file = directory.getChildFile("service-test.aidaw");
    file.deleteFile();

    juce::String error;
    expect(aidaw::ProjectFileService::saveProject(source, file, &error), "project file saves");
    expect(file.existsAsFile(), "project file exists after save");

    aidaw::Project restored;
    expect(aidaw::ProjectFileService::loadProject(file, restored, &error), "project file loads");
    expect(restored.getName() == source.getName(), "loaded project name matches");

    expect(aidaw::ProjectFileService::saveBackup(restored, file, &error), "project backup saves");
    expect(file.getParentDirectory().getChildFile("Backups").isDirectory(), "backup directory exists");
}

void testMidiClipEditingHelpers()
{
    aidaw::Project project;
    auto& track = project.createTrack(aidaw::TrackType::midi, "Lead");
    auto& clip = project.createClip(track.id, "Phrase", 0.0, 4.0);
    [[maybe_unused]] auto& note = project.addMidiNote(track.id, clip.id, 60, 0.13, 0.5, 0.7f);

    expect(project.findClip(track.id, clip.id) != nullptr, "clip lookup works");
    expect(project.quantizeClip(track.id, clip.id, 0.25), "clip quantize succeeds");
    expect(clip.notes.front().startBeat == 0.25, "note start quantizes to grid");

    expect(project.transposeClip(track.id, clip.id, 12), "clip transpose succeeds");
    expect(clip.notes.front().pitch == 72, "note pitch transposes");

    const auto originalClipId = clip.id;
    const auto expectedNoteCount = clip.notes.size();
    auto& duplicate = project.duplicateClip(track.id, clip.id, 4.0);
    expect(duplicate.id != originalClipId, "duplicate receives new id");
    expect(duplicate.startBeat == 4.0, "duplicate uses requested start beat");
    expect(duplicate.notes.size() == expectedNoteCount, "duplicate copies notes");
}

float channelAbsSum(const juce::AudioBuffer<float>& buffer, int channel)
{
    float sum = 0.0f;
    const auto* data = buffer.getReadPointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        sum += std::abs(data[sample]);

    return sum;
}

void testTrackProcessingGraphGainPan()
{
    aidaw::Project project;
    auto& track = project.createTrack(aidaw::TrackType::midi, "Diagnostic Tone");
    track.gain = 0.5f;
    track.pan = -1.0f;
    auto& clip = project.createClip(track.id, "Note", 0.0, 4.0);
    [[maybe_unused]] auto& note = project.addMidiNote(track.id, clip.id, 48, 0.0, 4.0, 1.0f);

    aidaw::TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(44100.0, 256, 2);

    juce::AudioBuffer<float> output(2, 256);
    graph.render(output, output.getNumSamples(), 0.0, 120.0 / 60.0 / 44100.0);

    expect(channelAbsSum(output, 0) > 0.1f, "left channel receives diagnostic tone");
    expect(channelAbsSum(output, 1) < 0.0001f, "right channel is muted by hard-left pan");
}

void testTrackProcessingGraphInstrumentModes()
{
    aidaw::Project project;
    auto& sineTrack = project.createTrack(aidaw::TrackType::midi, "Sine");
    sineTrack.instrument = aidaw::InstrumentType::sineSynth;
    auto& sineClip = project.createClip(sineTrack.id, "Sine Note", 0.0, 4.0);
    [[maybe_unused]] auto& sineNote = project.addMidiNote(sineTrack.id, sineClip.id, 60, 0.0, 1.0, 1.0f);

    auto& drumTrack = project.createTrack(aidaw::TrackType::midi, "Drums");
    drumTrack.instrument = aidaw::InstrumentType::drumSynth;
    auto& drumClip = project.createClip(drumTrack.id, "Kick", 0.0, 4.0);
    [[maybe_unused]] auto& kick = project.addMidiNote(drumTrack.id, drumClip.id, 36, 0.0, 0.25, 1.0f);

    aidaw::TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(44100.0, 512, 2);

    juce::AudioBuffer<float> output(2, 512);
    graph.render(output, output.getNumSamples(), 0.0, 120.0 / 60.0 / 44100.0);

    expect(channelAbsSum(output, 0) > 0.1f, "instrument modes render non-silent output");
}

void testTrackProcessingGraphEffects()
{
    aidaw::Project project;
    auto& track = project.createTrack(aidaw::TrackType::midi, "Effected");
    track.instrument = aidaw::InstrumentType::subtractiveSynth;
    project.addTrackEffect(track.id, aidaw::EffectType::lowPass, 0.5f);
    project.addTrackEffect(track.id, aidaw::EffectType::saturation, 0.25f);
    project.addTrackEffect(track.id, aidaw::EffectType::delay, 0.4f);

    auto& clip = project.createClip(track.id, "Tone", 0.0, 4.0);
    [[maybe_unused]] auto& note = project.addMidiNote(track.id, clip.id, 48, 0.0, 1.0, 1.0f);

    aidaw::TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(44100.0, 1024, 2);

    juce::AudioBuffer<float> output(2, 1024);
    graph.render(output, output.getNumSamples(), 0.0, 120.0 / 60.0 / 44100.0);

    expect(channelAbsSum(output, 0) > 0.1f, "effect chain keeps output audible");
}

juce::File writeTestWav()
{
    auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                    .getChildFile("aidaw_audio_clip_test.wav");
    file.deleteFile();

    juce::WavAudioFormat wavFormat;
    auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream());
    auto writer = std::unique_ptr<juce::AudioFormatWriter>(
        wavFormat.createWriterFor(stream.get(), 44100.0, 1, 16, {}, 0));

    if (writer == nullptr)
        return {};

    stream.release();

    juce::AudioBuffer<float> buffer(1, 44100);
    auto* data = buffer.getWritePointer(0);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto phase = juce::MathConstants<double>::twoPi * 220.0 * static_cast<double>(sample) / 44100.0;
        data[sample] = static_cast<float>(std::sin(phase) * 0.4);
    }

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return file;
}

void testTrackProcessingGraphAudioClipRender()
{
    auto wavFile = writeTestWav();
    expect(wavFile.existsAsFile(), "test wav is written");

    aidaw::Project project;
    auto& track = project.createTrack(aidaw::TrackType::audio, "Audio");
    track.gain = 0.8f;
    [[maybe_unused]] auto& clip = project.createAudioClip(track.id, "Imported", wavFile, 0.0, 2.0);

    aidaw::TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(44100.0, 512, 2);

    juce::AudioBuffer<float> output(2, 512);
    graph.render(output, output.getNumSamples(), 0.1, 120.0 / 60.0 / 44100.0);

    expect(channelAbsSum(output, 0) > 0.1f, "audio clip renders non-silent output");
    expect(channelAbsSum(output, 1) > 0.1f, "mono audio clip is copied to right channel");

    wavFile.deleteFile();
}

void testTrackProcessingGraphAutomation()
{
    aidaw::Project project;
    auto& track = project.createTrack(aidaw::TrackType::midi, "Automated");
    track.gain = 0.0f;
    project.addAutomationPoint(track.id, aidaw::AutomationTarget::trackGain, 0.0, 0.0f);
    project.addAutomationPoint(track.id, aidaw::AutomationTarget::trackGain, 4.0, 1.0f);

    auto& clip = project.createClip(track.id, "Tone", 0.0, 4.0);
    [[maybe_unused]] auto& note = project.addMidiNote(track.id, clip.id, 48, 0.0, 4.0, 1.0f);

    aidaw::TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(44100.0, 512, 2);

    juce::AudioBuffer<float> early(2, 512);
    graph.render(early, early.getNumSamples(), 0.0, 120.0 / 60.0 / 44100.0);

    juce::AudioBuffer<float> late(2, 512);
    graph.render(late, late.getNumSamples(), 3.0, 120.0 / 60.0 / 44100.0);

    expect(channelAbsSum(late, 0) > channelAbsSum(early, 0), "gain automation increases rendered level");
}

void testOfflineRenderer()
{
    aidaw::Project project;
    project.getTransport().bpm = 120.0;
    auto& track = project.createTrack(aidaw::TrackType::midi, "Render Tone");
    auto& clip = project.createClip(track.id, "Tone", 0.0, 4.0);
    [[maybe_unused]] auto& note = project.addMidiNote(track.id, clip.id, 48, 0.0, 4.0, 1.0f);

    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                          .getChildFile("aidaw_offline_render_test.wav");
    file.deleteFile();

    aidaw::OfflineRenderer::Options options;
    options.lengthBeats = 1.0;
    options.blockSize = 256;

    juce::String error;
    expect(aidaw::OfflineRenderer::renderToWav(project, file, options, &error), "offline render succeeds");
    expect(file.existsAsFile(), "offline render file exists");
    expect(file.getSize() > 1000, "offline render file has audio data");
    file.deleteFile();
}

void testCommandExecutor()
{
    aidaw::Project project;

    auto createTrack = aidaw::CommandExecutor::executeJson(project, R"({"type":"create_track","trackType":"midi","name":"AI Bass"})");
    expect(createTrack.ok, "create_track command succeeds");
    const auto trackId = static_cast<int>(createTrack.data.getProperty("id", 0));

    auto createClip = aidaw::CommandExecutor::executeJson(
        project,
        "{\"type\":\"create_midi_clip\",\"trackId\":" + juce::String(trackId) + ",\"name\":\"AI Clip\",\"startBeat\":0,\"lengthBeats\":4}");
    expect(createClip.ok, "create_midi_clip command succeeds");
    const auto clipId = static_cast<int>(createClip.data.getProperty("id", 0));

    auto addNotes = aidaw::CommandExecutor::executeJson(
        project,
        "{\"type\":\"add_midi_notes\",\"trackId\":"
            + juce::String(trackId)
            + ",\"clipId\":"
            + juce::String(clipId)
            + ",\"notes\":[{\"pitch\":48,\"startBeat\":0,\"lengthBeats\":1,\"velocity\":0.9}]}");
    expect(addNotes.ok, "add_midi_notes command succeeds");

    auto setGain = aidaw::CommandExecutor::executeJson(
        project,
        "{\"type\":\"set_track_gain\",\"trackId\":" + juce::String(trackId) + ",\"gain\":0.5}");
    expect(setGain.ok, "set_track_gain command succeeds");

    const auto* track = project.findTrack(trackId);
    expect(track != nullptr && track->gain == 0.5f, "command mutates track gain");
    expect(project.findClip(trackId, clipId)->notes.size() == 1, "command adds MIDI note");

    auto summary = aidaw::CommandExecutor::executeJson(project, R"({"type":"summarize_project"})");
    expect(summary.ok && summary.data.toString().contains("AI Bass"), "summarize_project returns track context");
    expect(aidaw::CommandExecutor::toolManifestJson().contains("create_track"), "tool manifest lists command tools");

    aidaw::CommandHistory history;
    auto badCommand = aidaw::CommandExecutor::executeJson(project, R"({"type":"add_midi_notes","trackId":999,"clipId":1,"notes":[]})", history);
    expect(! badCommand.ok, "history command captures validation failure");
    expect(history.entries().size() == 1, "command history records entry");
    expect(history.toDisplayString().contains("FAIL"), "command history display includes status");
}
}

int main()
{
    testProjectModel();
    testProjectSerializationRoundTrip();
    testProjectFileService();
    testMidiClipEditingHelpers();
    testTrackProcessingGraphGainPan();
    testTrackProcessingGraphInstrumentModes();
    testTrackProcessingGraphEffects();
    testTrackProcessingGraphAudioClipRender();
    testTrackProcessingGraphAutomation();
    testOfflineRenderer();
    testCommandExecutor();

    if (failures != 0)
        return 1;

    std::cout << "All model tests passed\n";
    return 0;
}
