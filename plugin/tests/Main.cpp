#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Synthortion/WarmDistortion.h"
#include "Synthortion/Bitcrusher.h"
#include "Synthortion/PingPongDelay.h"
#include "Synthortion/SynthortionChorus.h"
#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/PresetManager.h"
#include "Synthortion/PluginProcessor.h"
#include <iostream>
#include <BinaryData.h>

class DspSanityTests final : public juce::UnitTest
{
public:
    DspSanityTests() : juce::UnitTest("DSP Modules Sanity", "Synthortion") {}

    void runTest() override
    {
        beginTest("WarmDistortion prepare & process");
        {
            synthortion::dsp::WarmDistortion dist;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            dist.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 256; ++i)
                    buffer.setSample(ch, i, std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * (float)i / 48000.0f));

            synthortion::dsp::WarmDistortionParams params;
            params.drive = 0.5f;
            params.volumeCompensation = false;

            dist.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("BitCrusher prepare & process");
        {
            synthortion::dsp::BitCrusher bc;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            bc.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 256; ++i)
                    buffer.setSample(ch, i, 0.5f);

            synthortion::dsp::BitCrusherParams params;
            params.crush = 0.5f;

            bc.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("PingPongDelay prepare & process");
        {
            synthortion::dsp::PingPongDelay delay;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            delay.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);

            synthortion::dsp::PingPongDelayParams params;
            params.mix = 0.5f;
            params.feedback = 0.4f;
            params.delayTimeMs = 250.0f;
            params.dampingFrequency = 12000.0f;

            delay.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("SynthortionChorus prepare & process");
        {
            synthortion::dsp::SynthortionChorus chorus;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            chorus.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);

            synthortion::dsp::ChorusParams params;
            params.mix = 0.5f;
            params.width = 0.5f;

            chorus.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("SpectrumAnalyzer prepare & process");
        {
            synthortion::SpectrumAnalyzer analyzer;
            analyzer.prepare(48000.0, 60.0f);

            std::array<float, synthortion::SpectrumAnalyzer::kFftSize> input;
            input.fill(0.1f);
            const auto& mags = analyzer.process(input.data(), synthortion::SpectrumAnalyzer::kFftSize);
            expect(mags.size() == synthortion::SpectrumAnalyzer::kNumBands);
        }
    }
};

class PresetJsonTests final : public juce::UnitTest
{
public:
    PresetJsonTests() : juce::UnitTest("Preset JSON Schema & Parser", "Synthortion") {}

    void runTest() override
    {
        beginTest("Valid JSON parsing & schema 1 compliance");
        {
            const juce::String validJson = R"({
                "schemaVersion": 1,
                "metadata": {
                    "name": "Cyber Acid Lead",
                    "category": "Lead",
                    "author": "Synthortion Core",
                    "description": "Aggressive resonant overdrive with ping-pong tape flutter.",
                    "tags": ["Lead", "Acid", "Distortion", "Cyberpunk"],
                    "favorite": true,
                    "createdAt": "2026-08-28T12:00:00Z",
                    "modifiedAt": "2026-08-28T12:30:00Z"
                },
                "parameters": {
                    "INPUT_GAIN": 0.833333,
                    "OUTPUT_GAIN": 0.833333,
                    "COLOR": 0.65,
                    "BITCRUSH": 0.25,
                    "DELAY_TIME_FREE": 0.124562,
                    "DELAY_TIME_SYNC": 0.307692,
                    "DELAY_MIX": 0.35,
                    "DELAY_FEEDBACK": 0.45,
                    "CHORUS_MIX": 0.20,
                    "CHORUS_WIDE": 0.75,
                    "PLUGIN_BYPASS": 0.0,
                    "DRIVE_ON": 1.0,
                    "BITCRUSH_ON": 1.0,
                    "DELAY_ON": 1.0,
                    "CHORUS_ON": 1.0,
                    "DRIVE_ROUTE": 0.0,
                    "DELAY_SYNC": 1.0
                },
                "uiPreferences": {
                    "uiScale": 1.25,
                    "spectrumDecay": 0.30,
                    "skipBootSequence": true
                }
            })";

            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(validJson, data);
            expect(result.success, "Parsing valid JSON should succeed");
            expectEquals(data.schemaVersion, 1);
            expectEquals(data.metadata.name, juce::String("Cyber Acid Lead"));
            expectEquals(data.metadata.category, juce::String("Lead"));
            expectEquals(data.metadata.author, juce::String("Synthortion Core"));
            expectEquals(data.metadata.description, juce::String("Aggressive resonant overdrive with ping-pong tape flutter."));
            expectEquals(data.metadata.tags.size(), 4);
            expect(data.metadata.favorite);
            expectEquals(data.metadata.createdAt, juce::String("2026-08-28T12:00:00Z"));
            expectEquals(data.metadata.modifiedAt, juce::String("2026-08-28T12:30:00Z"));

