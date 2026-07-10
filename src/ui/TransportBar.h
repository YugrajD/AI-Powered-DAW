#pragma once

#include "../audio/AudioEngine.h"

#include <juce_gui_extra/juce_gui_extra.h>

class TransportBar final : public juce::Component
{
public:
    explicit TransportBar(aidaw::AudioEngine& audioEngine);

    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void(bool)> onMetronomeChanged;

    void resized() override;
    void paint(juce::Graphics& graphics) override;
    void refresh();

private:
    aidaw::AudioEngine& audioEngine;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::ToggleButton metronomeToggle { "Metronome" };
    juce::Label beatLabel;
};
