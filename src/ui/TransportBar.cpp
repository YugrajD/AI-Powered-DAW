#include "TransportBar.h"

TransportBar::TransportBar(aidaw::AudioEngine& engine)
    : audioEngine(engine)
{
    playButton.onClick = [this]
    {
        if (onPlay)
            onPlay();
    };
    addAndMakeVisible(playButton);

    stopButton.onClick = [this]
    {
        if (onStop)
            onStop();
    };
    addAndMakeVisible(stopButton);

    metronomeToggle.onClick = [this]
    {
        if (onMetronomeChanged)
            onMetronomeChanged(metronomeToggle.getToggleState());
    };
    addAndMakeVisible(metronomeToggle);

    beatLabel.setJustificationType(juce::Justification::centredLeft);
    beatLabel.setFont(juce::FontOptions { 14.0f, juce::Font::bold });
    beatLabel.setColour(juce::Label::textColourId, juce::Colour { 0xffdce3ef });
    addAndMakeVisible(beatLabel);

    refresh();
}

void TransportBar::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour { 0xff202631 });
    graphics.setColour(juce::Colour { 0xff3a4352 });
    graphics.drawRect(getLocalBounds());
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds().reduced(10, 7);
    playButton.setBounds(bounds.removeFromLeft(84));
    bounds.removeFromLeft(8);
    stopButton.setBounds(bounds.removeFromLeft(84));
    bounds.removeFromLeft(14);
    metronomeToggle.setBounds(bounds.removeFromLeft(140));
    bounds.removeFromLeft(14);
    beatLabel.setBounds(bounds);
}

void TransportBar::refresh()
{
    metronomeToggle.setToggleState(audioEngine.isMetronomeEnabled(), juce::dontSendNotification);
    beatLabel.setText((audioEngine.isPlaying() ? "Playing" : "Stopped")
                          + juce::String(" | Beat ")
                          + juce::String(audioEngine.getPositionBeats(), 2)
                          + " | "
                          + juce::String(audioEngine.getTempo(), 1)
                          + " BPM",
                      juce::dontSendNotification);
}
