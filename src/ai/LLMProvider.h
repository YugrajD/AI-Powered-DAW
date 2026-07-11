#pragma once

#include <juce_core/juce_core.h>

namespace aidaw
{
struct LLMRequest
{
    juce::String userPrompt;
    juce::String projectSummary;
    juce::String toolManifestJson;
};

struct LLMResponse
{
    bool ok = false;
    juce::String commandJson;
    juce::String error;
};

class ILLMProvider
{
public:
    virtual ~ILLMProvider() = default;
    [[nodiscard]] virtual LLMResponse generateCommand(const LLMRequest& request) = 0;
};

class MockLLMProvider final : public ILLMProvider
{
public:
    explicit MockLLMProvider(juce::String commandJson);

    [[nodiscard]] LLMResponse generateCommand(const LLMRequest& request) override;

private:
    juce::String commandJson;
};

class OpenAICompatibleRequestBuilder
{
public:
    [[nodiscard]] static juce::String buildChatCompletionsJson(const juce::String& model, const LLMRequest& request);
};

class OllamaRequestBuilder
{
public:
    [[nodiscard]] static juce::String buildGenerateJson(const juce::String& model, const LLMRequest& request);
};
}
