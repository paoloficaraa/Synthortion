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
            testEditorContainsEightRotarySliders();
            testEditorContainsLabels();
            testEditorContainsBypassToggle();
            testEditorSliderAttachmentFollowsApvtsParameter();
            testEditorBypassToggleFollowsApvtsParameter();
            testEditorPaintRendersDarkGreyBackground();
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

        void testEditorContainsEightRotarySliders()
        {
            beginTest ("Plugin editor contains eight rotary sliders");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int sliderCount = 0;
            int rotarySliderCount = 0;
            for (auto* child : editor.getChildren())
            {
                if (auto* slider = dynamic_cast<juce::Slider*> (child))
                {
                    ++sliderCount;
                    if (slider->getSliderStyle() == juce::Slider::Rotary)
                        ++rotarySliderCount;
                }
            }

            expect (sliderCount == 8, "Editor should contain eight Sliders");
            expect (rotarySliderCount == 8, "All editor sliders should be in Rotary style");
        }

        void testEditorContainsLabels()
        {
            beginTest ("Plugin editor contains title and slider labels");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int labelCount = 0;
            for (auto* child : editor.getChildren())
                if (dynamic_cast<juce::Label*> (child) != nullptr)
                    ++labelCount;

            expect (labelCount == 10, "Editor should contain ten Labels (title + 8 slider titles + COMING SOON)");
        }

        void testEditorContainsBypassToggle()
        {
            beginTest ("Plugin editor contains a stock JUCE bypass ToggleButton");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* btn = editor.findBypassButton();
            expect (btn != nullptr, "Editor should contain a bypass ToggleButton");
        }

        void testEditorSliderAttachmentFollowsApvtsParameter()
        {
            beginTest ("Plugin editor slider attachments respond to APVTS parameter changes");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* colorParam = processor.getAPVTS().getParameter ("COLOR");
            jassert (colorParam != nullptr);
            colorParam->setValueNotifyingHost (0.0f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

            colorParam->setValueNotifyingHost (0.75f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

            const double expected = processor.getAPVTS().getParameterRange ("COLOR").convertFrom0to1 (0.75);
            bool anySliderFollowed = false;
            for (auto* child : editor.getChildren())
                if (auto* slider = dynamic_cast<juce::Slider*> (child))
                    if (std::abs (slider->getValue() - expected) < 0.1)
                        anySliderFollowed = true;

            expect (anySliderFollowed,
                    "At least one editor slider should follow the COLOR parameter via SliderAttachment");
        }

        void testEditorBypassToggleFollowsApvtsParameter()
        {
            beginTest ("Plugin editor bypass ToggleButton is attached to PLUGIN_BYPASS");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* btn = editor.findBypassButton();
            expect (btn != nullptr, "Editor should expose a bypass ToggleButton");
            if (btn == nullptr) return;

            expect (! btn->getToggleState());

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (1.0f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (btn->getToggleState(), "Button should follow PLUGIN_BYPASS parameter turning on");

            btn->triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (processor.getAPVTS().getRawParameterValue ("PLUGIN_BYPASS")->load() < 0.5f,
                    "PLUGIN_BYPASS parameter should follow button click turning off");
        }

        void testEditorPaintRendersDarkGreyBackground()
        {
            beginTest ("Plugin editor paints a darkgrey background");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            editor.repaint();
            const auto snapshot = editor.createComponentSnapshot (editor.getLocalBounds());

            const auto centre = snapshot.getPixelAt (editor.getWidth() / 2, editor.getHeight() / 2);
            expect (centre.getRed() < 100 && centre.getGreen() < 100 && centre.getBlue() < 100,
                    "Editor centre should render a dark background");
        }
    };

    static AudioPluginTests synthortionTests;
}
