#pragma once

#include "../audio/AudioEngine.h"
#include "../core/DiagnosticLog.h"
#include "../core/Project.h"

#include <juce_gui_extra/juce_gui_extra.h>

class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshStatus();

    aidaw::Project project;
    aidaw::DiagnosticLog log;
    aidaw::AudioEngine audioEngine;
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::ToggleButton metronomeToggle { "Metronome" };
    juce::TextEditor diagnosticsEditor;
};
