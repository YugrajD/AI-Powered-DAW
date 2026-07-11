#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class AutomationLaneView final : public juce::Component
{
public:
    explicit AutomationLaneView(aidaw::Project& project);

    std::function<void()> onEdited;

    void setTrack(aidaw::EntityId trackId);
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    [[nodiscard]] const aidaw::AutomationLane* gainLane() const;

    aidaw::Project& project;
    aidaw::EntityId selectedTrackId = 0;

    static constexpr double pixelsPerBeat = 96.0;
};
