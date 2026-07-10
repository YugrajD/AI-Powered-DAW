#include <juce_gui_extra/juce_gui_extra.h>

class AIPoweredDAWApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "AI Powered DAW"; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override {}
    void shutdown() override {}

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted(const juce::String&) override {}
};

START_JUCE_APPLICATION(AIPoweredDAWApplication)
