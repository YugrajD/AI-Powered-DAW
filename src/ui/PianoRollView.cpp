#include "PianoRollView.h"

namespace
{
juce::Colour backgroundColour { 0xff141922 };
juce::Colour keyHeaderColour { 0xff242b37 };
juce::Colour beatGridColour { 0xff303847 };
juce::Colour octaveGridColour { 0xff465164 };
juce::Colour noteColour { 0xfff0c15d };
juce::Colour noteBorderColour { 0xffffdfa0 };

bool isBlackKey(int pitch) noexcept
{
    const auto semitone = pitch % 12;
    return semitone == 1 || semitone == 3 || semitone == 6 || semitone == 8 || semitone == 10;
}
}

PianoRollView::PianoRollView(aidaw::Project& projectToDisplay)
    : project(projectToDisplay)
{
}

void PianoRollView::setClip(aidaw::EntityId trackId, aidaw::EntityId clipId)
{
    selectedTrackId = trackId;
    selectedClipId = clipId;
    repaint();
}

void PianoRollView::paint(juce::Graphics& graphics)
{
    graphics.fillAll(backgroundColour);

    const auto bounds = getLocalBounds();
    const auto roll = bounds.withTrimmedLeft(keyboardWidth);
    const auto pitchCount = (maxPitch - minPitch) + 1;
    const auto rowHeight = static_cast<float>(juce::jmax(1, roll.getHeight())) / static_cast<float>(pitchCount);

    graphics.setColour(keyHeaderColour);
    graphics.fillRect(bounds.withWidth(keyboardWidth));

    for (int pitch = maxPitch; pitch >= minPitch; --pitch)
    {
        const auto pitchIndex = maxPitch - pitch;
        const auto y = roll.getY() + static_cast<int>(std::floor(static_cast<float>(pitchIndex) * rowHeight));
        const auto nextY = roll.getY() + static_cast<int>(std::floor(static_cast<float>(pitchIndex + 1) * rowHeight));
        const auto row = juce::Rectangle<int> { 0, y, bounds.getWidth(), juce::jmax(1, nextY - y) };

        if (isBlackKey(pitch))
        {
            graphics.setColour(juce::Colour { 0x33252d38 });
            graphics.fillRect(row.withTrimmedLeft(keyboardWidth));
        }

        graphics.setColour(pitch % 12 == 0 ? octaveGridColour : beatGridColour);
        graphics.drawHorizontalLine(row.getBottom(), 0.0f, static_cast<float>(bounds.getRight()));

        if (pitch % 12 == 0)
        {
            graphics.setColour(juce::Colour { 0xffcbd3e1 });
            graphics.setFont(juce::FontOptions { 11.0f });
            graphics.drawText("C" + juce::String((pitch / 12) - 1),
                              8, row.getY(), keyboardWidth - 12, row.getHeight(),
                              juce::Justification::centredLeft);
        }
    }

    const auto* clip = selectedClip();
    const auto clipLength = clip != nullptr ? clip->lengthBeats : 4.0;
    const int visibleBeats = juce::jmax(1, static_cast<int>(std::ceil(clipLength)));
    for (int beat = 0; beat <= visibleBeats; ++beat)
    {
        const auto x = roll.getX() + static_cast<int>(std::round(static_cast<double>(beat) * pixelsPerBeat));
        graphics.setColour(beat % 4 == 0 ? octaveGridColour : beatGridColour);
        graphics.drawVerticalLine(x, static_cast<float>(roll.getY()), static_cast<float>(roll.getBottom()));
    }

    graphics.setColour(octaveGridColour);
    graphics.drawRect(bounds);
    graphics.drawVerticalLine(keyboardWidth, 0.0f, static_cast<float>(bounds.getBottom()));

    if (clip == nullptr)
    {
        graphics.setColour(juce::Colour { 0xff9ba6b8 });
        graphics.drawText("No MIDI clip selected", roll.reduced(12), juce::Justification::centred);
        return;
    }

    for (const auto& note : clip->notes)
    {
        if (note.pitch < minPitch || note.pitch > maxPitch)
            continue;

        const auto pitchIndex = maxPitch - note.pitch;
        const auto x = roll.getX() + static_cast<int>(std::round(note.startBeat * pixelsPerBeat));
        const auto y = roll.getY() + static_cast<int>(std::floor(static_cast<float>(pitchIndex) * rowHeight));
        const auto w = juce::jmax(10, static_cast<int>(std::round(note.lengthBeats * pixelsPerBeat)));
        const auto h = juce::jmax(5, static_cast<int>(std::ceil(rowHeight)) - 2);
        const auto noteRect = juce::Rectangle<int> { x, y + 1, w, h };

        graphics.setColour(noteColour.withAlpha(0.65f + (note.velocity * 0.35f)));
        graphics.fillRoundedRectangle(noteRect.toFloat(), 3.0f);
        graphics.setColour(noteBorderColour);
        graphics.drawRoundedRectangle(noteRect.toFloat(), 3.0f, 1.0f);
    }
}

void PianoRollView::mouseDown(const juce::MouseEvent& event)
{
    if (event.x < keyboardWidth || selectedClip() == nullptr)
        return;

    const auto pitch = pitchFromY(event.y);
    const auto beat = beatFromX(event.x);
    [[maybe_unused]] auto& note = project.addMidiNote(selectedTrackId,
                                                      selectedClipId,
                                                      pitch,
                                                      beat,
                                                      0.25,
                                                      0.85f);

    repaint();

    if (onEdited)
        onEdited();
}

const aidaw::Clip* PianoRollView::selectedClip() const
{
    return project.findClip(selectedTrackId, selectedClipId);
}

int PianoRollView::pitchFromY(int y) const noexcept
{
    const auto roll = getLocalBounds().withTrimmedLeft(keyboardWidth);
    const auto pitchCount = (maxPitch - minPitch) + 1;
    const auto rowHeight = static_cast<double>(juce::jmax(1, roll.getHeight())) / static_cast<double>(pitchCount);
    const auto pitchIndex = juce::jlimit(0, pitchCount - 1, static_cast<int>(std::floor(static_cast<double>(y - roll.getY()) / rowHeight)));
    return maxPitch - pitchIndex;
}

double PianoRollView::beatFromX(int x) const noexcept
{
    const auto rollX = getLocalBounds().withTrimmedLeft(keyboardWidth).getX();
    const auto rawBeat = static_cast<double>(x - rollX) / pixelsPerBeat;
    const auto quantizedBeat = std::round(rawBeat / 0.25) * 0.25;
    return juce::jmax(0.0, quantizedBeat);
}
