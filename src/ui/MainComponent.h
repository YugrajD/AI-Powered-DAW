#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class MainComponent final : public juce::Component
{
public:
    MainComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    aidaw::Project project;
    juce::Label titleLabel;
    juce::Label statusLabel;
};
