#pragma once

#include <JuceHeader.h>
#include "Synthortion/AnalogLookAndFeel.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/PanelComponent.h"
#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AnalogLookAndFeelTests : public juce::UnitTest
{
public:
    AnalogLookAndFeelTests()
        : juce::UnitTest("AnalogLookAndFeel", "Synthortion")
    {
    }

    void runTest() override
    {
        const juce::MessageManagerLock mmLock;

        testBebasNeueFontRouting();
        testMontserratFontRouting();
        testTypographyScale();
    }

private:
    void testBebasNeueFontRouting()
    {
        beginTest("BebasNeue font routing returns a non-null typeface");

        AnalogLookAndFeel lookAndFeel;
        auto typeface = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue"));

        expect(typeface != nullptr, "Expected non-null BebasNeue typeface");
    }

    void testMontserratFontRouting()
    {
        beginTest("Default sans-serif font routing returns Montserrat typeface");

        AnalogLookAndFeel lookAndFeel;
        auto montserrat = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat"));
        auto defaultSans = lookAndFeel.getTypefaceForFont(
            juce::FontOptions().withName(juce::Font::getDefaultSansSerifFontName()));

        expect(montserrat != nullptr, "Expected non-null Montserrat typeface");
        expect(defaultSans == montserrat, "Expected default sans-serif to resolve to Montserrat");
    }

    void testTypographyScale()
    {
        beginTest("Typography scale fonts are pre-computed and resolve correctly");

        AnalogLookAndFeel lookAndFeel;

        auto heading = lookAndFeel.getSectionHeadingFont();
        expect (juce::roundToInt(heading.getHeight()) == 18, "Section heading should be 18px");
        expect (lookAndFeel.getTypefaceForFont(heading) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue")),
                "Section heading should resolve to the BebasNeue typeface");

        auto paramLabel = lookAndFeel.getParameterLabelFont();
        expect (juce::roundToInt(paramLabel.getHeight()) == 13, "Parameter label should be 13px");
        expect (paramLabel.getTypefaceStyle() == "Medium", "Parameter label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(paramLabel) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter label should resolve to the Montserrat typeface");

        auto paramValue = lookAndFeel.getParameterValueFont();
        expect (juce::roundToInt(paramValue.getHeight()) == 12, "Parameter value should be 12px");
        expect (paramValue.getTypefaceStyle() == "Regular", "Parameter value should be Regular weight");
        expect (lookAndFeel.getTypefaceForFont(paramValue) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter value should resolve to the Montserrat typeface");

        auto bypass = lookAndFeel.getBypassLabelFont();
        expect (juce::roundToInt(bypass.getHeight()) == 13, "Bypass label should be 13px");
        expect (bypass.getTypefaceStyle() == "Medium", "Bypass label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(bypass) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Bypass label should resolve to the Montserrat typeface");
    }
};

static AnalogLookAndFeelTests analogLookAndFeelTests;

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
            testPanelComponentRendersBackgroundColour();
            testBypassComponentLedTogglesOnClick();
            testPanelComponentTitleFont();
            testEditorIsOpaque();
            testPanelComponentIsOpaque();
            testBypassComponentIsNotOpaque();
            testCopperColorPalette();
        }

    private:
        void testPanelComponentRendersBackgroundColour()
        {
            beginTest ("PanelComponent renders an opaque background colour");

            AnalogLookAndFeel lookAndFeel;

            const juce::Colour bgColour (0xFF123456);
            PanelComponent panel ("DISTORTION", bgColour);
            panel.setSize (100, 100);
            panel.setLookAndFeel (&lookAndFeel);

            const auto snapshot = panel.createComponentSnapshot (panel.getLocalBounds());

            expect (snapshot.getPixelAt (50, 50) == bgColour,
                    "PanelComponent centre pixel should match the background colour");
            expect (snapshot.getPixelAt (0, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (0, 99).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 99).getAlpha() == 0xFF);
        }

        void testBypassComponentLedTogglesOnClick()
        {
            beginTest ("BypassComponent LED toggles on programmatic click");

            BypassComponent bypass;
            bypass.setSize (200, 24);

            expect (! bypass.isLedOn());

            bypass.getToggleButton().triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (bypass.isLedOn());
            expect (bypass.getToggleButton().getToggleState());
        }

        void testPanelComponentTitleFont()
        {
            beginTest ("PanelComponent title font is BebasNeue 18px");

            PanelComponent panel ("DISTORTION", juce::Colours::black);
            auto font = panel.getTitleFont();

            expect (juce::roundToInt(font.getHeight()) == 18, "Panel title should be 18px");
            expect (font.getTypefaceName().containsIgnoreCase("Bebas"), "Panel title should use BebasNeue");
        }

        void testEditorIsOpaque()
        {
            beginTest ("Plugin editor is opaque");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.isOpaque(), "Editor should be opaque for rendering performance");
        }

        void testPanelComponentIsOpaque()
        {
            beginTest ("PanelComponent is opaque");

            PanelComponent panel ("DISTORTION", juce::Colours::black);

            expect (panel.isOpaque(), "PanelComponent wrapper should be opaque for rendering performance");
        }

        void testBypassComponentIsNotOpaque()
        {
            beginTest ("BypassComponent stays non-opaque for LED glow");

            BypassComponent bypass;

            expect (! bypass.isOpaque(), "BypassComponent should remain non-opaque to preserve semi-transparent LED glow");
        }

        void testCopperColorPalette()
        {
            beginTest ("Color palette uses saturated violet and warm magenta");

            AnalogLookAndFeel lookAndFeel;

            const auto copper = lookAndFeel.findColour (AnalogLookAndFeel::copperAccentColourId);
            const auto copperBright = lookAndFeel.findColour (AnalogLookAndFeel::copperBrightColourId);

            expect (copper == juce::Colour (0xFF7C3AED), "COPPER should be #7C3AED");
            expect (copperBright == juce::Colour (0xFFFF2D78), "COPPER_BRIGHT should be #FF2D78");
        }
    };

    static AudioPluginTests synthortionTests;
}
