#include "MainComponent.h"

#include "../core/ProjectSerializer.h"

MainComponent::MainComponent()
    : audioEngine(project, log)
{
    auto& drums = project.createTrack(aidaw::TrackType::midi, "Drums");
    drums.gain = 0.35f;
    drums.pan = -0.15f;
    auto& starterClip = project.createClip(drums.id, "Starter Loop", 0.0, 4.0);
    demoTrackId = drums.id;
    demoClipId = starterClip.id;
    [[maybe_unused]] auto& note1 = project.addMidiNote(demoTrackId, demoClipId, 36, 0.0, 0.25, 1.0f);
    [[maybe_unused]] auto& note2 = project.addMidiNote(demoTrackId, demoClipId, 36, 1.0, 0.25, 0.9f);
    [[maybe_unused]] auto& note3 = project.addMidiNote(demoTrackId, demoClipId, 43, 1.5, 0.25, 0.75f);
    [[maybe_unused]] auto& note4 = project.addMidiNote(demoTrackId, demoClipId, 36, 2.0, 0.25, 1.0f);
    [[maybe_unused]] auto& note5 = project.addMidiNote(demoTrackId, demoClipId, 46, 2.75, 0.25, 0.8f);
    [[maybe_unused]] auto& note6 = project.addMidiNote(demoTrackId, demoClipId, 43, 3.5, 0.25, 0.85f);
    log.info("Created project shell");
    log.info("Added starter MIDI track, clip, and note pattern");
    log.info("Serialized project state: "
             + juce::String(aidaw::ProjectSerializer::toJson(project).length())
             + " characters");
    audioEngine.initialise();

    titleLabel.setText("AI Powered DAW", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions { 28.0f, juce::Font::bold });
    addAndMakeVisible(titleLabel);

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::FontOptions { 15.0f });
    addAndMakeVisible(statusLabel);

    transportBar.onPlay = [this]
    {
        audioEngine.play();
        log.info("Transport started");
        refreshDiagnostics();
        refreshStatus();
    };
    transportBar.onStop = [this]
    {
        audioEngine.stop();
        log.info("Transport stopped");
        refreshDiagnostics();
        refreshStatus();
    };
    transportBar.onMetronomeChanged = [this](bool enabled)
    {
        audioEngine.setMetronomeEnabled(enabled);
        log.info(juce::String("Metronome ") + (enabled ? "enabled" : "disabled"));
        refreshDiagnostics();
    };
    addAndMakeVisible(transportBar);

    addAndMakeVisible(arrangementView);
    pianoRollView.setClip(demoTrackId, demoClipId);
    pianoRollView.onEdited = [this]
    {
        log.info("Added MIDI note from piano roll");
        audioEngine.refreshProjectGraph();
        arrangementView.repaint();
        inspectorPanel.repaint();
        refreshDiagnostics();
    };
    addAndMakeVisible(pianoRollView);
    inspectorPanel.setSelection(demoTrackId, demoClipId);
    addAndMakeVisible(inspectorPanel);

    transposeUpButton.onClick = [this]
    {
        project.transposeClip(demoTrackId, demoClipId, 12);
        audioEngine.refreshProjectGraph();
        refreshDiagnostics();
    };
    addAndMakeVisible(transposeUpButton);

    transposeDownButton.onClick = [this]
    {
        project.transposeClip(demoTrackId, demoClipId, -12);
        audioEngine.refreshProjectGraph();
        refreshDiagnostics();
    };
    addAndMakeVisible(transposeDownButton);

    quantizeButton.onClick = [this]
    {
        project.quantizeClip(demoTrackId, demoClipId, 0.25);
        audioEngine.refreshProjectGraph();
        refreshDiagnostics();
    };
    addAndMakeVisible(quantizeButton);

    duplicateButton.onClick = [this]
    {
        auto& duplicated = project.duplicateClip(demoTrackId, demoClipId, 4.0);
        log.info("Duplicated clip to beat " + juce::String(duplicated.startBeat, 2));
        audioEngine.refreshProjectGraph();
        refreshDiagnostics();
    };
    addAndMakeVisible(duplicateButton);

    diagnosticsEditor.setMultiLine(true);
    diagnosticsEditor.setReadOnly(true);
    diagnosticsEditor.setScrollbarsShown(true);
    diagnosticsEditor.setCaretVisible(false);
    diagnosticsEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour { 0xff101217 });
    diagnosticsEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour { 0xff3a4250 });
    diagnosticsEditor.setColour(juce::TextEditor::textColourId, juce::Colour { 0xffd9dde7 });
    diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
    addAndMakeVisible(diagnosticsEditor);

    refreshStatus();
    startTimerHz(20);
    setSize(1200, 760);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

void MainComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour { 0xff16181d });

    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour { 0xff252a33 });
    graphics.fillRoundedRectangle(bounds.reduced(24.0f), 8.0f);

    graphics.setColour(juce::Colour { 0xff3a4250 });
    graphics.drawRoundedRectangle(bounds.reduced(24.0f), 8.0f, 1.0f);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(48);
    titleLabel.setBounds(bounds.removeFromTop(40));
    statusLabel.setBounds(bounds.removeFromTop(28));
    bounds.removeFromTop(12);

    transportBar.setBounds(bounds.removeFromTop(42));
    bounds.removeFromTop(10);

    auto editBounds = bounds.removeFromTop(34);
    transposeDownButton.setBounds(editBounds.removeFromLeft(64));
    editBounds.removeFromLeft(8);
    transposeUpButton.setBounds(editBounds.removeFromLeft(64));
    editBounds.removeFromLeft(8);
    quantizeButton.setBounds(editBounds.removeFromLeft(96));
    editBounds.removeFromLeft(8);
    duplicateButton.setBounds(editBounds.removeFromLeft(96));

    bounds.removeFromTop(16);
    auto workspace = bounds.removeFromTop(496);
    inspectorPanel.setBounds(workspace.removeFromRight(260));
    workspace.removeFromRight(16);
    arrangementView.setBounds(workspace.removeFromTop(260));
    bounds.removeFromTop(16);
    pianoRollView.setBounds(workspace.removeFromTop(220));
    bounds.removeFromTop(16);
    diagnosticsEditor.setBounds(bounds.removeFromBottom(180));
}

void MainComponent::timerCallback()
{
    transportBar.refresh();
    refreshStatus();
}

void MainComponent::refreshStatus()
{
    statusLabel.setText("Stage 1: "
                            + project.getName()
                            + " | "
                            + juce::String(project.getTracks().size())
                            + " track | audio "
                            + (audioEngine.isDeviceOpen() ? "online" : "offline")
                            + " | "
                            + (audioEngine.isPlaying() ? "playing" : "stopped")
                            + " | beat "
                            + juce::String(audioEngine.getPositionBeats(), 2)
                            + " | "
                            + juce::String(audioEngine.getTempo(), 1)
                            + " BPM | cb "
                            + juce::String(audioEngine.getLastCallbackMilliseconds(), 3)
                            + "/"
                            + juce::String(audioEngine.getMaxCallbackMilliseconds(), 3)
                            + " ms | overruns "
                            + juce::String(audioEngine.getOverrunCount()),
                        juce::dontSendNotification);
}

void MainComponent::refreshDiagnostics()
{
    diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
}
