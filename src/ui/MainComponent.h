#pragma once

#include "../audio/AudioEngine.h"
#include "../core/DiagnosticLog.h"
#include "../core/Project.h"
#include "ArrangementView.h"
#include "InspectorPanel.h"
#include "PianoRollView.h"
#include "TransportBar.h"

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
    void refreshDeviceControls();

    aidaw::Project project;
    aidaw::DiagnosticLog log;
    aidaw::AudioEngine audioEngine;
    aidaw::EntityId demoTrackId = 0;
    aidaw::EntityId demoClipId = 0;
    aidaw::EntityId audioTrackId = 0;
    juce::Label titleLabel;
    juce::Label statusLabel;
    ArrangementView arrangementView { project };
    PianoRollView pianoRollView { project };
    InspectorPanel inspectorPanel { project };
    TransportBar transportBar { audioEngine };
    juce::TextButton transposeUpButton { "+12" };
    juce::TextButton transposeDownButton { "-12" };
    juce::TextButton quantizeButton { "Quantize" };
    juce::TextButton duplicateButton { "Duplicate" };
    juce::ComboBox instrumentSelector;
    juce::TextButton addLowPassButton { "Low-pass" };
    juce::TextButton addSaturationButton { "Saturate" };
    juce::TextButton addDelayButton { "Delay" };
    juce::TextButton importAudioButton { "Import Audio" };
    juce::TextEditor diagnosticsEditor;
    std::unique_ptr<juce::FileChooser> audioFileChooser;
};
