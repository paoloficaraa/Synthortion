#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Fire-inspired color constants
const juce::Colour FIRE_PRIMARY = juce::Colour(255, 165, 0);     // Orange primary
const juce::Colour FIRE_ACCENT = juce::Colour(255, 69, 0);       // Red-orange accent
const juce::Colour FIRE_DARK = juce::Colour(25, 25, 25);         // Dark background
const juce::Colour FIRE_MID = juce::Colour(40, 40, 40);          // Mid-tone
const juce::Colour FIRE_LIGHT = juce::Colour(200, 200, 200);     // Light text
const juce::Colour FIRE_SHADOW = juce::Colour(10, 10, 10);       // Shadow color
const juce::Colour FIRE_HIGHLIGHT = juce::Colour(255, 200, 100); // Highlight

class SynthortionLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthortionLookAndFeel();
    ~SynthortionLookAndFeel() override;

    void drawRotarySlider(juce::Graphics &, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider &) override;

    void drawLabel(juce::Graphics &g, juce::Label &label) override;

    void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox &box) override;

    void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override;

    void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                              const juce::Colour &backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override;

    juce::Font getLabelFont(juce::Label &label) override;
    juce::Font getComboBoxFont(juce::ComboBox &box) override;
    juce::Font getPopupMenuFont() override;

    // Fire-inspired helper methods
    void drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                         float cornerRadius, float shadowSize = 2.0f);
    void drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                        const juce::Colour &glowColour, float intensity = 0.5f);
    void drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                          float angle, bool isMouseOver, bool isMouseDown);

    // Scale factor for responsive design
    float scale = 1.0f;

private:
    juce::Colour baseColour = juce::Colour::fromFloatRGBA(0.1f, 0.12f, 0.16f, 1.0f);
    juce::Colour highlightColour = juce::Colour::fromFloatRGBA(0.2f, 0.22f, 0.26f, 1.0f);
    juce::Colour accentColour = juce::Colours::orange;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthortionLookAndFeel)
};