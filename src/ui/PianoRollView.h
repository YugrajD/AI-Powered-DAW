#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class PianoRollView final : public juce::Component
{
public:
    explicit PianoRollView(aidaw::Project& project);

    void setClip(aidaw::EntityId trackId, aidaw::EntityId clipId);
    void paint(juce::Graphics& graphics) override;

private:
    [[nodiscard]] const aidaw::Clip* selectedClip() const;

    aidaw::Project& project;
    aidaw::EntityId selectedTrackId = 0;
    aidaw::EntityId selectedClipId = 0;

    static constexpr int keyboardWidth = 58;
    static constexpr int minPitch = 24;
    static constexpr int maxPitch = 72;
    static constexpr double pixelsPerBeat = 96.0;
};
