#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Synthortion/PanelComponent.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/AnalogLookAndFeel.h"

namespace
{
    class TestAudioProcessor : public juce::AudioProcessor
    {
    public:
        TestAudioProcessor()
            : apvts(*this, nullptr, "test",
                    { std::make_unique<juce::AudioParameterBool>("PLUGIN_BYPASS", "Bypass", false) })
        {
        }

        const juce::String getName() const override { return "Test"; }
        void prepareToPlay(double, int) override {}
        void releaseResources() override {}
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        using AudioProcessor::processBlock;
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}

        juce::AudioProcessorValueTreeState apvts;
    };
}

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

        TestAudioProcessor dummyProcessor;
        synthortion::BypassComponent bypass(dummyProcessor.apvts);
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