            expect(data.parameters.find("INPUT_GAIN") != data.parameters.end());
            expectWithinAbsoluteError(data.parameters["INPUT_GAIN"], 0.833333f, 0.0001f);
            expectWithinAbsoluteError(data.parameters["COLOR"], 0.65f, 0.0001f);
            expectWithinAbsoluteError(data.parameters["DRIVE_ON"], 1.0f, 0.0001f);

            expect(data.uiPreferences.find("uiScale") != data.uiPreferences.end());
            expectWithinAbsoluteError(static_cast<double>(data.uiPreferences["uiScale"]), 1.25, 0.0001);
            expect(static_cast<bool>(data.uiPreferences["skipBootSequence"]));
        }

        beginTest("Missing optional metadata defaults gracefully");
        {
            const juce::String minimalJson = R"({
                "schemaVersion": 1,
                "metadata": {
                    "name": "Minimal Preset",
                    "category": "Bass"
                },
                "parameters": {
                    "COLOR": 0.5
                }
            })";

            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(minimalJson, data);
            expect(result.success);
            expectEquals(data.metadata.name, juce::String("Minimal Preset"));
            expectEquals(data.metadata.category, juce::String("Bass"));
            expectEquals(data.metadata.author, juce::String("User"));
            expectEquals(data.metadata.description, juce::String());
            expectEquals(data.metadata.tags.size(), 0);
            expect(!data.metadata.favorite);
        }

        beginTest("Parameter values clamped to [0.0, 1.0]");
        {
            const juce::String outOfRangeJson = R"({
                "schemaVersion": 1,
                "metadata": { "name": "Clamping Test", "category": "FX" },
                "parameters": {
                    "COLOR": 1.75,
                    "BITCRUSH": -0.5
                }
            })";

            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(outOfRangeJson, data);
            expect(result.success);
            expectWithinAbsoluteError(data.parameters["COLOR"], 1.0f, 0.0001f);
            expectWithinAbsoluteError(data.parameters["BITCRUSH"], 0.0f, 0.0001f);
        }

        beginTest("Forward compatibility: Unrecognized keys ignored");
        {
            const juce::String extraKeysJson = R"({
                "schemaVersion": 1,
                "metadata": {
                    "name": "Future Preset",
                    "category": "Experimental",
                    "futureMetadataField": "shouldBeIgnored"
                },
                "parameters": {
                    "COLOR": 0.8,
                    "FUTURE_SYNTH_PARAM_X": 0.42
                },
                "unknownTopLevelSection": {
                    "data": [1, 2, 3]
                }
            })";

            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(extraKeysJson, data);
            expect(result.success);
            expectEquals(data.metadata.name, juce::String("Future Preset"));
            expectWithinAbsoluteError(data.parameters["COLOR"], 0.8f, 0.0001f);
        }

        beginTest("Corrupted JSON rejected");
        {
            const juce::String corruptedJson = "{ schemaVersion: 1, metadata: { name: broken ";
            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(corruptedJson, data);
            expect(!result.success);
            expectEquals(static_cast<int>(result.code), static_cast<int>(synthortion::PresetErrorCode::InvalidJson));
        }

        beginTest("Schema mismatch rejected");
        {
            const juce::String futureSchemaJson = R"({
                "schemaVersion": 99,
                "metadata": { "name": "Version 99", "category": "Init" },
                "parameters": {}
            })";
            synthortion::PresetData data;
            auto result = synthortion::PresetData::parseJson(futureSchemaJson, data);
            expect(!result.success);
            expectEquals(static_cast<int>(result.code), static_cast<int>(synthortion::PresetErrorCode::SchemaMismatch));
        }

        beginTest("Missing required name or category rejected");
        {
            const juce::String noNameJson = R"({
                "schemaVersion": 1,
                "metadata": { "category": "Lead" },
                "parameters": {}
            })";
            synthortion::PresetData data1;
            auto result1 = synthortion::PresetData::parseJson(noNameJson, data1);
            expect(!result1.success);

            const juce::String emptyNameJson = R"({
                "schemaVersion": 1,
                "metadata": { "name": "", "category": "Lead" },
                "parameters": {}
            })";
            synthortion::PresetData data2;
            auto result2 = synthortion::PresetData::parseJson(emptyNameJson, data2);
            expect(!result2.success);
        }

        beginTest("Serialization and deserialization roundtrip");
        {
            synthortion::PresetData original;
            original.schemaVersion = 1;
            original.metadata.name = "Roundtrip Test";
            original.metadata.category = "Pad";
            original.metadata.author = "Author X";
            original.metadata.description = "Test Description";
            original.metadata.tags = { "Pad", "Warm", "Stereo" };
            original.metadata.favorite = true;
            original.metadata.createdAt = "2026-08-28T10:00:00Z";
            original.metadata.modifiedAt = "2026-08-28T11:00:00Z";
            original.parameters["INPUT_GAIN"] = 0.5f;
            original.parameters["COLOR"] = 0.75f;
            original.parameters["DELAY_MIX"] = 0.25f;
            original.uiPreferences["uiScale"] = 1.1;
            original.uiPreferences["spectrumDecay"] = 0.35;
            original.uiPreferences["skipBootSequence"] = false;

            juce::String jsonStr = original.toJsonString(true);
            expect(jsonStr.isNotEmpty());

            synthortion::PresetData parsed;
            auto res = synthortion::PresetData::parseJson(jsonStr, parsed);
            expect(res.success);
            expectEquals(parsed.metadata.name, original.metadata.name);
            expectEquals(parsed.metadata.category, original.metadata.category);
            expectEquals(parsed.metadata.author, original.metadata.author);
            expectEquals(parsed.metadata.description, original.metadata.description);
            expectEquals(parsed.metadata.tags.size(), original.metadata.tags.size());
            expect(parsed.metadata.favorite == original.metadata.favorite);
            expectWithinAbsoluteError(parsed.parameters["COLOR"], original.parameters["COLOR"], 0.0001f);
            expectWithinAbsoluteError(static_cast<double>(parsed.uiPreferences["uiScale"]), 1.1, 0.0001);
        }
    }
};

