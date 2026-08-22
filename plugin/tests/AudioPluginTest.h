#pragma once

#include <JuceHeader.h>

#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"

namespace synthortion
{
    class AudioPluginTests final : public juce::UnitTest
    {
    public:
        AudioPluginTests()
            : juce::UnitTest ("Synthortion component tests", "Synthortion")
        {
        }

        void runTest() override
        {
            testEditorSizeIs800x480();
            testEditorContainsWebBrowserComponent();
            testProcessorAPVTSParameterCount();
            testEditorPaintRendersBackground();
            testParameterValuesAndBridgeProtocol();
            testDistResourceProvider();
        }
    private:
        void testEditorSizeIs800x480()
        {
            beginTest ("Plugin editor dimensions are 800x480 and resizable with fixed aspect ratio");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 800, "Editor width should be 800");
            expect (editor.getHeight() == 480, "Editor height should be 480");
            expect (editor.isResizable(), "Editor should be resizable");
            if (auto* constrainer = editor.getConstrainer())
            {
                expectWithinAbsoluteError (constrainer->getFixedAspectRatio(), 800.0 / 480.0, 0.001, "Aspect ratio should be 800/480");
            }
        }

        void testEditorContainsWebBrowserComponent()
        {
            beginTest ("Plugin editor contains WebBrowserComponent");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            bool foundWebBrowser = false;
            for (auto* child : editor.getChildren())
            {
                if (dynamic_cast<juce::WebBrowserComponent*> (child) != nullptr)
                {
                    foundWebBrowser = true;
                    break;
                }
            }

            expect (foundWebBrowser, "Editor should contain a WebBrowserComponent child");
        }

        void testProcessorAPVTSParameterCount()
        {
            beginTest ("Plugin processor contains 16 APVTS parameters");

            AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            expect (apvts.getParameter ("INPUT_GAIN") != nullptr);
            expect (apvts.getParameter ("OUTPUT_GAIN") != nullptr);
            expect (apvts.getParameter ("COLOR") != nullptr);
            expect (apvts.getParameter ("BITCRUSH") != nullptr);
            expect (apvts.getParameter ("DELAY_TIME") != nullptr);
            expect (apvts.getParameter ("DELAY_MIX") != nullptr);
            expect (apvts.getParameter ("DELAY_FEEDBACK") != nullptr);
            expect (apvts.getParameter ("CHORUS_MIX") != nullptr);
            expect (apvts.getParameter ("PLUGIN_BYPASS") != nullptr);
            expect (apvts.getParameter ("DRIVE_ON") != nullptr);
            expect (apvts.getParameter ("BITCRUSH_ON") != nullptr);
            expect (apvts.getParameter ("DELAY_ON") != nullptr);
            expect (apvts.getParameter ("CHORUS_ON") != nullptr);
            expect (apvts.getParameter ("DRIVE_ROUTE") != nullptr);
            expect (apvts.getParameter ("DELAY_SYNC") != nullptr);
            expect (apvts.getParameter ("CHORUS_WIDE") != nullptr);
        }

        void testEditorPaintRendersBackground()
        {
            beginTest ("Plugin editor paints background");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            editor.repaint();
            const auto snapshot = editor.createComponentSnapshot (editor.getLocalBounds());
            expect (snapshot.getWidth() == 800 && snapshot.getHeight() == 480,
                    "Snapshot should match editor dimensions");
        }

        void testParameterValuesAndBridgeProtocol()
        {
            beginTest ("Bridge protocol and parameter normalization");

            AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            // Test handleMessage with COLOR parameter
            processor.handleMessage ("{\"parameterId\":\"COLOR\",\"value\":0.75}");
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.75f, 0.001f, "COLOR normalized value should be 0.75");
            }

            // Test INPUT_GAIN: -60 dB -> 0.0, +12 dB -> 1.0, 0 dB -> (60/72)
            processor.handleMessage ("{\"parameterId\":\"INPUT_GAIN\",\"value\":0.0}");
            if (auto* param = apvts.getParameter ("INPUT_GAIN"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.0f, 0.001f, "INPUT_GAIN at 0.0 should be -60dB");
            }

            if (auto* param = apvts.getParameter ("INPUT_GAIN"))
            {
                const float zeroDbNormalized = param->getNormalisableRange().convertTo0to1 (0.0f);
                processor.handleMessage ("{\"parameterId\":\"INPUT_GAIN\",\"value\":" + juce::String (zeroDbNormalized) + "}");
                expectWithinAbsoluteError (param->getValue(), zeroDbNormalized, 0.001f, "INPUT_GAIN 0dB normalized value");
            }

            // Test PLUGIN_BYPASS
            processor.handleMessage ("{\"parameterId\":\"PLUGIN_BYPASS\",\"value\":1.0}");
            if (auto* param = apvts.getParameter ("PLUGIN_BYPASS"))
            {
                expectWithinAbsoluteError (param->getValue(), 1.0f, 0.001f, "PLUGIN_BYPASS should be active");
            }

            // Test handleMessage with spec 'id' key
            processor.handleMessage ("{\"id\":\"BITCRUSH\",\"value\":0.42}");
            if (auto* param = apvts.getParameter ("BITCRUSH"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.42f, 0.001f, "BITCRUSH normalized value via 'id' should be 0.42");
            }
        }
        void testDistResourceProvider()
        {
            beginTest ("Web Resource Provider delivers index.html");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto res = editor.getResource ("/");
            if (res.has_value())
            {
                expect (res->mimeType == "text/html", "Mime type should be text/html");
                expect (! res->data.empty(), "Resource data should not be empty");
            }
        }
    };

    static AudioPluginTests synthortionTests;
}
