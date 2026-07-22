#pragma once

#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Shared opaque wrapper for the DISTORTION, MODULATION and GAIN sections. */
    class PanelComponent final : public juce::Component,
                                 private juce::Timer
    {
    public:
        PanelComponent (const juce::String& title, const juce::Colour& bgColour);
        ~PanelComponent() override;

        void paint (juce::Graphics& g) override;
        void resized() override;

        const juce::String& getTitle() const noexcept { return title; }
        juce::Colour getBackgroundColour() const noexcept { return bgColour; }
        juce::Font getTitleFont() const noexcept { return headingFont; }

        void setPlaceholder (bool isPlaceholder) noexcept;
        void setGlitchOverlay (GlitchOverlay* overlay) noexcept { glitchOverlay = overlay; }
        void setHeadingStyle (float fontHeight, float inset) noexcept;

        float getCursorUnderlineAlpha() const noexcept;
        void advanceBlinkTick (int ticks) noexcept;

        static constexpr int cursorBlinkPeriodTicksForTests() noexcept { return kCursorBlinkPeriodTicks; }
        static constexpr int cursorBlinkHalfPeriodTicksForTests() noexcept { return kCursorBlinkHalfPeriodTicks; }

    private:
        void timerCallback() override;
        void drawPlaceholderContent (juce::Graphics& g);

        juce::String title;
        juce::Colour bgColour;
        juce::Font headingFont;
        float headingInset = 8.0f;
        GlitchOverlay* glitchOverlay = nullptr;
        int cursorBlinkTick = 0;
        bool isComingSoonPlaceholder = false;

        static constexpr int kCursorBlinkTimerHz = 60;
        static constexpr int kCursorBlinkPeriodTicks = 60;     // 1000 ms full fade cycle at 60 Hz
        static constexpr int kCursorBlinkHalfPeriodTicks = 30; // 500 ms fade in / fade out per issue #34
        static constexpr float kCursorWidth = 40.0f;
        static constexpr float kCursorHeight = 2.0f;
        static constexpr float kCursorGap = 6.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelComponent)
    };
}
