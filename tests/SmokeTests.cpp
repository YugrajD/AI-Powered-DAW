#include <juce_core/juce_core.h>

int main()
{
    juce::String appName {"AI Powered DAW"};
    return appName.isNotEmpty() ? 0 : 1;
}
