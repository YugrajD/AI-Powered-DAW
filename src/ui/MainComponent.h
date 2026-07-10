#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class MainComponent final : public juce::Component
{
public:
    MainComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Label statusLabel;
};
