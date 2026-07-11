#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class MixerPanel final : public juce::Component
{
public:
    explicit MixerPanel(aidaw::Project& project);

    std::function<void()> onMixerChanged;

    void refreshFromProject();
    void resized() override;
    void paint(juce::Graphics& graphics) override;

private:
    struct ChannelStrip
    {
        aidaw::EntityId trackId = 0;
        juce::Label nameLabel;
        juce::Slider gainSlider;
        juce::Slider panSlider;
    };

    void rebuildStrips();

    aidaw::Project& project;
    std::vector<std::unique_ptr<ChannelStrip>> strips;
};
