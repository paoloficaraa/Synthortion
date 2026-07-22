#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class SynthortionLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthortionLookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

    enum ColourIds
    {
        backgroundColourId     = 0x1501001,
        panelFillColourId      = 0x1501002,
        panelOutlineColourId   = 0x1501003,
        accentColourId         = 0x1501004,
        accentBrightColourId   = 0x1501005,
        textColourId           = 0x1501006,
        knobFillColourId       = 0x1501007,
        surfaceAltColourId     = 0x1501008,
        dimmedColourId         = 0x1501009,
    };

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool isHighlighted, bool isDown) override;

    void drawPanelBackground(juce::Graphics&, const juce::Rectangle<int>&,
                             bool isRecessed, const juce::String& title);

    void drawPanelBackground(juce::Graphics&, const juce::Rectangle<int>&,
                             bool isRecessed, const juce::String& title,
                             const juce::Colour& bgColour);

    void drawPanelBackground(juce::Graphics&, const juce::Rectangle<int>&,
                             bool isRecessed, const juce::String& title,
                             const juce::Colour& bgColour,
                             float titleFontHeight, float titleInset);

    juce::Font getSectionHeadingFont() const noexcept { return sectionHeadingFont; }
    juce::Font getParameterLabelFont() const noexcept { return parameterLabelFont; }
    juce::Font getParameterValueFont() const noexcept { return parameterValueFont; }
    juce::Font getBypassLabelFont() const noexcept { return bypassLabelFont; }

    /** Set the global bypass dimming mix used when drawing active elements. */
    void setBypassMix (float value) noexcept { bypassMix = juce::jlimit (0.0f, 1.0f, value); }

    /** Current bypass dimming mix in [0, 1]. */
    float getBypassMix() const noexcept { return bypassMix; }

    static constexpr float kTightKerning = -0.5f;

private:
    void drawSwitchHandle(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                          bool isOn, bool isHighlighted, bool isDown) const;
    void drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                          const juce::String& title) const;

    void drawElevationShadow(juce::Graphics& g, const juce::Rectangle<float>& knobBounds) const;

    void drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                          const juce::String& title,
                          float titleFontHeight, float titleInset) const;

    void drawTwinShadow(juce::Graphics& g, const juce::Rectangle<float>& knobBounds) const;

    void drawKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                  float knobAngle, float sliderPos,
                  float rotaryStartAngle, float rotaryEndAngle, int steps,
                  bool isHovering, float dragGlowMix, float detentPulseProgress,
                  bool withOutline) const;

    void drawKnobCap(juce::Graphics& g, const juce::Rectangle<float>& knobBounds,
                     bool withOutline) const;
    void drawSegmentedArc(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          int steps, float dragGlowMix) const;
    void drawPointer(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                     float knobAngle, float dragGlowMix, float detentPulseProgress) const;
    void drawHoverRingGlow(juce::Graphics& g, const juce::Rectangle<float>& knobBounds) const;

    juce::Typeface::Ptr bebasNeueTypeface;
    juce::Typeface::Ptr montserratTypeface;

    juce::Font sectionHeadingFont;
    juce::Font parameterLabelFont;
    juce::Font parameterValueFont;
    juce::Font bypassLabelFont;
    float bypassMix = 0.0f;

    static constexpr float kKnobReduction = 4.0f;
    static constexpr float kKnobShadowOffset = 2.0f;
    static constexpr float kKnobPointerThickness = 3.0f;
    static constexpr float kKnobPointerLength = 0.78f;
    static constexpr float kKnobArcInset = 3.0f;
    static constexpr float kKnobArcThickness = 2.5f;
    static constexpr float kKnobArcHaloThickness = 5.5f;
    static constexpr float kKnobArcHaloAlpha = 0.35f;
    static constexpr float kKnobHoverRingAlpha = 0.4f;
    static constexpr float kKnobDetentHaloRadius = 4.0f;
    static constexpr float kKnobDetentHaloAlpha = 0.6f;
    static constexpr float kKnobCapHighlight = 0.22f;

    static constexpr float kSwitchWidth = 36.0f;
    static constexpr float kSwitchHeight = 18.0f;
    static constexpr float kSwitchCornerRadius = 3.0f;

    static constexpr float kSectionTitleHeight = 22.0f;
    static constexpr float kSectionTitleInset = 8.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthortionLookAndFeel)
};
