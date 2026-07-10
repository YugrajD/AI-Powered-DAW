#include <juce_gui_extra/juce_gui_extra.h>

#include "ui/MainComponent.h"

class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(juce::String name)
        : DocumentWindow(std::move(name),
                         juce::Desktop::getInstance().getDefaultLookAndFeel()
                             .findColour(juce::ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        centreWithSize(getWidth(), getHeight());
        setResizable(true, true);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class AIPoweredDAWApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "AI Powered DAW"; }
    const juce::String getApplicationVersion() override { return AIDAW_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted(const juce::String&) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AIPoweredDAWApplication)
