#pragma once

#include "Synthortion/AnalogLookAndFeel.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/PanelComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

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
    };

    static AudioPluginTests synthortionTests;
}
