#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <optional>

namespace synthortion
{
    enum class PresetErrorCode
    {
        Ok = 0,
        FileNotFound,
        InvalidJson,
        SchemaMismatch,
        WriteFailed,
        PermissionDenied,
        CannotDeleteFactoryPreset,
        DuplicateName
    };

    struct PresetResult
    {
        bool success = true;
        PresetErrorCode code = PresetErrorCode::Ok;
        juce::String message;

        static PresetResult ok() noexcept
        {
            return { true, PresetErrorCode::Ok, {} };
        }

        static PresetResult fail(PresetErrorCode c, const juce::String& msg)
        {
            return { false, c, msg };
        }
    };
    struct PresetId
    {
        bool isFactory = false;
        juce::String category;
        juce::String name;

        juce::String toString() const
        {
            const juce::String prefix = isFactory ? "factory://" : "user://";
            return prefix + category + "/" + name;
        }

        static PresetId parse(const juce::String& idString)
        {
            PresetId pid;
            if (idString.startsWith("factory://"))
            {
                pid.isFactory = true;
                auto rest = idString.substring(10);
                auto slashIdx = rest.indexOfChar('/');
                if (slashIdx >= 0)
                {
                    pid.category = rest.substring(0, slashIdx);
                    pid.name = rest.substring(slashIdx + 1);
                }
                else
                {
                    pid.name = rest;
                }
            }
            else if (idString.startsWith("user://"))
            {
                pid.isFactory = false;
                auto rest = idString.substring(7);
                auto slashIdx = rest.indexOfChar('/');
                if (slashIdx >= 0)
                {
                    pid.category = rest.substring(0, slashIdx);
                    pid.name = rest.substring(slashIdx + 1);
                }
                else
                {
                    pid.name = rest;
                }
            }
            else
            {
                pid.name = idString;
            }
            return pid;
        }
    };

    struct PresetMetadata
    {
        juce::String name;
        juce::String category = "User";
        juce::String author = "User";
        juce::String description;
        juce::StringArray tags;
        bool favorite = false;
        juce::String createdAt;
        juce::String modifiedAt;
    };
    struct PresetHeader
    {
        juce::String id;
        juce::String name;
        juce::String category;
        juce::String author;
        juce::String description;
        juce::StringArray tags;
        bool isFactory = false;
        juce::String filePath;
        bool favorite = false;
        juce::String createdAt;
        juce::String modifiedAt;

        juce::var toVar() const;
        static PresetHeader fromMetadata(const PresetMetadata& meta, const juce::String& id, bool isFactory, const juce::String& filePath);
    };

    struct PresetData
    {
        int schemaVersion = 1;
        PresetMetadata metadata;
        std::map<juce::String, float> parameters;
        std::map<juce::String, juce::var> uiPreferences;

        juce::var toVar() const;
        juce::String toJsonString(bool pretty = true) const;

        static PresetResult fromVar(const juce::var& v, PresetData& outPreset);
        static PresetResult parseJson(const juce::String& jsonString, PresetData& outPreset);
        static PresetData fromAPVTS(const juce::AudioProcessorValueTreeState& apvts, const PresetMetadata& meta);
        void applyToAPVTS(juce::AudioProcessorValueTreeState& apvts) const;
    };

    class PresetManager
    {
    public:
        PresetManager();
        explicit PresetManager(const juce::File& userPresetsDirectory);
        ~PresetManager() = default;

        static juce::File getDefaultUserPresetsDirectory();
        const juce::File& getUserPresetsDirectory() const noexcept { return userPresetsDirectory; }
        void setUserPresetsDirectory(const juce::File& dir);

        void ensureDirectoryHierarchy() const;
        void scanUserPresets();
        void scanPresets();

        static juce::String sanitizeFilename(const juce::String& name);
        static juce::String makePresetId(bool isFactory, const juce::String& category, const juce::String& name);

        PresetResult saveUserPreset(const PresetData& data, bool allowOverwrite = false);
        PresetResult saveUserPreset(const PresetMetadata& meta,
                                   const juce::AudioProcessorValueTreeState& apvts,
                                   bool allowOverwrite = false);
        PresetResult saveUserPreset(const juce::String& category,
                                   const juce::String& name,
                                   const juce::String& author,
                                   const juce::String& description,
                                   const juce::StringArray& tags,
                                   const juce::AudioProcessorValueTreeState& apvts,
                                   bool allowOverwrite = false);
        PresetResult loadPresetData(const juce::String& id, PresetData& outData) const;
        PresetResult loadPreset(const juce::String& id, juce::AudioProcessorValueTreeState& apvts);
        PresetResult deleteUserPreset(const juce::String& id);

        void initializeFactoryPresets();
        int getNumFactoryPresets() const noexcept { return static_cast<int>(factoryPresetsOrdered.size()); }
        const PresetData* getFactoryPreset(int index) const noexcept;
        juce::String getFactoryPresetName(int index) const;
        int getFactoryPresetIndex(const juce::String& id) const;
        PresetResult loadFactoryPreset(int index, juce::AudioProcessorValueTreeState& apvts);

        void registerFactoryPreset(const juce::String& id, const juce::String& jsonContent);
        void registerFactoryPreset(const PresetData& data);
        void clearFactoryPresets();

        const std::vector<PresetHeader>& getCatalog() const noexcept { return catalog; }
        std::optional<PresetHeader> getPresetHeaderById(const juce::String& id) const;
        std::vector<PresetHeader> getPresetsForCategory(const juce::String& category) const;
        juce::StringArray getCategories() const;

        juce::String getActivePresetId() const { return activePresetId; }
        void setActivePresetId(const juce::String& id) { activePresetId = id; }

        static const juce::StringArray& getCanonicalCategories();

    private:
        void rebuildCatalog();

        juce::File userPresetsDirectory;
        std::vector<PresetHeader> catalog;
        std::vector<PresetData> factoryPresetsOrdered;
        std::unordered_map<juce::String, size_t> factoryPresetIndexMap;
        std::unordered_map<juce::String, juce::File> userPresetFiles;
        juce::String activePresetId;
    };
}
