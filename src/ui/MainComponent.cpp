#include "MainComponent.h"

MainComponent::MainComponent()
{
    auto& drums = project.createTrack(aidaw::TrackType::midi, "Drums");
    project.createClip(drums.id, "Starter Loop", 0.0, 4.0);

    titleLabel.setText("AI Powered DAW", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions { 28.0f, juce::Font::bold });
    addAndMakeVisible(titleLabel);

    statusLabel.setText("Stage 0: "
                            + project.getName()
                            + " | "
                            + juce::String(project.getTracks().size())
                            + " track",
                        juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::FontOptions { 15.0f });
    addAndMakeVisible(statusLabel);

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
}
