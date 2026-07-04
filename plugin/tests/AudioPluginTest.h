#pragma once

#include <JuceHeader.h>
#include "Synthortion/AnalogLookAndFeel.h"

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
};

static AnalogLookAndFeelTests analogLookAndFeelTests;
