#pragma once

#include "Project.h"

namespace aidaw
{
class ProjectSerializer
{
public:
    static constexpr int currentSchemaVersion = 1;

    [[nodiscard]] static juce::var toVar(const Project& project);
    [[nodiscard]] static juce::String toJson(const Project& project);
    [[nodiscard]] static Project fromJson(const juce::String& json);
};
}
