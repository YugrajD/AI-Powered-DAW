#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace aidaw
{
enum class LogLevel
{
    info,
    warning,
    error
};

struct LogEntry
{
    LogLevel level = LogLevel::info;
    juce::String message;
    juce::Time timestamp;
};

class DiagnosticLog
{
public:
    void info(juce::String message);
    void warning(juce::String message);
    void error(juce::String message);

    [[nodiscard]] std::vector<LogEntry> snapshot() const;
    [[nodiscard]] juce::String toDisplayString() const;

private:
    void append(LogLevel level, juce::String message);

    mutable juce::CriticalSection lock;
    std::vector<LogEntry> entries;
};
}
