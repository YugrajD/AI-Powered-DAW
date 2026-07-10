#include "DiagnosticLog.h"

namespace aidaw
{
namespace
{
juce::String levelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::info:
            return "INFO";
        case LogLevel::warning:
            return "WARN";
        case LogLevel::error:
            return "ERROR";
    }

    return "INFO";
}
}

void DiagnosticLog::info(juce::String message)
{
    append(LogLevel::info, std::move(message));
}

void DiagnosticLog::warning(juce::String message)
{
    append(LogLevel::warning, std::move(message));
}

void DiagnosticLog::error(juce::String message)
{
    append(LogLevel::error, std::move(message));
}

std::vector<LogEntry> DiagnosticLog::snapshot() const
{
    const juce::ScopedLock guard(lock);
    return entries;
}

juce::String DiagnosticLog::toDisplayString() const
{
    const auto currentEntries = snapshot();
    juce::String output;

    for (const auto& entry : currentEntries)
    {
        output << entry.timestamp.formatted("%H:%M:%S")
               << " [" << levelToString(entry.level) << "] "
               << entry.message << "\n";
    }

    return output;
}

void DiagnosticLog::append(LogLevel level, juce::String message)
{
    const juce::ScopedLock guard(lock);
    entries.push_back(LogEntry { level, std::move(message), juce::Time::getCurrentTime() });
}
}
