#include "ArrangementView.h"

namespace
{
juce::Colour panelColour { 0xff1d222b };
juce::Colour headerColour { 0xff252b35 };
juce::Colour gridColour { 0xff343b48 };
juce::Colour strongGridColour { 0xff495263 };
juce::Colour clipColour { 0xff48b6a3 };
juce::Colour clipBorderColour { 0xff9de3d7 };
}

ArrangementView::ArrangementView(aidaw::Project& projectToDisplay)
    : project(projectToDisplay)
{
}

void ArrangementView::paint(juce::Graphics& graphics)
{
    graphics.fillAll(panelColour);

    const auto bounds = getLocalBounds();
    const auto timeline = bounds.withTrimmedLeft(trackHeaderWidth).withTrimmedTop(rulerHeight);

    graphics.setColour(headerColour);
    graphics.fillRect(bounds.withWidth(trackHeaderWidth));
    graphics.fillRect(bounds.withHeight(rulerHeight));

    graphics.setColour(strongGridColour);
    graphics.drawLine(static_cast<float>(trackHeaderWidth), 0.0f,
                      static_cast<float>(trackHeaderWidth), static_cast<float>(bounds.getBottom()));
    graphics.drawLine(0.0f, static_cast<float>(rulerHeight),
                      static_cast<float>(bounds.getRight()), static_cast<float>(rulerHeight));

    const int visibleBeats = juce::jmax(1, static_cast<int>(std::ceil(timeline.getWidth() / pixelsPerBeat)));
    for (int beat = 0; beat <= visibleBeats; ++beat)
    {
        const auto x = trackHeaderWidth + static_cast<int>(std::round(static_cast<double>(beat) * pixelsPerBeat));
        const auto isBar = beat % 4 == 0;
        graphics.setColour(isBar ? strongGridColour : gridColour);
        graphics.drawVerticalLine(x, static_cast<float>(rulerHeight), static_cast<float>(bounds.getBottom()));

        if (isBar)
        {
            graphics.setColour(juce::Colour { 0xffc7cedb });
            graphics.drawText("Bar " + juce::String((beat / 4) + 1),
                              x + 6, 4, 76, rulerHeight - 8,
                              juce::Justification::centredLeft);
        }
    }

    const auto& tracks = project.getTracks();
    for (size_t trackIndex = 0; trackIndex < tracks.size(); ++trackIndex)
    {
        const auto y = rulerHeight + static_cast<int>(trackIndex) * trackHeight;
        const auto row = juce::Rectangle<int> { 0, y, bounds.getWidth(), trackHeight };
        const auto trackHeader = row.withWidth(trackHeaderWidth).reduced(10, 8);
        const auto lane = row.withTrimmedLeft(trackHeaderWidth);

        graphics.setColour(trackIndex % 2 == 0 ? juce::Colour { 0xff202631 } : juce::Colour { 0xff1b2029 });
        graphics.fillRect(row.withTrimmedLeft(trackHeaderWidth));

        graphics.setColour(gridColour);
        graphics.drawHorizontalLine(row.getBottom(), 0.0f, static_cast<float>(bounds.getRight()));

        graphics.setColour(juce::Colour { 0xffe2e7f0 });
        graphics.setFont(juce::FontOptions { 15.0f, juce::Font::bold });
        graphics.drawText(tracks[trackIndex].name, trackHeader, juce::Justification::centredLeft);

        graphics.setFont(juce::FontOptions { 12.0f });
        graphics.setColour(juce::Colour { 0xff9ba6b8 });
        graphics.drawText(tracks[trackIndex].type == aidaw::TrackType::midi ? "MIDI" : "Audio",
                          trackHeader.withTrimmedTop(24),
                          juce::Justification::centredLeft);

        for (const auto& clip : tracks[trackIndex].clips)
        {
            const auto clipX = lane.getX() + static_cast<int>(std::round(clip.startBeat * pixelsPerBeat));
            const auto clipW = juce::jmax(28, static_cast<int>(std::round(clip.lengthBeats * pixelsPerBeat)));
            auto clipRect = juce::Rectangle<int> { clipX, y + 12, clipW, trackHeight - 24 };

            graphics.setColour(clipColour);
            graphics.fillRoundedRectangle(clipRect.toFloat(), 6.0f);
            graphics.setColour(clipBorderColour);
            graphics.drawRoundedRectangle(clipRect.toFloat(), 6.0f, 1.0f);

            graphics.setColour(juce::Colour { 0xff0e2724 });
            graphics.setFont(juce::FontOptions { 13.0f, juce::Font::bold });
            graphics.drawText(clip.name, clipRect.reduced(8, 4), juce::Justification::centredLeft);
        }
    }
}
