#include "Synthortion/PresetManager.h"
#include "Synthortion/PluginProcessor.h"
#include <juce_core/juce_core.h>
#include <BinaryData.h>

namespace synthortion
{
    static const juce::StringArray kCanonicalCategories =
    {
        "Init",
        "Bass",
        "Lead",
        "Lo-Fi",
        "Pad",
        "Pluck",
        "FX",
        "Experimental",
        "User"
    };

    const juce::StringArray& PresetManager::getCanonicalCategories()
    {
        return kCanonicalCategories;
    }

    juce::var PresetHeader::toVar() const
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("id", id);
        obj->setProperty("name", name);
        obj->setProperty("category", category);
        obj->setProperty("author", author);
        obj->setProperty("description", description);

        juce::Array<juce::var> tagsVar;
        for (const auto& tag : tags)
            tagsVar.add(tag);
        obj->setProperty("tags", tagsVar);

        obj->setProperty("isFactory", isFactory);
        obj->setProperty("filePath", filePath);
        obj->setProperty("favorite", favorite);
        obj->setProperty("createdAt", createdAt);
        obj->setProperty("modifiedAt", modifiedAt);

        return juce::var(obj.get());
    }

    PresetHeader PresetHeader::fromMetadata(const PresetMetadata& meta, const juce::String& id, bool isFactory, const juce::String& filePath)
    {
        PresetHeader header;
        header.id = id;
        header.name = meta.name;
        header.category = meta.category;
        header.author = meta.author;
        header.description = meta.description;
        header.tags = meta.tags;
        header.isFactory = isFactory;
        header.filePath = filePath;
        header.favorite = meta.favorite;
        header.createdAt = meta.createdAt;
        header.modifiedAt = meta.modifiedAt;
        return header;
    }

    juce::var PresetData::toVar() const
    {
        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        root->setProperty("schemaVersion", schemaVersion);

        juce::DynamicObject::Ptr metaObj = new juce::DynamicObject();
        metaObj->setProperty("name", metadata.name);
        metaObj->setProperty("category", metadata.category);
        metaObj->setProperty("author", metadata.author);
        metaObj->setProperty("description", metadata.description);

        juce::Array<juce::var> tagsVar;
        for (int i = 0; i < juce::jmin(8, metadata.tags.size()); ++i)
            tagsVar.add(metadata.tags[i]);
        metaObj->setProperty("tags", tagsVar);

        metaObj->setProperty("favorite", metadata.favorite);
        metaObj->setProperty("createdAt", metadata.createdAt);
        metaObj->setProperty("modifiedAt", metadata.modifiedAt);
        root->setProperty("metadata", juce::var(metaObj.get()));

        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        for (const auto& [paramId, val] : parameters)
        {
            paramsObj->setProperty(paramId, static_cast<double>(juce::jlimit(0.0f, 1.0f, val)));
        }
        root->setProperty("parameters", juce::var(paramsObj.get()));

        if (!uiPreferences.empty())
        {
            juce::DynamicObject::Ptr uiObj = new juce::DynamicObject();
            for (const auto& [key, val] : uiPreferences)
            {
                uiObj->setProperty(key, val);
            }
            root->setProperty("uiPreferences", juce::var(uiObj.get()));
        }

        return juce::var(root.get());
    }

    juce::String PresetData::toJsonString(bool pretty) const
    {
        return juce::JSON::toString(toVar(), !pretty);
    }

    PresetResult PresetData::fromVar(const juce::var& v, PresetData& outPreset)
    {
        if (!v.isObject())
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Root JSON must be an object.");

        auto* rootObj = v.getDynamicObject();
        if (rootObj == nullptr)
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Failed to read root dynamic object.");

        if (!rootObj->hasProperty("schemaVersion"))
            return PresetResult::fail(PresetErrorCode::SchemaMismatch, "Missing 'schemaVersion' property.");

        auto schemaVersionVar = rootObj->getProperty("schemaVersion");
        if (!schemaVersionVar.isInt() && !schemaVersionVar.isInt64())
            return PresetResult::fail(PresetErrorCode::SchemaMismatch, "'schemaVersion' must be an integer.");

        int version = static_cast<int>(schemaVersionVar);
        if (version != 1)
            return PresetResult::fail(PresetErrorCode::SchemaMismatch, "Unsupported schemaVersion: " + juce::String(version));

        outPreset.schemaVersion = version;

        // Metadata
        if (!rootObj->hasProperty("metadata") || !rootObj->getProperty("metadata").isObject())
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Missing or invalid 'metadata' object.");

        auto* metaObj = rootObj->getProperty("metadata").getDynamicObject();
        if (metaObj == nullptr)
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Corrupt 'metadata' object.");

        if (!metaObj->hasProperty("name") || metaObj->getProperty("name").toString().trim().isEmpty())
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Preset 'name' in metadata is missing or empty.");

        juce::String nameStr = metaObj->getProperty("name").toString().trim();
        if (nameStr.length() > 32)
            nameStr = nameStr.substring(0, 32);

        outPreset.metadata.name = nameStr;
        outPreset.metadata.category = metaObj->getProperty("category").toString().trim();
        if (outPreset.metadata.category.isEmpty())
            outPreset.metadata.category = "User";

        outPreset.metadata.author = metaObj->hasProperty("author") ? metaObj->getProperty("author").toString() : "User";
        outPreset.metadata.description = metaObj->hasProperty("description") ? metaObj->getProperty("description").toString() : "";

        outPreset.metadata.tags.clear();
        if (metaObj->hasProperty("tags") && metaObj->getProperty("tags").isArray())
        {
            auto* tagsArray = metaObj->getProperty("tags").getArray();
            if (tagsArray != nullptr)
            {
                for (int i = 0; i < juce::jmin(8, tagsArray->size()); ++i)
                {
                    outPreset.metadata.tags.add((*tagsArray)[i].toString());
                }
            }
        }

        outPreset.metadata.favorite = metaObj->hasProperty("favorite") ? static_cast<bool>(metaObj->getProperty("favorite")) : false;
        outPreset.metadata.createdAt = metaObj->hasProperty("createdAt") ? metaObj->getProperty("createdAt").toString() : "";
        outPreset.metadata.modifiedAt = metaObj->hasProperty("modifiedAt") ? metaObj->getProperty("modifiedAt").toString() : "";

        // Parameters
        outPreset.parameters.clear();
        if (rootObj->hasProperty("parameters") && rootObj->getProperty("parameters").isObject())
        {
            auto* paramsObj = rootObj->getProperty("parameters").getDynamicObject();
            if (paramsObj != nullptr)
            {
                const auto& props = paramsObj->getProperties();
                for (const auto& namedVal : props)
                {
                    float floatVal = static_cast<float>(namedVal.value);
                    outPreset.parameters[namedVal.name.toString()] = juce::jlimit(0.0f, 1.0f, floatVal);
                }
            }
        }

        // UI Preferences
        outPreset.uiPreferences.clear();
        if (rootObj->hasProperty("uiPreferences") && rootObj->getProperty("uiPreferences").isObject())
        {
            auto* uiObj = rootObj->getProperty("uiPreferences").getDynamicObject();
            if (uiObj != nullptr)
            {
                const auto& props = uiObj->getProperties();
                for (const auto& namedVal : props)
                {
                    outPreset.uiPreferences[namedVal.name.toString()] = namedVal.value;
                }
            }
        }

        return PresetResult::ok();
    }

    PresetResult PresetData::parseJson(const juce::String& jsonString, PresetData& outPreset)
    {
        juce::var parsedVar;
        auto result = juce::JSON::parse(jsonString, parsedVar);
        if (result.failed())
        {
            return PresetResult::fail(PresetErrorCode::InvalidJson, "JSON Parse Error: " + result.getErrorMessage());
        }

        return fromVar(parsedVar, outPreset);
    }

    PresetData PresetData::fromAPVTS(const juce::AudioProcessorValueTreeState& apvts, const PresetMetadata& meta)
    {
        PresetData data;
        data.schemaVersion = 1;
        data.metadata = meta;

        if (data.metadata.createdAt.isEmpty())
            data.metadata.createdAt = juce::Time::getCurrentTime().toISO8601(true);
        data.metadata.modifiedAt = juce::Time::getCurrentTime().toISO8601(true);

        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            {
                data.parameters[pWithId->paramID] = pWithId->getValue();
            }
        }

        auto uiPrefsTree = apvts.state.getChildWithName(UIPreferences::kNodeName);
        data.uiPreferences[UIPreferences::kUiScale] = uiPrefsTree.getProperty(UIPreferences::kUiScale, UIPreferences::kDefaultUiScale);
        data.uiPreferences[UIPreferences::kSpectrumDecay] = uiPrefsTree.getProperty(UIPreferences::kSpectrumDecay, UIPreferences::kDefaultSpectrumDecay);
        data.uiPreferences[UIPreferences::kSkipBootSequence] = uiPrefsTree.getProperty(UIPreferences::kSkipBootSequence, UIPreferences::kDefaultSkipBootSequence);

        return data;
    }

    void PresetData::applyToAPVTS(juce::AudioProcessorValueTreeState& apvts) const
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED;

        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            {
                auto it = parameters.find(pWithId->paramID);
                if (it != parameters.end())
                {
                    float clampedVal = juce::jlimit(0.0f, 1.0f, it->second);
                    pWithId->setValueNotifyingHost(clampedVal);
                }
                else
                {
                    pWithId->setValueNotifyingHost(pWithId->getDefaultValue());
                }
            }
        }

        UIPreferences::ensureTree(apvts.state);
        auto uiPrefsTree = apvts.state.getChildWithName(UIPreferences::kNodeName);
        if (uiPrefsTree.isValid())
        {
            auto itScale = uiPreferences.find(UIPreferences::kUiScale);
            if (itScale != uiPreferences.end())
                uiPrefsTree.setProperty(UIPreferences::kUiScale, itScale->second, nullptr);

            auto itDecay = uiPreferences.find(UIPreferences::kSpectrumDecay);
            if (itDecay != uiPreferences.end())
                uiPrefsTree.setProperty(UIPreferences::kSpectrumDecay, itDecay->second, nullptr);

            auto itBoot = uiPreferences.find(UIPreferences::kSkipBootSequence);
            if (itBoot != uiPreferences.end())
                uiPrefsTree.setProperty(UIPreferences::kSkipBootSequence, itBoot->second, nullptr);
        }
    }

    PresetManager::PresetManager()
        : userPresetsDirectory(getDefaultUserPresetsDirectory())
    {
        initializeFactoryPresets();
        if (!factoryPresetsOrdered.empty())
        {
            activePresetId = makePresetId(true, factoryPresetsOrdered[0].metadata.category, factoryPresetsOrdered[0].metadata.name);
        }
        scanPresets();
    }

    PresetManager::PresetManager(const juce::File& userPresetsDir)
        : userPresetsDirectory(userPresetsDir)
    {
        initializeFactoryPresets();
        if (!factoryPresetsOrdered.empty())
        {
            activePresetId = makePresetId(true, factoryPresetsOrdered[0].metadata.category, factoryPresetsOrdered[0].metadata.name);
        }
        scanPresets();
    }

    juce::File PresetManager::getDefaultUserPresetsDirectory()
    {
#if JUCE_WINDOWS
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Synthortion")
            .getChildFile("Presets");
#elif JUCE_MAC
        return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
            .getChildFile("Library")
            .getChildFile("Audio")
            .getChildFile("Presets")
            .getChildFile("Synthortion");
#else
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Synthortion")
            .getChildFile("Presets");
#endif
    }

    void PresetManager::setUserPresetsDirectory(const juce::File& dir)
    {
        userPresetsDirectory = dir;
        scanPresets();
    }

    void PresetManager::ensureDirectoryHierarchy() const
    {
        if (!userPresetsDirectory.exists())
            userPresetsDirectory.createDirectory();

        for (const auto& category : kCanonicalCategories)
        {
            auto catDir = userPresetsDirectory.getChildFile(category);
            if (!catDir.exists())
                catDir.createDirectory();
        }
    }

    juce::String PresetManager::sanitizeFilename(const juce::String& name)
    {
        juce::String sanitized = name.trim();
        const juce::String illegalChars = "\\/:*?\"<>|";
        for (int i = 0; i < illegalChars.length(); ++i)
        {
            sanitized = sanitized.replaceCharacter(illegalChars[i], '_');
        }

        if (sanitized.length() > 32)
            sanitized = sanitized.substring(0, 32).trimEnd();

        return sanitized;
    }

    juce::String PresetManager::makePresetId(bool isFactory, const juce::String& category, const juce::String& name)
    {
        const juce::String prefix = isFactory ? "factory://" : "user://";
        return prefix + category + "/" + name;
    }

    void PresetManager::scanUserPresets()
    {
        userPresetFiles.clear();
        catalog.clear();

        // 1. Factory Presets in catalog in exact index order
        for (const auto& data : factoryPresetsOrdered)
        {
            juce::String id = makePresetId(true, data.metadata.category, data.metadata.name);
            catalog.push_back(PresetHeader::fromMetadata(data.metadata, id, true, {}));
        }

        if (!userPresetsDirectory.isDirectory())
            return;

        // 2. Scan root folder -> maps to category "User"
        auto rootFiles = userPresetsDirectory.findChildFiles(juce::File::findFiles, false, "*.synthortionpreset");
        for (const auto& file : rootFiles)
        {
            juce::String content = file.loadFileAsString();
            PresetData data;
            if (PresetData::parseJson(content, data).success)
            {
                juce::String id = makePresetId(false, "User", data.metadata.name);
                userPresetFiles[id] = file;
                data.metadata.category = "User";
                catalog.push_back(PresetHeader::fromMetadata(data.metadata, id, false, file.getFullPathName()));
            }
        }

        // 3. Scan category subfolders
        auto subDirs = userPresetsDirectory.findChildFiles(juce::File::findDirectories, false);
        for (const auto& subDir : subDirs)
        {
            juce::String categoryName = subDir.getFileName();
            auto presetFiles = subDir.findChildFiles(juce::File::findFiles, false, "*.synthortionpreset");
            for (const auto& file : presetFiles)
            {
                juce::String content = file.loadFileAsString();
                PresetData data;
                if (PresetData::parseJson(content, data).success)
                {
                    juce::String id = makePresetId(false, categoryName, data.metadata.name);
                    userPresetFiles[id] = file;
                    data.metadata.category = categoryName;
                    catalog.push_back(PresetHeader::fromMetadata(data.metadata, id, false, file.getFullPathName()));
                }
            }
        }
    }

    void PresetManager::scanPresets()
    {
        scanUserPresets();
    }

    void PresetManager::rebuildCatalog()
    {
        scanUserPresets();
    }

    PresetResult PresetManager::saveUserPreset(const PresetData& data, bool allowOverwrite)
    {
        if (data.metadata.name.trim().isEmpty())
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Preset name cannot be empty.");

        juce::String category = data.metadata.category.trim();
        if (category.isEmpty())
            category = "User";

        auto targetDir = userPresetsDirectory.getChildFile(category);
        if (!targetDir.isDirectory())
        {
            if (!targetDir.createDirectory())
                return PresetResult::fail(PresetErrorCode::WriteFailed, "Failed to create category directory: " + targetDir.getFullPathName());
        }

        juce::String sanitizedName = sanitizeFilename(data.metadata.name);
        if (sanitizedName.isEmpty())
            return PresetResult::fail(PresetErrorCode::InvalidJson, "Sanitized preset name is empty.");

        auto destinationFile = targetDir.getChildFile(sanitizedName + ".synthortionpreset");

        if (destinationFile.existsAsFile() && !allowOverwrite)
        {
            return PresetResult::fail(PresetErrorCode::DuplicateName, "Preset '" + data.metadata.name + "' already exists in category '" + category + "'.");
        }

        PresetData copyData = data;
        copyData.metadata.name = sanitizedName;
        copyData.metadata.category = category;
            copyData.metadata.createdAt = juce::Time::getCurrentTime().toISO8601(true);
        copyData.metadata.modifiedAt = juce::Time::getCurrentTime().toISO8601(true);

        juce::String jsonContent = copyData.toJsonString(true);

        // Atomic File Replacement via juce::TemporaryFile
        juce::TemporaryFile tempFile(destinationFile);
        if (!tempFile.getFile().replaceWithText(jsonContent))
        {
            return PresetResult::fail(PresetErrorCode::WriteFailed, "Failed to write temporary file: " + tempFile.getFile().getFullPathName());
        }

        if (!tempFile.overwriteTargetFileWithTemporary())
        {
            return PresetResult::fail(PresetErrorCode::WriteFailed, "Failed to atomically overwrite destination file: " + destinationFile.getFullPathName());
        }

        juce::String id = makePresetId(false, category, copyData.metadata.name);
        userPresetFiles[id] = destinationFile;
        activePresetId = id;
        rebuildCatalog();

        return PresetResult::ok();
    }

    PresetResult PresetManager::saveUserPreset(const juce::String& category,
                                               const juce::String& name,
                                               const juce::String& author,
                                               const juce::String& description,
                                               const juce::StringArray& tags,
                                               const juce::AudioProcessorValueTreeState& apvts,
                                               bool allowOverwrite)
    {
        PresetMetadata meta;
        meta.name = name;
        meta.category = category.isNotEmpty() ? category : "User";
        meta.author = author.isNotEmpty() ? author : "User";
        meta.description = description;
        meta.tags = tags;

        PresetData data = PresetData::fromAPVTS(apvts, meta);
        return saveUserPreset(data, allowOverwrite);
    }

    PresetResult PresetManager::loadPresetData(const juce::String& id, PresetData& outData) const
    {
        if (id.startsWith("factory://"))
        {
            auto it = factoryPresetIndexMap.find(id);
            if (it != factoryPresetIndexMap.end() && it->second < factoryPresetsOrdered.size())
            {
                outData = factoryPresetsOrdered[it->second];
                return PresetResult::ok();
            }
            return PresetResult::fail(PresetErrorCode::FileNotFound, "Factory preset not found: " + id);
        }

        auto it = userPresetFiles.find(id);
        if (it != userPresetFiles.end())
        {
            const auto& file = it->second;
            if (!file.existsAsFile())
                return PresetResult::fail(PresetErrorCode::FileNotFound, "Preset file not found on disk: " + file.getFullPathName());

            juce::String content = file.loadFileAsString();
            return PresetData::parseJson(content, outData);
        }

        return PresetResult::fail(PresetErrorCode::FileNotFound, "Preset not found: " + id);
    }

    PresetResult PresetManager::loadPreset(const juce::String& id, juce::AudioProcessorValueTreeState& apvts)
    {
        PresetData data;
        auto res = loadPresetData(id, data);
        if (!res.success)
            return res;

        data.applyToAPVTS(apvts);
        activePresetId = id;
        return PresetResult::ok();
    }

    PresetResult PresetManager::deleteUserPreset(const juce::String& id)
    {
        if (id.startsWith("factory://"))
        {
            return PresetResult::fail(PresetErrorCode::CannotDeleteFactoryPreset, "Cannot delete factory preset: " + id);
        }

        auto it = userPresetFiles.find(id);
        if (it == userPresetFiles.end())
        {
            return PresetResult::fail(PresetErrorCode::FileNotFound, "User preset not found: " + id);
        }

        auto file = it->second;
        if (file.existsAsFile())
        {
            if (!file.deleteFile())
                return PresetResult::fail(PresetErrorCode::PermissionDenied, "Failed to delete preset file: " + file.getFullPathName());
        }

        userPresetFiles.erase(it);
        if (activePresetId == id)
            activePresetId = "";

        rebuildCatalog();
        return PresetResult::ok();
    }

    void PresetManager::initializeFactoryPresets()
    {
        factoryPresetsOrdered.clear();
        factoryPresetIndexMap.clear();

        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            int dataSize = 0;
            const char* dataPtr = BinaryData::getNamedResource(BinaryData::namedResourceList[i], dataSize);
            if (dataPtr != nullptr && dataSize > 0)
            {
                juce::String jsonStr = juce::String::fromUTF8(dataPtr, dataSize);
                PresetData data;
                if (PresetData::parseJson(jsonStr, data).success)
                {
                    juce::String id = makePresetId(true, data.metadata.category, data.metadata.name);
                    size_t index = factoryPresetsOrdered.size();
                    factoryPresetsOrdered.push_back(data);
                    factoryPresetIndexMap[id] = index;
                }
            }
        }
    }

    const PresetData* PresetManager::getFactoryPreset(int index) const noexcept
    {
        if (index >= 0 && index < static_cast<int>(factoryPresetsOrdered.size()))
            return &factoryPresetsOrdered[static_cast<size_t>(index)];
        return nullptr;
    }

    juce::String PresetManager::getFactoryPresetName(int index) const
    {
        if (index >= 0 && index < static_cast<int>(factoryPresetsOrdered.size()))
            return factoryPresetsOrdered[static_cast<size_t>(index)].metadata.name;
        return {};
    }

    int PresetManager::getFactoryPresetIndex(const juce::String& id) const
    {
        auto it = factoryPresetIndexMap.find(id);
        if (it != factoryPresetIndexMap.end())
            return static_cast<int>(it->second);
        return -1;
    }

    PresetResult PresetManager::loadFactoryPreset(int index, juce::AudioProcessorValueTreeState& apvts)
    {
        if (index < 0 || index >= static_cast<int>(factoryPresetsOrdered.size()))
            return PresetResult::fail(PresetErrorCode::FileNotFound, "Factory preset index out of range: " + juce::String(index));

        const auto& data = factoryPresetsOrdered[static_cast<size_t>(index)];
        data.applyToAPVTS(apvts);
        activePresetId = makePresetId(true, data.metadata.category, data.metadata.name);
        return PresetResult::ok();
    }

    void PresetManager::registerFactoryPreset(const juce::String& id, const juce::String& jsonContent)
    {
        PresetData data;
        if (PresetData::parseJson(jsonContent, data).success)
        {
            auto it = factoryPresetIndexMap.find(id);
            if (it != factoryPresetIndexMap.end())
            {
                factoryPresetsOrdered[it->second] = data;
            }
            else
            {
                size_t index = factoryPresetsOrdered.size();
                factoryPresetsOrdered.push_back(data);
                factoryPresetIndexMap[id] = index;
            }
            rebuildCatalog();
        }
    }

    void PresetManager::registerFactoryPreset(const PresetData& data)
    {
        juce::String id = makePresetId(true, data.metadata.category, data.metadata.name);
        auto it = factoryPresetIndexMap.find(id);
        if (it != factoryPresetIndexMap.end())
        {
            factoryPresetsOrdered[it->second] = data;
        }
        else
        {
            size_t index = factoryPresetsOrdered.size();
            factoryPresetsOrdered.push_back(data);
            factoryPresetIndexMap[id] = index;
        }
        rebuildCatalog();
    }

    void PresetManager::clearFactoryPresets()
    {
        factoryPresetsOrdered.clear();
        factoryPresetIndexMap.clear();
        rebuildCatalog();
    }

    std::optional<PresetHeader> PresetManager::getPresetHeaderById(const juce::String& id) const
    {
        for (const auto& header : catalog)
        {
            if (header.id == id)
                return header;
        }
        return std::nullopt;
    }

    std::vector<PresetHeader> PresetManager::getPresetsForCategory(const juce::String& category) const
    {
        std::vector<PresetHeader> result;
        for (const auto& header : catalog)
        {
            if (header.category.equalsIgnoreCase(category))
                result.push_back(header);
        }
        return result;
    }

    juce::StringArray PresetManager::getCategories() const
    {
        juce::StringArray cats;
        for (const auto& canonical : kCanonicalCategories)
            cats.add(canonical);

        for (const auto& header : catalog)
        {
            if (!cats.contains(header.category, true))
                cats.add(header.category);
        }
        return cats;
    }
}
