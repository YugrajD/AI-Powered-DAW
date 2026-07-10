#include "InspectorPanel.h"

namespace
{
juce::Colour backgroundColour { 0xff1a2029 };
juce::Colour borderColour { 0xff3a4352 };
juce::Colour labelColour { 0xff97a3b6 };
juce::Colour valueColour { 0xffe4e9f2 };

void drawField(juce::Graphics& graphics, juce::Rectangle<int>& bounds, const juce::String& label, const juce::String& value)
{
    auto row = bounds.removeFromTop(28);
    graphics.setColour(labelColour);
    graphics.setFont(juce::FontOptions { 12.0f });
    graphics.drawText(label, row.removeFromLeft(96), juce::Justification::centredLeft);

    graphics.setColour(valueColour);
    graphics.setFont(juce::FontOptions { 13.0f, juce::Font::bold });
    graphics.drawText(value, row, juce::Justification::centredLeft);
}

juce::String instrumentName(aidaw::InstrumentType instrument)
{
    switch (instrument)
    {
        case aidaw::InstrumentType::sineSynth:
            return "Sine Synth";
        case aidaw::InstrumentType::subtractiveSynth:
            return "Subtractive Synth";
        case aidaw::InstrumentType::drumSynth:
            return "Drum Synth";
    }

    return "Sine Synth";
}
}

InspectorPanel::InspectorPanel(aidaw::Project& projectToInspect)
    : project(projectToInspect)
{
}

void InspectorPanel::setSelection(aidaw::EntityId trackId, aidaw::EntityId clipId)
{
    selectedTrackId = trackId;
    selectedClipId = clipId;
    repaint();
}

void InspectorPanel::paint(juce::Graphics& graphics)
{
    graphics.fillAll(backgroundColour);
    graphics.setColour(borderColour);
    graphics.drawRect(getLocalBounds());

    auto bounds = getLocalBounds().reduced(16);
    graphics.setColour(valueColour);
    graphics.setFont(juce::FontOptions { 16.0f, juce::Font::bold });
    graphics.drawText("Inspector", bounds.removeFromTop(28), juce::Justification::centredLeft);
    bounds.removeFromTop(8);

    const auto* track = selectedTrack();
    const auto* clip = selectedClip();

    if (track == nullptr)
    {
        graphics.setColour(labelColour);
        graphics.drawText("No track selected", bounds, juce::Justification::centred);
        return;
    }

    drawField(graphics, bounds, "Track", track->name);
    drawField(graphics, bounds, "Type", track->type == aidaw::TrackType::midi ? "MIDI" : "Audio");
    drawField(graphics, bounds, "Instrument", instrumentName(track->instrument));
    drawField(graphics, bounds, "Effects", juce::String(track->effects.size()));
    drawField(graphics, bounds, "Gain", juce::String(track->gain, 2));
    drawField(graphics, bounds, "Pan", juce::String(track->pan, 2));
    drawField(graphics, bounds, "Clips", juce::String(track->clips.size()));

    bounds.removeFromTop(10);

    if (clip == nullptr)
    {
        drawField(graphics, bounds, "Clip", "None");
        return;
    }

    drawField(graphics, bounds, "Clip", clip->name);
    drawField(graphics, bounds, "Start", juce::String(clip->startBeat, 2) + " beats");
    drawField(graphics, bounds, "Length", juce::String(clip->lengthBeats, 2) + " beats");
    drawField(graphics, bounds, "Loop", clip->loopEnabled ? "On" : "Off");
    drawField(graphics, bounds, "Notes", juce::String(clip->notes.size()));
}

const aidaw::Track* InspectorPanel::selectedTrack() const
{
    return project.findTrack(selectedTrackId);
}

const aidaw::Clip* InspectorPanel::selectedClip() const
{
    return project.findClip(selectedTrackId, selectedClipId);
}
