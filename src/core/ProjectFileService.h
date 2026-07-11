#pragma once

#include "Project.h"

namespace aidaw
{
class ProjectFileService
{
public:
    [[nodiscard]] static bool saveProject(const Project& project, const juce::File& file, juce::String* error = nullptr);
    [[nodiscard]] static bool saveBackup(const Project& project, const juce::File& projectFile, juce::String* error = nullptr);
    [[nodiscard]] static bool loadProject(const juce::File& file, Project& destination, juce::String* error = nullptr);
};
}
