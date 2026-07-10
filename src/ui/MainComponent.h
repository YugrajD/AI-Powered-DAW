#pragma once

#include "../audio/AudioEngine.h"
#include "../core/DiagnosticLog.h"
#include "../core/Project.h"
#include "ArrangementView.h"
#include "InspectorPanel.h"
#include "PianoRollView.h"

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
    void refreshDiagnostics();

    aidaw::Project project;
    aidaw::DiagnosticLog log;
    aidaw::AudioEngine audioEngine;
    aidaw::EntityId demoTrackId = 0;
    aidaw::EntityId demoClipId = 0;
    juce::Label titleLabel;
    juce::Label statusLabel;
    ArrangementView arrangementView { project };
    PianoRollView pianoRollView { project };
    InspectorPanel inspectorPanel { project };
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton transposeUpButton { "+12" };
    juce::TextButton transposeDownButton { "-12" };
    juce::TextButton quantizeButton { "Quantize" };
    juce::TextButton duplicateButton { "Duplicate" };
    juce::ToggleButton metronomeToggle { "Metronome" };
    juce::TextEditor diagnosticsEditor;
};
