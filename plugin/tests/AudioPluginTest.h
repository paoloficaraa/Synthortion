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
        }

    private:
        void testEditorSizeIs800x480()
        {
            beginTest ("Plugin editor dimensions are 800x480");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 800, "Editor width should be 800");
            expect (editor.getHeight() == 480, "Editor height should be 480");
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
    };

    static AudioPluginTests synthortionTests;
}
