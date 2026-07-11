#include "OfflineRenderer.h"

#include "TrackProcessingGraph.h"

namespace aidaw
{
namespace
{
void setError(juce::String* error, juce::String message)
{
    if (error != nullptr)
        *error = std::move(message);
}
}

bool OfflineRenderer::renderToWav(const Project& project,
                                  const juce::File& outputFile,
                                  const Options& options,
                                  juce::String* error)
{
    if (outputFile == juce::File {})
    {
        setError(error, "No output file selected");
        return false;
    }

    if (options.sampleRate <= 0.0 || options.blockSize <= 0 || options.numChannels <= 0 || options.lengthBeats <= 0.0)
    {
        setError(error, "Invalid render options");
        return false;
    }

    const auto target = outputFile.withFileExtension(".wav");
    const auto parent = target.getParentDirectory();
    if (! parent.exists() && ! parent.createDirectory())
    {
        setError(error, "Could not create render directory");
        return false;
    }

    const auto tempo = project.getTransport().bpm > 0.0 ? project.getTransport().bpm : 120.0;
    const auto secondsPerBeat = 60.0 / tempo;
    const auto totalSamples = static_cast<int64_t>(std::ceil(options.lengthBeats * secondsPerBeat * options.sampleRate));
    const auto beatsPerSample = tempo / 60.0 / options.sampleRate;

    TrackProcessingGraph graph;
    graph.configureFromProject(project);
    graph.prepare(options.sampleRate, options.blockSize, options.numChannels);

    juce::WavAudioFormat wavFormat;
    target.deleteFile();
    auto stream = std::unique_ptr<juce::FileOutputStream>(target.createOutputStream());
    auto writer = std::unique_ptr<juce::AudioFormatWriter>(
        wavFormat.createWriterFor(stream.get(),
                                  options.sampleRate,
                                  static_cast<unsigned int>(options.numChannels),
                                  24,
                                  {},
                                  0));

    if (writer == nullptr)
    {
        setError(error, "Could not create WAV writer");
        return false;
    }

    stream.release();

    juce::AudioBuffer<float> block(options.numChannels, options.blockSize);
    int64_t samplesRendered = 0;
    auto currentBeat = options.startBeat;

    while (samplesRendered < totalSamples)
    {
        const auto samplesThisBlock = static_cast<int>(juce::jmin<int64_t>(options.blockSize, totalSamples - samplesRendered));
        graph.render(block, samplesThisBlock, currentBeat, beatsPerSample);

        if (samplesThisBlock < options.blockSize)
            block.clear(samplesThisBlock, options.blockSize - samplesThisBlock);

        writer->writeFromAudioSampleBuffer(block, 0, samplesThisBlock);

        samplesRendered += samplesThisBlock;
        currentBeat += static_cast<double>(samplesThisBlock) * beatsPerSample;
    }

    return true;
}
}
