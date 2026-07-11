#include "AutomationLaneView.h"

AutomationLaneView::AutomationLaneView(aidaw::Project& projectToDisplay)
    : project(projectToDisplay)
{
}

void AutomationLaneView::setTrack(aidaw::EntityId trackId)
{
    selectedTrackId = trackId;
    repaint();
}

void AutomationLaneView::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour { 0xff151b24 });
    auto bounds = getLocalBounds().reduced(10);

    graphics.setColour(juce::Colour { 0xffd7dee9 });
    graphics.setFont(juce::FontOptions { 13.0f, juce::Font::bold });
    graphics.drawText("Gain Automation", bounds.removeFromTop(20), juce::Justification::centredLeft);

    auto laneBounds = bounds.reduced(0, 6);
    graphics.setColour(juce::Colour { 0xff303847 });
    graphics.drawRect(laneBounds);

    for (int beat = 0; beat <= 16; ++beat)
    {
        const auto x = laneBounds.getX() + static_cast<int>(std::round(static_cast<double>(beat) * pixelsPerBeat));
        if (x > laneBounds.getRight())
            break;

        graphics.setColour(beat % 4 == 0 ? juce::Colour { 0xff465164 } : juce::Colour { 0xff2a313e });
        graphics.drawVerticalLine(x, static_cast<float>(laneBounds.getY()), static_cast<float>(laneBounds.getBottom()));
    }

    const auto* lane = gainLane();
    if (lane == nullptr || lane->points.empty())
    {
        graphics.setColour(juce::Colour { 0xff8793a6 });
        graphics.drawText("Click to add gain automation points", laneBounds, juce::Justification::centred);
        return;
    }

    juce::Path path;
    bool started = false;

    for (const auto& point : lane->points)
    {
        const auto x = static_cast<float>(laneBounds.getX() + (point.beat * pixelsPerBeat));
        const auto y = static_cast<float>(laneBounds.getBottom())
                       - (juce::jlimit(0.0f, 1.25f, point.value) / 1.25f * static_cast<float>(laneBounds.getHeight()));

        if (! started)
        {
            path.startNewSubPath(x, y);
            started = true;
        }
        else
        {
            path.lineTo(x, y);
        }

        graphics.setColour(juce::Colour { 0xffffcf6d });
        graphics.fillEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f);
    }

    graphics.setColour(juce::Colour { 0xffffcf6d });
    graphics.strokePath(path, juce::PathStrokeType { 2.0f });
}

void AutomationLaneView::mouseDown(const juce::MouseEvent& event)
{
    auto laneBounds = getLocalBounds().reduced(10).withTrimmedTop(26).reduced(0, 6);
    if (! laneBounds.contains(event.position.toInt()))
        return;

    const auto beat = juce::jmax(0.0, static_cast<double>(event.x - laneBounds.getX()) / pixelsPerBeat);
    const auto normalized = 1.0f - (static_cast<float>(event.y - laneBounds.getY()) / static_cast<float>(laneBounds.getHeight()));
    const auto value = juce::jlimit(0.0f, 1.25f, normalized * 1.25f);

    project.addAutomationPoint(selectedTrackId, aidaw::AutomationTarget::trackGain, beat, value);
    repaint();

    if (onEdited)
        onEdited();
}

const aidaw::AutomationLane* AutomationLaneView::gainLane() const
{
    const auto* track = project.findTrack(selectedTrackId);
    if (track == nullptr)
        return nullptr;

    for (const auto& lane : track->automation)
        if (lane.target == aidaw::AutomationTarget::trackGain)
            return &lane;

    return nullptr;
}