class PresetManagerTests final : public juce::UnitTest
{
public:
    PresetManagerTests() : juce::UnitTest("PresetManager Core & File I/O", "Synthortion") {}

    void runTest() override
    {
        beginTest("Directory hierarchy creation");
        {
            juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("Synthortion_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            tempDir.createDirectory();

            synthortion::PresetManager pm(tempDir);
            pm.ensureDirectoryHierarchy();

            for (const auto& cat : synthortion::PresetManager::getCanonicalCategories())
            {
                auto catDir = tempDir.getChildFile(cat);
                expect(catDir.isDirectory(), "Category subfolder should exist: " + cat);
            }

            tempDir.deleteRecursively();
        }

        beginTest("Filename sanitization");
        {
            expectEquals(synthortion::PresetManager::sanitizeFilename("Valid Name"), juce::String("Valid Name"));
            expectEquals(synthortion::PresetManager::sanitizeFilename("Lead: Heavy / Acid * Dirty?"), juce::String("Lead_ Heavy _ Acid _ Dirty_"));
            expectEquals(synthortion::PresetManager::sanitizeFilename("A very very very very very long preset name exceeding thirty-two characters limit"),
                         juce::String("A very very very very very long"));
        }

        beginTest("Save user preset, atomic write & duplicate protection");
        {
            juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("Synthortion_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            tempDir.createDirectory();

            synthortion::PresetManager pm(tempDir);
            pm.ensureDirectoryHierarchy();

            synthortion::PresetData preset;
            preset.metadata.name = "My Bass Patch";
            preset.metadata.category = "Bass";
            preset.metadata.author = "Tester";
            preset.metadata.description = "Thumping bass";
            preset.metadata.tags = { "Bass", "Punchy" };
            preset.parameters["INPUT_GAIN"] = 0.5f;
            preset.parameters["COLOR"] = 0.7f;

            // Save without overwrite
            auto res1 = pm.saveUserPreset(preset, false);
            expect(res1.success, "Saving new preset should succeed");

            juce::File expectedFile = tempDir.getChildFile("Bass").getChildFile("My Bass Patch.synthortionpreset");
            expect(expectedFile.existsAsFile(), "Saved file must exist on disk");

            // Attempt saving again with allowOverwrite = false -> should fail with DuplicateName
            auto res2 = pm.saveUserPreset(preset, false);
            expect(!res2.success, "Duplicate save without allowOverwrite should fail");
            expectEquals(static_cast<int>(res2.code), static_cast<int>(synthortion::PresetErrorCode::DuplicateName));

            // Attempt saving with allowOverwrite = true -> should succeed
            preset.metadata.description = "Updated description";
            auto res3 = pm.saveUserPreset(preset, true);
            expect(res3.success, "Overwriting preset when allowOverwrite = true should succeed");

            // Verify content updated
            synthortion::PresetData loadedData;
            auto loadRes = pm.loadPresetData(synthortion::PresetManager::makePresetId(false, "Bass", "My Bass Patch"), loadedData);
            expect(loadRes.success);
            expectEquals(loadedData.metadata.description, juce::String("Updated description"));

            tempDir.deleteRecursively();
        }

        beginTest("Scan user presets & build in-memory catalog");
        {
            juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("Synthortion_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            tempDir.createDirectory();

            synthortion::PresetManager pm(tempDir);
            pm.clearFactoryPresets();
            pm.ensureDirectoryHierarchy();

            // Create valid preset in Lead/
            synthortion::PresetData p1;
            p1.metadata.name = "Neon Lead";
            p1.metadata.category = "Lead";
            p1.metadata.tags = { "Lead", "Bright" };
            p1.parameters["COLOR"] = 0.9f;
            pm.saveUserPreset(p1, false);

            // Create valid preset in root (maps to category User)
            synthortion::PresetData p2;
            p2.metadata.name = "Root Preset";
            p2.metadata.category = "User";
            p2.parameters["COLOR"] = 0.3f;
            auto rootFile = tempDir.getChildFile("Root Preset.synthortionpreset");
            rootFile.replaceWithText(p2.toJsonString(true));

            // Create corrupted file in FX/
            auto corruptFile = tempDir.getChildFile("FX").getChildFile("Corrupted.synthortionpreset");
            corruptFile.replaceWithText("not a valid json {{{ broken");

            // Re-scan
            pm.scanUserPresets();

            const auto& catalog = pm.getCatalog();
            expectEquals(catalog.size(), static_cast<size_t>(2), "Corrupted preset should be skipped, 2 valid presets indexed");

            auto leadPresets = pm.getPresetsForCategory("Lead");
            expectEquals(leadPresets.size(), static_cast<size_t>(1));
            expectEquals(leadPresets[0].name, juce::String("Neon Lead"));
            expectEquals(leadPresets[0].category, juce::String("Lead"));

            auto userPresets = pm.getPresetsForCategory("User");
            expectEquals(userPresets.size(), static_cast<size_t>(1));
            expectEquals(userPresets[0].name, juce::String("Root Preset"));

            tempDir.deleteRecursively();
        }

        beginTest("Delete user preset vs factory preset protection");
        {
            juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("Synthortion_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            tempDir.createDirectory();

            synthortion::PresetManager pm(tempDir);
            pm.clearFactoryPresets();
            pm.ensureDirectoryHierarchy();
            // Register a factory preset
            synthortion::PresetData factoryP;
            factoryP.metadata.name = "Default Init";
            factoryP.metadata.category = "Init";
            factoryP.parameters["COLOR"] = 0.4f;
            pm.registerFactoryPreset(factoryP);

            // Save user preset
            synthortion::PresetData userP;
            userP.metadata.name = "Custom Pad";
            userP.metadata.category = "Pad";
            userP.parameters["COLOR"] = 0.6f;
            pm.saveUserPreset(userP, false);

            expectEquals(pm.getCatalog().size(), static_cast<size_t>(2));

            // Try to delete factory preset -> should fail
            juce::String factoryId = synthortion::PresetManager::makePresetId(true, "Init", "Default Init");
            auto delFactoryRes = pm.deleteUserPreset(factoryId);
            expect(!delFactoryRes.success, "Deleting factory preset must be blocked");
            expectEquals(static_cast<int>(delFactoryRes.code), static_cast<int>(synthortion::PresetErrorCode::CannotDeleteFactoryPreset));
            expectEquals(pm.getCatalog().size(), static_cast<size_t>(2));

            // Delete user preset -> should succeed
            juce::String userId = synthortion::PresetManager::makePresetId(false, "Pad", "Custom Pad");
            auto delUserRes = pm.deleteUserPreset(userId);
            expect(delUserRes.success, "Deleting user preset should succeed");
            expectEquals(pm.getCatalog().size(), static_cast<size_t>(1));
            expectEquals(pm.getCatalog()[0].id, factoryId);

            // Delete non-existent preset -> should fail with FileNotFound
            auto delMissingRes = pm.deleteUserPreset("user://Pad/NonExistent");
            expect(!delMissingRes.success);
            expectEquals(static_cast<int>(delMissingRes.code), static_cast<int>(synthortion::PresetErrorCode::FileNotFound));

            tempDir.deleteRecursively();
        }

        beginTest("APVTS save and load roundtrip synchronization");
        {
            juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("Synthortion_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            tempDir.createDirectory();

            synthortion::AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            // Set specific parameter values
            if (auto* color = apvts.getParameter("COLOR"))
                color->setValueNotifyingHost(0.85f);
            if (auto* bitcrush = apvts.getParameter("BITCRUSH"))
                bitcrush->setValueNotifyingHost(0.40f);
            if (auto* driveOn = apvts.getParameter("DRIVE_ON"))
                driveOn->setValueNotifyingHost(0.0f);

            // Set UI preferences in state
            auto uiPrefs = apvts.state.getChildWithName(synthortion::UIPreferences::kNodeName);
            if (uiPrefs.isValid())
            {
                uiPrefs.setProperty(synthortion::UIPreferences::kUiScale, 1.33, nullptr);
                uiPrefs.setProperty(synthortion::UIPreferences::kSpectrumDecay, 0.45, nullptr);
                uiPrefs.setProperty(synthortion::UIPreferences::kSkipBootSequence, true, nullptr);
            }

            synthortion::PresetManager pm(tempDir);
            pm.ensureDirectoryHierarchy();

            // Save active processor state to user preset
            auto saveRes = pm.saveUserPreset("Lo-Fi", "Vintage Tape", "Test Author", "Warm tape saturation", { "Lo-Fi", "Tape" }, apvts, false);
            expect(saveRes.success, "Saving APVTS state to preset should succeed");

            // Change APVTS parameters to different values
            if (auto* color = apvts.getParameter("COLOR"))
                color->setValueNotifyingHost(0.10f);
            if (auto* bitcrush = apvts.getParameter("BITCRUSH"))
                bitcrush->setValueNotifyingHost(0.90f);
            if (auto* driveOn = apvts.getParameter("DRIVE_ON"))
                driveOn->setValueNotifyingHost(1.0f);

            // Load preset back into APVTS
            juce::String presetId = synthortion::PresetManager::makePresetId(false, "Lo-Fi", "Vintage Tape");
            auto loadRes = pm.loadPreset(presetId, apvts);
            expect(loadRes.success, "Loading preset into APVTS should succeed");

            // Verify APVTS parameters are restored
            if (auto* color = apvts.getParameter("COLOR"))
                expectWithinAbsoluteError(color->getValue(), 0.85f, 0.001f);
            if (auto* bitcrush = apvts.getParameter("BITCRUSH"))
                expectWithinAbsoluteError(bitcrush->getValue(), 0.40f, 0.001f);
            if (auto* driveOn = apvts.getParameter("DRIVE_ON"))
                expectWithinAbsoluteError(driveOn->getValue(), 0.0f, 0.001f);

            // Verify UI preferences are restored in APVTS state
            auto restoredUiPrefs = apvts.state.getChildWithName(synthortion::UIPreferences::kNodeName);
            expect(restoredUiPrefs.isValid());
            expectWithinAbsoluteError(static_cast<double>(restoredUiPrefs.getProperty(synthortion::UIPreferences::kUiScale)), 1.33, 0.001);
            expectWithinAbsoluteError(static_cast<double>(restoredUiPrefs.getProperty(synthortion::UIPreferences::kSpectrumDecay)), 0.45, 0.001);
            expect(static_cast<bool>(restoredUiPrefs.getProperty(synthortion::UIPreferences::kSkipBootSequence)));

            tempDir.deleteRecursively();
        }
    }
};
class FactoryPresetAndHostProgramTests final : public juce::UnitTest
{
public:
    FactoryPresetAndHostProgramTests() : juce::UnitTest("Factory Presets & Host Program Sync", "Synthortion") {}

    void runTest() override
    {
        beginTest("BinaryData embeds all 12 factory presets");
        {
            expectEquals(BinaryData::namedResourceListSize, 12, "BinaryData must contain exactly 12 embedded files");
        }

        beginTest("Factory Preset Registry in PresetManager contains 12 valid presets");
        {
            juce::File emptyTempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                        .getChildFile("Synthortion_Factory_Tests_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64()));
            emptyTempDir.createDirectory();

            synthortion::PresetManager pm(emptyTempDir);

            expectEquals(pm.getNumFactoryPresets(), 12, "PresetManager must register exactly 12 factory presets");

            const struct ExpectedPreset {
                const char* name;
                const char* category;
                const char* id;
            } expectedPresets[12] = {
                { "Default Init", "Init", "factory://Init/Default Init" },
                { "Sub Destroyer", "Bass", "factory://Bass/Sub Destroyer" },
                { "Acid Crush", "Bass", "factory://Bass/Acid Crush" },
                { "Cyber Neon", "Lead", "factory://Lead/Cyber Neon" },
                { "Vintage Lead", "Lead", "factory://Lead/Vintage Lead" },
                { "Cassette 1984", "Lo-Fi", "factory://Lo-Fi/Cassette 1984" },
                { "Arcade Cabinet", "Lo-Fi", "factory://Lo-Fi/Arcade Cabinet" },
                { "BBD Dimension", "Pad", "factory://Pad/BBD Dimension" },
                { "Ambient Mist", "Pad", "factory://Pad/Ambient Mist" },
                { "Bit Storm", "FX", "factory://FX/Bit Storm" },
                { "Dub Chamber", "FX", "factory://FX/Dub Chamber" },
                { "Modular Chaos", "Experimental", "factory://Experimental/Modular Chaos" }
            };

            for (int i = 0; i < 12; ++i)
            {
                const auto* preset = pm.getFactoryPreset(i);
                expect(preset != nullptr, "Factory preset at index " + juce::String(i) + " must exist");
                if (preset != nullptr)
                {
                    expectEquals(preset->metadata.name, juce::String(expectedPresets[i].name));
                    expectEquals(preset->metadata.category, juce::String(expectedPresets[i].category));
                    expectEquals(preset->schemaVersion, 1);

                    // Verify all parameter values are normalized floats in [0.0, 1.0]
                    expect(preset->parameters.size() > 0, "Preset must contain parameters");
                    for (const auto& entry : preset->parameters)
                    {
                        const auto& paramId = entry.first;
                        float val = entry.second;
                        expect(val >= 0.0f && val <= 1.0f,
                               "Parameter " + paramId + " in preset " + preset->metadata.name + " must be in [0.0, 1.0] but got " + juce::String(val));
                    }
                }

                expectEquals(pm.getFactoryPresetName(i), juce::String(expectedPresets[i].name));
                expectEquals(pm.getFactoryPresetIndex(expectedPresets[i].id), i);
            }

            expectEquals(pm.getFactoryPresetIndex("user://Bass/Custom"), -1);
            expectEquals(pm.getFactoryPresetIndex("factory://Invalid/Unknown"), -1);
            expectEquals(pm.getFactoryPresetName(-1), juce::String());
            expectEquals(pm.getFactoryPresetName(12), juce::String());

            // Verify unified in-memory catalog contains all 12 factory presets in order
            const auto& catalog = pm.getCatalog();
            expectEquals(catalog.size(), static_cast<size_t>(12));
            for (int k = 0; k < 12; ++k)
            {
                expectEquals(catalog[static_cast<size_t>(k)].id, juce::String(expectedPresets[k].id));
                expect(catalog[static_cast<size_t>(k)].isFactory);
            }

            emptyTempDir.deleteRecursively();
        }

        beginTest("DAW Host Program Synchronization in AudioPluginAudioProcessor");
        {
            synthortion::AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            expectEquals(processor.getNumPrograms(), 12, "getNumPrograms() must return 12");

            // Initial program should be 0 (Default Init)
            expectEquals(processor.getCurrentProgram(), 0, "Default active program should be 0");
            expectEquals(processor.getProgramName(0), juce::String("Default Init"));
            expectEquals(processor.getProgramName(1), juce::String("Sub Destroyer"));
            expectEquals(processor.getProgramName(11), juce::String("Modular Chaos"));
            expectEquals(processor.getProgramName(12), juce::String());

            // Change program name should be a no-op
            processor.changeProgramName(0, "Attempted Rename");
            expectEquals(processor.getProgramName(0), juce::String("Default Init"), "changeProgramName must be a no-op for factory presets");

            // Switch program to 1 ("Sub Destroyer")
            processor.setCurrentProgram(1);
            expectEquals(processor.getCurrentProgram(), 1, "getCurrentProgram() should reflect program 1");
            if (auto* colorParam = apvts.getParameter("COLOR"))
            {
                expectWithinAbsoluteError(colorParam->getValue(), 0.85f, 0.001f);
            }
            if (auto* chorusWideParam = apvts.getParameter("CHORUS_WIDE"))
            {
                expectWithinAbsoluteError(chorusWideParam->getValue(), 0.0f, 0.001f);
            }

            // Switch program to 3 ("Cyber Neon")
            processor.setCurrentProgram(3);
            expectEquals(processor.getCurrentProgram(), 3);
            if (auto* colorParam = apvts.getParameter("COLOR"))
            {
                expectWithinAbsoluteError(colorParam->getValue(), 0.55f, 0.001f);
            }
            if (auto* chorusWideParam = apvts.getParameter("CHORUS_WIDE"))
            {
                expectWithinAbsoluteError(chorusWideParam->getValue(), 0.85f, 0.001f);
            }

            // Switch program to 11 ("Modular Chaos")
            processor.setCurrentProgram(11);
            expectEquals(processor.getCurrentProgram(), 11);
            if (auto* colorParam = apvts.getParameter("COLOR"))
            {
                expectWithinAbsoluteError(colorParam->getValue(), 0.95f, 0.001f);
            }

            // Setting an out-of-bounds program should not crash or change program
            processor.setCurrentProgram(-1);
            expectEquals(processor.getCurrentProgram(), 11);
            processor.setCurrentProgram(99);
            expectEquals(processor.getCurrentProgram(), 11);

            // When a user preset is loaded / active, getCurrentProgram() must return -1
            processor.getPresetManager().setActivePresetId("user://Lead/MyCustomPreset");
            expectEquals(processor.getCurrentProgram(), -1, "getCurrentProgram() must return -1 for user presets");
        }
    }
};

static FactoryPresetAndHostProgramTests factoryPresetAndHostProgramTests;

static DspSanityTests dspSanityTests;
static PresetJsonTests presetJsonTests;
static PresetManagerTests presetManagerTests;

int main(int /*argc*/, char** /*argv*/)
{
    juce::initialiseJuce_GUI();

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runTestsInCategory("Synthortion");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* res = runner.getResult(i);
        std::cout << "Test: " << res->unitTestName.toStdString() << " - Passed: " << res->passes << " Failed: " << res->failures << "\n";
        for (const auto& msg : res->messages)
        {
            std::cout << "  " << msg.toStdString() << "\n";
        }
        failures += static_cast<int>(res->failures);
    }
    std::cout << "TOTAL FAILURES: " << failures << "\n";

    juce::shutdownJuce_GUI();
    return failures == 0 ? 0 : 1;
}
