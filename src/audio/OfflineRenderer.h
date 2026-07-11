#pragma once

#include "../core/Project.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace aidaw
{
class OfflineRenderer
{
public:
    struct Options
    {
        double sampleRate = 44100.0;
        int blockSize = 512;
        int numChannels = 2;
        double startBeat = 0.0;
        double lengthBeats = 16.0;
    };

    [[nodiscard]] static bool renderToWav(const Project& project,
                                          const juce::File& outputFile,
                                          const Options& options,
                                          juce::String* error = nullptr);
};
}
