#pragma once

#include "LLMProvider.h"
#include "../commands/CommandExecutor.h"

namespace aidaw
{
struct AgentCommandResult
{
    bool ok = false;
    LLMRequest request;
    LLMResponse llmResponse;
    CommandResult preview;
    CommandResult execution;
};

class AgentCommandService
{
public:
    [[nodiscard]] static LLMRequest buildRequest(const Project& project, const juce::String& userPrompt);
    [[nodiscard]] static AgentCommandResult run(Project& project,
                                                const juce::String& userPrompt,
                                                ILLMProvider& provider,
                                                CommandHistory& history);
};
}
