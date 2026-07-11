#pragma once

#include "../audio/AudioEngine.h"
#include "../audio/OfflineRenderer.h"
#include "../commands/CommandExecutor.h"
#include "../core/DiagnosticLog.h"
#include "../core/Project.h"
#include "ArrangementView.h"
#include "AutomationLaneView.h"
#include "InspectorPanel.h"
#include "MixerPanel.h"
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
    void refreshProjectViews();

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
    AutomationLaneView automationLaneView { project };
    InspectorPanel inspectorPanel { project };
    MixerPanel mixerPanel { project };
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
    juce::TextButton saveProjectButton { "Save" };
    juce::TextButton loadProjectButton { "Load" };
    juce::TextButton exportWavButton { "Export WAV" };
    juce::TextEditor diagnosticsEditor;
    std::unique_ptr<juce::FileChooser> audioFileChooser;
    std::unique_ptr<juce::FileChooser> projectFileChooser;
    std::unique_ptr<juce::FileChooser> exportFileChooser;
    juce::File currentProjectFile;
    aidaw::CommandHistory commandHistory;
    juce::TextEditor commandEditor;
    juce::TextButton previewCommandButton { "Preview Command" };
    juce::TextButton executeCommandButton { "Execute Command" };
    juce::TextEditor commandHistoryEditor;
};
