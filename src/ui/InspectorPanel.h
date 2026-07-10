#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class InspectorPanel final : public juce::Component
{
public:
    explicit InspectorPanel(aidaw::Project& project);

    void setSelection(aidaw::EntityId trackId, aidaw::EntityId clipId);
    void paint(juce::Graphics& graphics) override;

private:
    [[nodiscard]] const aidaw::Track* selectedTrack() const;
    [[nodiscard]] const aidaw::Clip* selectedClip() const;

    aidaw::Project& project;
    aidaw::EntityId selectedTrackId = 0;
    aidaw::EntityId selectedClipId = 0;
};
