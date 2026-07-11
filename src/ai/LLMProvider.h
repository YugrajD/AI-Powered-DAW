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

enum class LLMProviderKind
{
    mock,
    openAICompatible,
    ollama
};

struct LLMProviderConfig
{
    LLMProviderKind kind = LLMProviderKind::mock;
    juce::String endpoint;
    juce::String model;
    juce::String apiKey;
    int timeoutMs = 30000;
};

struct HttpRequest
{
    juce::String url;
    juce::String method = "POST";
    juce::String headers;
    juce::String body;
    int timeoutMs = 30000;
};

struct HttpResponse
{
    int statusCode = 0;
    juce::String body;
    juce::String error;
};

class IHttpTransport
{
public:
    virtual ~IHttpTransport() = default;
    [[nodiscard]] virtual HttpResponse send(const HttpRequest& request) = 0;
};

class JuceHttpTransport final : public IHttpTransport
{
public:
    [[nodiscard]] HttpResponse send(const HttpRequest& request) override;
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

class OpenAICompatibleProvider final : public ILLMProvider
{
public:
    OpenAICompatibleProvider(LLMProviderConfig config, IHttpTransport& transport);

    [[nodiscard]] LLMResponse generateCommand(const LLMRequest& request) override;
    [[nodiscard]] static LLMResponse parseResponse(const juce::String& responseBody);

private:
    LLMProviderConfig config;
    IHttpTransport& transport;
};

class OllamaProvider final : public ILLMProvider
{
public:
    OllamaProvider(LLMProviderConfig config, IHttpTransport& transport);

    [[nodiscard]] LLMResponse generateCommand(const LLMRequest& request) override;
    [[nodiscard]] static LLMResponse parseResponse(const juce::String& responseBody);

private:
    LLMProviderConfig config;
    IHttpTransport& transport;
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
