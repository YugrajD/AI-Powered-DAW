#include "MainComponent.h"

#include "../core/ProjectSerializer.h"

MainComponent::MainComponent()
    : audioEngine(log)
{
    auto& drums = project.createTrack(aidaw::TrackType::midi, "Drums");
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

    statusLabel.setText("Stage 1: "
                            + project.getName()
                            + " | "
                            + juce::String(project.getTracks().size())
                            + " track | audio "
                            + (audioEngine.isDeviceOpen() ? "online" : "offline"),
                        juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::FontOptions { 15.0f });
    addAndMakeVisible(statusLabel);

    diagnosticsEditor.setMultiLine(true);
    diagnosticsEditor.setReadOnly(true);
    diagnosticsEditor.setScrollbarsShown(true);
    diagnosticsEditor.setCaretVisible(false);
    diagnosticsEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour { 0xff101217 });
    diagnosticsEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour { 0xff3a4250 });
    diagnosticsEditor.setColour(juce::TextEditor::textColourId, juce::Colour { 0xffd9dde7 });
    diagnosticsEditor.setText(log.toDisplayString(), juce::dontSendNotification);
    addAndMakeVisible(diagnosticsEditor);

    setSize(1200, 760);
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
    bounds.removeFromTop(16);
    diagnosticsEditor.setBounds(bounds.removeFromBottom(180));
}
