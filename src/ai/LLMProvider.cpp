#include "LLMProvider.h"

namespace aidaw
{
namespace
{
juce::String systemPrompt()
{
    return "You are an AI coproducer inside a DAW. Return exactly one JSON command matching the provided tool manifest. Do not return prose.";
}

juce::String combinedPrompt(const LLMRequest& request)
{
    juce::String prompt;
    prompt << "Project summary:\n"
           << request.projectSummary
           << "\nTool manifest:\n"
           << request.toolManifestJson
           << "\nUser request:\n"
           << request.userPrompt;
    return prompt;
}
}

MockLLMProvider::MockLLMProvider(juce::String command)
    : commandJson(std::move(command))
{
}

LLMResponse MockLLMProvider::generateCommand(const LLMRequest& request)
{
    juce::ignoreUnused(request);
    return LLMResponse { true, commandJson, {} };
}

juce::String OpenAICompatibleRequestBuilder::buildChatCompletionsJson(const juce::String& model, const LLMRequest& request)
{
    auto root = new juce::DynamicObject();
    root->setProperty("model", model);
    root->setProperty("temperature", 0.2);

    juce::Array<juce::var> messages;

    auto system = new juce::DynamicObject();
    system->setProperty("role", "system");
    system->setProperty("content", systemPrompt());
    messages.add(system);

    auto user = new juce::DynamicObject();
    user->setProperty("role", "user");
    user->setProperty("content", combinedPrompt(request));
    messages.add(user);

    root->setProperty("messages", messages);
    return juce::JSON::toString(root, true);
}

juce::String OllamaRequestBuilder::buildGenerateJson(const juce::String& model, const LLMRequest& request)
{
    auto root = new juce::DynamicObject();
    root->setProperty("model", model);
    root->setProperty("stream", false);
    root->setProperty("prompt", systemPrompt() + "\n\n" + combinedPrompt(request));
    return juce::JSON::toString(root, true);
}
}
