#pragma once

#include "../core/Project.h"

namespace aidaw
{
struct CommandResult
{
    bool ok = false;
    juce::String message;
    juce::var data;
};

class CommandExecutor
{
public:
    [[nodiscard]] static CommandResult previewJson(const Project& project, const juce::String& json);
    [[nodiscard]] static CommandResult executeJson(Project& project, const juce::String& json);
    [[nodiscard]] static juce::String summarizeProject(const Project& project);
};
}
