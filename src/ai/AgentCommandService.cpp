#include "AgentCommandService.h"

namespace aidaw
{
namespace
{
CommandResult makeFailure(juce::String message)
{
    return CommandResult { false, std::move(message), {} };
}

std::vector<juce::String> extractCommandJsons(const juce::String& planJson)
{
    std::vector<juce::String> commands;
    const auto parsed = juce::JSON::parse(planJson);
    if (parsed.isVoid())
        return commands;

    if (const auto* array = parsed.getArray())
    {
        for (const auto& command : *array)
            commands.push_back(juce::JSON::toString(command, true));

        return commands;
    }

    if (const auto* nestedCommands = parsed.getProperty("commands", {}).getArray())
    {
        for (const auto& command : *nestedCommands)
            commands.push_back(juce::JSON::toString(command, true));

        return commands;
    }

    commands.push_back(planJson);
    return commands;
}

int idFromResult(const CommandResult& result)
{
    return static_cast<int>(result.data.getProperty("id", 0));
}

juce::String resolveIdReferences(juce::String commandJson, const std::vector<CommandResult>& completedSteps)
{
    if (! completedSteps.empty())
    {
        const auto lastId = idFromResult(completedSteps.back());
        if (lastId != 0)
            commandJson = commandJson.replace("\"$lastId\"", juce::String(lastId));
    }

    for (size_t index = 0; index < completedSteps.size(); ++index)
    {
        const auto stepId = idFromResult(completedSteps[index]);
        if (stepId != 0)
            commandJson = commandJson.replace("\"$step" + juce::String(static_cast<int>(index + 1)) + ".id\"",
                                              juce::String(stepId));
    }

    return commandJson;
}
}

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

    result.commandJsons = extractCommandJsons(result.llmResponse.commandJson);
    if (result.commandJsons.empty())
    {
        result.execution = makeFailure("Agent response did not contain executable command JSON");
        return result;
    }

    for (auto& commandJson : result.commandJsons)
    {
        commandJson = resolveIdReferences(commandJson, result.stepResults);
        result.preview = CommandExecutor::previewJson(project, commandJson);
        result.execution = CommandExecutor::executeJson(project, commandJson, history);
        result.stepResults.push_back(result.execution);

        if (! result.execution.ok)
        {
            result.ok = false;
            return result;
        }
    }

    result.ok = true;
    return result;
}
}
