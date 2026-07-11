#include "ProjectFileService.h"

#include "ProjectSerializer.h"

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

bool ProjectFileService::saveProject(const Project& project, const juce::File& file, juce::String* error)
{
    if (file == juce::File {})
    {
        setError(error, "No project file selected");
        return false;
    }

    const auto target = file.withFileExtension(".aidaw");
    const auto parent = target.getParentDirectory();
    if (! parent.exists() && ! parent.createDirectory())
    {
        setError(error, "Could not create project directory");
        return false;
    }

    const auto json = ProjectSerializer::toJson(project);
    if (! target.replaceWithText(json))
    {
        setError(error, "Could not write project file");
        return false;
    }

    return true;
}

bool ProjectFileService::saveBackup(const Project& project, const juce::File& projectFile, juce::String* error)
{
    if (projectFile == juce::File {})
    {
        setError(error, "No project file selected");
        return false;
    }

    const auto backupDirectory = projectFile.getParentDirectory().getChildFile("Backups");
    if (! backupDirectory.exists() && ! backupDirectory.createDirectory())
    {
        setError(error, "Could not create backup directory");
        return false;
    }

    const auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d-%H%M%S");
    const auto backupFile = backupDirectory.getChildFile(projectFile.getFileNameWithoutExtension()
                                                         + "-"
                                                         + timestamp
                                                         + ".aidaw");
    return saveProject(project, backupFile, error);
}

bool ProjectFileService::loadProject(const juce::File& file, Project& destination, juce::String* error)
{
    if (! file.existsAsFile())
    {
        setError(error, "Project file does not exist");
        return false;
    }

    const auto json = file.loadFileAsString();
    if (json.isEmpty())
    {
        setError(error, "Project file is empty");
        return false;
    }

    destination = ProjectSerializer::fromJson(json);
    return true;
}
}
