#pragma once

#include <gin_plugin/gin_plugin.h>

class AnalogLookAndFeel : public gin::CopperLookAndFeel
{
public:
    AnalogLookAndFeel();

    enum ColourIds
    {
        backgroundColourId     = 0x1501001,
        panelColourId          = 0x1501002,
        panelRecessedColourId  = 0x1501003,
        copperAccentColourId   = 0x1501004,
        copperBrightColourId   = 0x1501005,
        creamTextColourId      = 0x1501006,
        graphiteKnobColourId  = 0x1501007,
    };

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool isHighlighted, bool isDown) override;

    void drawPanelBackground(juce::Graphics&, const juce::Rectangle<int>&,
                             bool isRecessed, const juce::String& title);

private:
    void draw3DKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                   float angle, float sliderPos, bool isMouseOver, bool isMouseDown,
                   float rotaryStartAngle, float rotaryEndAngle) const;
    void drawCopperRim(juce::Graphics& g, const juce::Rectangle<float>& bounds) const;
    void drawLEDArc(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                    float startAngle, float endAngle, float sliderPos) const;
    void drawSwitchHandle(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                          bool isOn, bool isHighlighted, bool isDown) const;

    static constexpr float kKnobReduction = 2.0f;
    static constexpr float kKnobFaceReduction = 1.5f;
    static constexpr float kKnobPointerLength = 0.65f;
    static constexpr float kKnobPointerThickness = 2.5f;
    static constexpr float kKnobCenterDotRadius = 2.0f;
    static constexpr int kKnobNumTicks = 12;
    static constexpr float kKnobTickLength = 3.0f;
    static constexpr float kKnobTickStartOffset = 2.0f;

    static constexpr float kSwitchWidth = 36.0f;
    static constexpr float kSwitchHeight = 18.0f;
    static constexpr float kSwitchCornerRadius = 3.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogLookAndFeel)
};