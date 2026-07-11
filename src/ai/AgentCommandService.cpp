#include "AgentCommandService.h"

namespace aidaw
{
LLMRequest AgentCommandService::buildRequest(const Project& project, const juce::String& userPrompt)
{
    return LLMRequest {
        userPrompt,
        CommandExecutor::summarizeProject(project),
        CommandExecutor::toolManifestJson()
    };
}

AgentCommandResult AgentCommandService::run(Project& project,
                                            const juce::String& userPrompt,
                                            ILLMProvider& provider,
                                            CommandHistory& history)
{
    AgentCommandResult result;
    result.request = buildRequest(project, userPrompt);
    result.llmResponse = provider.generateCommand(result.request);

    if (! result.llmResponse.ok)
    {
        result.execution = CommandResult { false, result.llmResponse.error, {} };
        return result;
    }

    result.preview = CommandExecutor::previewJson(project, result.llmResponse.commandJson);
    result.execution = CommandExecutor::executeJson(project, result.llmResponse.commandJson, history);
    result.ok = result.execution.ok;
    return result;
}
}
