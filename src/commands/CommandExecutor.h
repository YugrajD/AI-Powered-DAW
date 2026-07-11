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

struct CommandHistoryEntry
{
    juce::String commandJson;
    juce::String preview;
    CommandResult result;
    juce::Time timestamp;
};

class CommandHistory
{
public:
    void record(juce::String commandJson, juce::String preview, CommandResult result);
    [[nodiscard]] const std::vector<CommandHistoryEntry>& entries() const noexcept { return history; }
    [[nodiscard]] juce::String toDisplayString() const;

private:
    std::vector<CommandHistoryEntry> history;
};

class CommandExecutor
{
public:
    [[nodiscard]] static CommandResult previewJson(const Project& project, const juce::String& json);
    [[nodiscard]] static CommandResult executeJson(Project& project, const juce::String& json);
    [[nodiscard]] static CommandResult executeJson(Project& project, const juce::String& json, CommandHistory& history);
    [[nodiscard]] static juce::String summarizeProject(const Project& project);
};
}
