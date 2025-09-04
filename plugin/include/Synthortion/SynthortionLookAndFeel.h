#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// color constants
const juce::Colour BLACK = juce::Colour::fromString("#FF121212");
const juce::Colour DARK_GREY = juce::Colour::fromString("#FF1D1D1D");
const juce::Colour MID_GREY = juce::Colour::fromString("#FF2C2C2C");
const juce::Colour LIGHT_GREY = juce::Colour::fromString("#FFB4B4B4");
const juce::Colour PURPLE = juce::Colour::fromString("#8f00ff");
const juce::Colour PURPLE_DARK = juce::Colour::fromString("#7014b8");
const juce::Colour WHITE = juce::Colour::fromString("#FFDEDEDE");

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
    void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                           const juce::String &text, const juce::String &shortcutKeyText,
                           const juce::Drawable *icon, const juce::Colour *textColour) override;

    juce::Font getLabelFont(juce::Label &label) override;
    juce::Font getComboBoxFont(juce::ComboBox &box) override;
    juce::Font getPopupMenuFont() override;

    // Helper methods for stylised drawing
    void drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                         float cornerRadius, float shadowSize = 2.0f);
    void drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                        const juce::Colour &glowColour, float intensity = 0.5f);
    void drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                          float angle, bool isMouseOver, bool isMouseDown);

    // Section panels (rounded card-like panels with subtle border and bevel)
    void drawSectionPanel(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                          float cornerRadius = 10.0f) const;

    // Small caption chip (e.g., "EQ / FILTER")
    void drawFrameLabel(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                        const juce::String &text) const;

    // Scale factor for responsive design
    float scale = 1.0f;

private:
    juce::Colour baseCol;
    juce::Colour highlightCol;
    juce::Colour accentColour = PURPLE;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthortionLookAndFeel)
};