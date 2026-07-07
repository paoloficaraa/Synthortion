#pragma once

#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Shared opaque wrapper for the DISTORTION, MODULATION and GAIN sections. */
    class PanelComponent final : public juce::Component
    {
    public:
        PanelComponent (const juce::String& title, const juce::Colour& bgColour);

        void paint (juce::Graphics& g) override;
        void resized() override;

        const juce::String& getTitle() const noexcept { return title; }
        juce::Colour getBackgroundColour() const noexcept { return bgColour; }
        juce::Font getTitleFont() const noexcept { return headingFont; }

    private:
        juce::String title;
        juce::Colour bgColour;
        juce::Font headingFont;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelComponent)
    };
}
