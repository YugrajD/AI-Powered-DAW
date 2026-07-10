#pragma once

#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class PianoRollView final : public juce::Component
{
public:
    explicit PianoRollView(aidaw::Project& project);

    std::function<void()> onEdited;

    void setClip(aidaw::EntityId trackId, aidaw::EntityId clipId);
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    [[nodiscard]] const aidaw::Clip* selectedClip() const;
    [[nodiscard]] int pitchFromY(int y) const noexcept;
    [[nodiscard]] double beatFromX(int x) const noexcept;

    aidaw::Project& project;
    aidaw::EntityId selectedTrackId = 0;
    aidaw::EntityId selectedClipId = 0;

    static constexpr int keyboardWidth = 58;
    static constexpr int minPitch = 24;
    static constexpr int maxPitch = 72;
    static constexpr double pixelsPerBeat = 96.0;
};
