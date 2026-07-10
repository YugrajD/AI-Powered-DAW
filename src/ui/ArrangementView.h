#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class ArrangementView final : public juce::Component
{
public:
    explicit ArrangementView(aidaw::Project& project);

    void paint(juce::Graphics& graphics) override;

private:
    aidaw::Project& project;

    static constexpr int trackHeaderWidth = 164;
    static constexpr int trackHeight = 72;
    static constexpr int rulerHeight = 30;
    static constexpr double pixelsPerBeat = 56.0;
};
