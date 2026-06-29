#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Synthortion/PanelComponent.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/AnalogLookAndFeel.h"

//==============================================================================
class PanelComponentTest : public juce::UnitTest
{
public:
    PanelComponentTest()
        : juce::UnitTest("PanelComponent", "Synthortion")
    {
    }

    void runTest() override
    {
        beginTest("PanelComponent fills opaque background");

        synthortion::PanelComponent panel("TEST", false);
        panel.setSize(100, 100);

        auto image = juce::Image(juce::Image::ARGB, 100, 100, true);
        juce::Graphics g(image);

        panel.paint(g);

        auto centrePixel = image.getPixelAt(50, 50);
        expect(centrePixel.getAlpha() == 255, "Centre pixel should be fully opaque");

        auto cornerPixel = image.getPixelAt(0, 0);
        expect(cornerPixel.getAlpha() == 255, "Corner pixel should be fully opaque after fillAll");
    }
};

static PanelComponentTest panelComponentTest;

//==============================================================================
class BypassComponentTest : public juce::UnitTest
{
public:
    BypassComponentTest()
        : juce::UnitTest("BypassComponent", "Synthortion")
    {
    }

    void runTest() override
    {
        beginTest("BypassComponent initial state");

        juce::AudioProcessorValueTreeState dummyApvts(
            nullptr, nullptr, "dummy",
            { std::make_unique<juce::AudioParameterBool>("PLUGIN_BYPASS", "Bypass", false) });

        synthortion::BypassComponent bypass(dummyApvts);
        bypass.setSize(200, 24);

        // Initial LED should be dark
        auto image = juce::Image(juce::Image::ARGB, 200, 24, true);
        juce::Graphics g(image);

        bypass.paint(g);

        beginTest("BypassComponent toggle state");
        // Find the toggle button child and click it
        for (auto* child : bypass.getChildren())
        {
            if (auto* btn = dynamic_cast<juce::ToggleButton*>(child))
            {
                btn->setToggleState(true, juce::sendNotification);
                expect(btn->getToggleState() == true);

                auto image2 = juce::Image(juce::Image::ARGB, 200, 24, true);
                juce::Graphics g2(image2);
                bypass.paint(g2);

                break;
            }
        }
    }
};

static BypassComponentTest bypassComponentTest;

//==============================================================================
class AnalogLookAndFeelFontTest : public juce::UnitTest
{
public:
    AnalogLookAndFeelFontTest()
        : juce::UnitTest("AnalogLookAndFeel Fonts", "Synthortion")
    {
    }

    void runTest() override
    {
        beginTest("getTypefaceForFont returns BebasNeue for Bebas font name");

        AnalogLookAndFeel lf;

        juce::Font bebasFont(juce::FontOptions().withName("BebasNeue").withHeight(18.0f));
        auto bebasTypeface = lf.getTypefaceForFont(bebasFont);
        expect(bebasTypeface != nullptr, "BebasNeue typeface should not be null");

        beginTest("getTypefaceForFont returns Montserrat for default sans-serif");
        juce::Font defaultFont(juce::FontOptions().withName(juce::Font::getDefaultSansSerifFontName()).withHeight(13.0f));
        auto montTypeface = lf.getTypefaceForFont(defaultFont);
        expect(montTypeface != nullptr, "Montserrat typeface should not be null");
    }
};

static AnalogLookAndFeelFontTest analogLookAndFeelFontTest;
