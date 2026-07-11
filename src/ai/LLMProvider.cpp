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

juce::String trimMarkdownFence(juce::String text)
{
    text = text.trim();

    if (text.startsWith("```"))
    {
        auto firstNewline = text.indexOfChar('\n');
        if (firstNewline >= 0)
            text = text.substring(firstNewline + 1);

        if (text.endsWith("```"))
            text = text.dropLastCharacters(3);
    }

    return text.trim();
}

LLMResponse parseCommandJsonContent(const juce::String& content)
{
    const auto commandJson = trimMarkdownFence(content);
    if (commandJson.isEmpty())
        return LLMResponse { false, {}, "Provider returned empty command content" };

    if (juce::JSON::parse(commandJson).isVoid())
        return LLMResponse { false, {}, "Provider command content is not valid JSON" };

    return LLMResponse { true, commandJson, {} };
}

HttpRequest makeJsonRequest(const LLMProviderConfig& config, juce::String body, juce::String extraHeaders = {})
{
    HttpRequest request;
    request.url = config.endpoint;
    request.method = "POST";
    request.body = std::move(body);
    request.timeoutMs = config.timeoutMs;
    request.headers = "Content-Type: application/json\r\n";

    if (extraHeaders.isNotEmpty())
        request.headers << extraHeaders;

    return request;
}

LLMResponse responseFromHttp(const HttpResponse& response, const juce::String& providerName)
{
    if (response.statusCode < 200 || response.statusCode >= 300)
    {
        juce::String error = providerName + " request failed";
        if (response.statusCode != 0)
            error << " with HTTP " << response.statusCode;
        if (response.error.isNotEmpty())
            error << ": " << response.error;
        return LLMResponse { false, {}, error };
    }

    return LLMResponse { true, response.body, {} };
}
}

HttpResponse JuceHttpTransport::send(const HttpRequest& request)
{
    int statusCode = 0;
    auto url = juce::URL::createWithoutParsing(request.url).withPOSTData(request.body);
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withExtraHeaders(request.headers)
                       .withConnectionTimeoutMs(request.timeoutMs)
                       .withStatusCode(&statusCode)
                       .withHttpRequestCmd(request.method);

    auto stream = url.createInputStream(options);
    if (stream == nullptr)
        return HttpResponse { statusCode, {}, "Could not open HTTP stream" };

    return HttpResponse { statusCode, stream->readEntireStreamAsString(), {} };
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

OpenAICompatibleProvider::OpenAICompatibleProvider(LLMProviderConfig providerConfig, IHttpTransport& httpTransport)
    : config(std::move(providerConfig)),
      transport(httpTransport)
{
}

LLMResponse OpenAICompatibleProvider::generateCommand(const LLMRequest& request)
{
    auto headers = juce::String {};
    if (config.apiKey.isNotEmpty())
        headers << "Authorization: Bearer " << config.apiKey << "\r\n";

    const auto httpResponse = transport.send(makeJsonRequest(config,
                                                             OpenAICompatibleRequestBuilder::buildChatCompletionsJson(config.model, request),
                                                             headers));
    const auto raw = responseFromHttp(httpResponse, "OpenAI-compatible provider");
    return raw.ok ? parseResponse(raw.commandJson) : raw;
}

LLMResponse OpenAICompatibleProvider::parseResponse(const juce::String& responseBody)
{
    const auto parsed = juce::JSON::parse(responseBody);
    if (parsed.isVoid())
        return LLMResponse { false, {}, "OpenAI-compatible response is not valid JSON" };

    const auto* choices = parsed.getProperty("choices", juce::var {}).getArray();
    if (choices == nullptr || choices->isEmpty())
        return LLMResponse { false, {}, "OpenAI-compatible response has no choices" };

    const auto content = choices->getFirst().getProperty("message", juce::var {}).getProperty("content", {}).toString();
    return parseCommandJsonContent(content);
}

OllamaProvider::OllamaProvider(LLMProviderConfig providerConfig, IHttpTransport& httpTransport)
    : config(std::move(providerConfig)),
      transport(httpTransport)
{
}

LLMResponse OllamaProvider::generateCommand(const LLMRequest& request)
{
    const auto httpResponse = transport.send(makeJsonRequest(config,
                                                             OllamaRequestBuilder::buildGenerateJson(config.model, request)));
    const auto raw = responseFromHttp(httpResponse, "Ollama provider");
    return raw.ok ? parseResponse(raw.commandJson) : raw;
}

LLMResponse OllamaProvider::parseResponse(const juce::String& responseBody)
{
    const auto parsed = juce::JSON::parse(responseBody);
    if (parsed.isVoid())
        return LLMResponse { false, {}, "Ollama response is not valid JSON" };

    return parseCommandJsonContent(parsed.getProperty("response", {}).toString());
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
