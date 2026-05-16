#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace SynthortionColours
{
    static constexpr juce::uint32 kBlackArgb = 0xFF121212;
    static constexpr juce::uint32 kDarkGreyArgb = 0xFF1D1D1D;
    static constexpr juce::uint32 kMidGreyArgb = 0xFF2C2C2C;
    static constexpr juce::uint32 kLightGreyArgb = 0xFFB4B4B4;
    static constexpr juce::uint32 kPurpleArgb = 0xFF8F00FF;
    static constexpr juce::uint32 kPurpleDarkArgb = 0xFF7014B8;
    static constexpr juce::uint32 kWhiteArgb = 0xFFDEDEDE;
}

const juce::Colour BLACK = juce::Colour(SynthortionColours::kBlackArgb);
const juce::Colour DARK_GREY = juce::Colour(SynthortionColours::kDarkGreyArgb);
const juce::Colour MID_GREY = juce::Colour(SynthortionColours::kMidGreyArgb);
const juce::Colour LIGHT_GREY = juce::Colour(SynthortionColours::kLightGreyArgb);
const juce::Colour PURPLE = juce::Colour(SynthortionColours::kPurpleArgb);
const juce::Colour PURPLE_DARK = juce::Colour(SynthortionColours::kPurpleDarkArgb);
const juce::Colour WHITE = juce::Colour(SynthortionColours::kWhiteArgb);

class SynthortionLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthortionLookAndFeel();
    ~SynthortionLookAndFeel() override = default;

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

    void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
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

    void drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                         float cornerRadius, float shadowSize = kDefaultShadowSize) const;
    void drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                        const juce::Colour &glowColour, float intensity = kDefaultGlowIntensity) const;
    void drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                          float angle, bool isMouseOver, bool isMouseDown) const;

    void drawSectionPanel(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                          float cornerRadius = kSectionCornerRadius) const;

    void drawFrameLabel(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                        const juce::String &text) const;

private:
    static constexpr float kKnobReduction = 2.0f;
    static constexpr float kKnobFaceReduction = 1.5f;
    static constexpr float kKnobPointerLength = 0.7f;
    static constexpr float kKnobPointerThickness = 2.8f;
    static constexpr float kKnobCenterDotRadius = 2.0f;
    static constexpr int kKnobNumTicks = 24;
    static constexpr float kKnobTickLength = 3.0f;
    static constexpr float kKnobTickStartOffset = 2.0f;
    
    static constexpr float kSmallCornerRadius = 3.0f;
    static constexpr float kMediumCornerRadius = 4.0f;
    static constexpr float kLargeLabelCornerRadius = 4.0f;
    static constexpr float kSectionCornerRadius = 10.0f;
    static constexpr float kPopupItemCornerRadius = 4.0f;
    
    static constexpr float kDefaultShadowSize = 2.0f;
    static constexpr float kDefaultGlowIntensity = 0.5f;
    static constexpr int kGlowNumSteps = 10;
    static constexpr float kGlowStepSizeFactor = 0.1f;
    
    static constexpr float kFontHeightSmall = 9.5f;
    static constexpr float kFontHeightMedium = 11.0f;
    static constexpr float kFontHeightDefault = 12.0f;
    static constexpr float kFontKerningFactor = 0.1f;
    
    static constexpr float kComboBoxArrowSize = 3.0f;
    static constexpr float kComboBoxArrowStrokeWidth = 1.5f;
    static constexpr int kComboBoxTextPaddingLeft = 1;
    static constexpr int kComboBoxTextPaddingTop = 1;
    static constexpr int kComboBoxTextPaddingRight = 20;
    static constexpr int kComboBoxTextPaddingBottom = 2;
    
    static constexpr float kToggleCornerRadius = 4.0f;
    static constexpr float kToggleBorderWidth = 1.0f;
    static constexpr float kToggleGlowExpansion = 1.0f;
    static constexpr float kToggleGlowWidth = 2.0f;
    
    static constexpr float kPopupBorderWidth = 1.0f;
    static constexpr float kPopupItemReduction = 2.0f;
    static constexpr float kPopupTickRadius = 14.0f;
    static constexpr float kPopupTickPadding = 4.0f;
    static constexpr float kPopupTickSpacing = 4.0f;
    static constexpr float kPopupTextPadding = 10.0f;
    
    static constexpr float kButtonReduction = 0.5f;
    static constexpr float kButtonMixFactor = 0.15f;
    static constexpr float kButtonBrightenFactor = 0.2f;
    static constexpr float kButtonDarkenFactor = 0.2f;
    
    static constexpr float kLabelLedWidth = 14.0f;
    static constexpr float kLabelLedPadding = 3.0f;
    static constexpr float kLabelLedBorderWidth = 1.0f;
    
    static constexpr float kSectionPanelReduction1 = 0.5f;
    static constexpr float kSectionPanelReduction2 = 2.0f;
    
    static inline const juce::String kFontStyleBold{"Bold"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthortionLookAndFeel)
};