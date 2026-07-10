#include "MainComponent.h"

#include "../core/ProjectSerializer.h"

MainComponent::MainComponent()
    : audioEngine(project, log)
{
    auto& drums = project.createTrack(aidaw::TrackType::midi, "Drums");
    drums.gain = 0.35f;
    drums.pan = -0.15f;
    [[maybe_unused]] auto& starterClip = project.createClip(drums.id, "Starter Loop", 0.0, 4.0);
    log.info("Created project shell");
    log.info("Added starter MIDI track and clip");
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

    playButton.onClick = [this]
    {
        audioEngine.play();
        log.info("Transport started");
        diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
        refreshStatus();
    };
    addAndMakeVisible(playButton);

    stopButton.onClick = [this]
    {
        audioEngine.stop();
        log.info("Transport stopped");
        diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
        refreshStatus();
    };
    addAndMakeVisible(stopButton);

    metronomeToggle.setToggleState(audioEngine.isMetronomeEnabled(), juce::dontSendNotification);
    metronomeToggle.onClick = [this]
    {
        audioEngine.setMetronomeEnabled(metronomeToggle.getToggleState());
        log.info(juce::String("Metronome ")
                 + (metronomeToggle.getToggleState() ? "enabled" : "disabled"));
        diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
    };
    addAndMakeVisible(metronomeToggle);

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

    auto transportBounds = bounds.removeFromTop(36);
    playButton.setBounds(transportBounds.removeFromLeft(84));
    transportBounds.removeFromLeft(8);
    stopButton.setBounds(transportBounds.removeFromLeft(84));
    transportBounds.removeFromLeft(16);
    metronomeToggle.setBounds(transportBounds.removeFromLeft(140));

    bounds.removeFromTop(16);
    diagnosticsEditor.setBounds(bounds.removeFromBottom(180));
}

void MainComponent::timerCallback()
{
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
