#include "MixerPanel.h"

MixerPanel::MixerPanel(aidaw::Project& projectToMix)
    : project(projectToMix)
{
    refreshFromProject();
}

void MixerPanel::refreshFromProject()
{
    if (strips.size() != project.getTracks().size())
        rebuildStrips();

    for (auto& strip : strips)
    {
        if (const auto* track = project.findTrack(strip->trackId))
        {
            strip->nameLabel.setText(track->name, juce::dontSendNotification);
            strip->gainSlider.setValue(track->gain, juce::dontSendNotification);
            strip->panSlider.setValue(track->pan, juce::dontSendNotification);
        }
    }
}

void MixerPanel::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour { 0xff171d26 });
    graphics.setColour(juce::Colour { 0xff3a4352 });
    graphics.drawRect(getLocalBounds());
}

void MixerPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    const auto stripWidth = 112;

    for (auto& strip : strips)
    {
        auto stripBounds = bounds.removeFromLeft(stripWidth).reduced(6, 4);
        strip->nameLabel.setBounds(stripBounds.removeFromTop(24));
        stripBounds.removeFromTop(6);
        strip->gainSlider.setBounds(stripBounds.removeFromTop(88));
        stripBounds.removeFromTop(8);
        strip->panSlider.setBounds(stripBounds.removeFromTop(54));
    }
}

void MixerPanel::rebuildStrips()
{
    for (auto& strip : strips)
    {
        removeChildComponent(&strip->nameLabel);
        removeChildComponent(&strip->gainSlider);
        removeChildComponent(&strip->panSlider);
    }

    strips.clear();

    for (const auto& track : project.getTracks())
    {
        auto strip = std::make_unique<ChannelStrip>();
        strip->trackId = track.id;

        strip->nameLabel.setJustificationType(juce::Justification::centred);
        strip->nameLabel.setFont(juce::FontOptions { 12.0f, juce::Font::bold });
        strip->nameLabel.setColour(juce::Label::textColourId, juce::Colour { 0xffe2e8f2 });
        addAndMakeVisible(strip->nameLabel);

        strip->gainSlider.setSliderStyle(juce::Slider::LinearVertical);
        strip->gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        strip->gainSlider.setRange(0.0, 1.25, 0.01);
        strip->gainSlider.onValueChange = [this, trackId = strip->trackId, slider = &strip->gainSlider]
        {
            if (auto* track = project.findTrack(trackId))
                track->gain = static_cast<float>(slider->getValue());

            if (onMixerChanged)
                onMixerChanged();
        };
        addAndMakeVisible(strip->gainSlider);

        strip->panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        strip->panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        strip->panSlider.setRange(-1.0, 1.0, 0.01);
        strip->panSlider.onValueChange = [this, trackId = strip->trackId, slider = &strip->panSlider]
        {
            if (auto* track = project.findTrack(trackId))
                track->pan = static_cast<float>(slider->getValue());

            if (onMixerChanged)
                onMixerChanged();
        };
        addAndMakeVisible(strip->panSlider);

        strips.push_back(std::move(strip));
    }

    resized();
}
