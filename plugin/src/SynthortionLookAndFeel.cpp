#include "Synthortion/SynthortionLookAndFeel.h"

SynthortionLookAndFeel::SynthortionLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, BLACK);
    setColour(juce::DocumentWindow::backgroundColourId, BLACK);
    setColour(juce::DialogWindow::backgroundColourId, BLACK);

    setColour(juce::Slider::backgroundColourId, MID_GREY);
    setColour(juce::Slider::thumbColourId, PURPLE);
    setColour(juce::Slider::trackColourId, PURPLE_DARK);
    setColour(juce::Slider::rotarySliderFillColourId, PURPLE);
    setColour(juce::Slider::rotarySliderOutlineColourId, MID_GREY);

    setColour(juce::TextButton::buttonColourId, MID_GREY);
    setColour(juce::TextButton::buttonOnColourId, PURPLE);
    setColour(juce::TextButton::textColourOffId, LIGHT_GREY);
    setColour(juce::TextButton::textColourOnId, BLACK);

    setColour(juce::Label::textColourId, LIGHT_GREY);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ComboBox::backgroundColourId, MID_GREY);
    setColour(juce::ComboBox::textColourId, LIGHT_GREY);
    setColour(juce::ComboBox::outlineColourId, DARK_GREY);
    setColour(juce::ComboBox::buttonColourId, DARK_GREY);
    setColour(juce::ComboBox::arrowColourId, LIGHT_GREY);
}

void SynthortionLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider &slider)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(kKnobReduction);
    const float knobAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const bool isMouseOver = slider.isMouseOverOrDragging();
    const bool isMouseDown = slider.isMouseButtonDown();

    drawGradientKnob(g, bounds, knobAngle, isMouseOver, isMouseDown);
}

void SynthortionLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        const auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        const auto font = getLabelFont(label);

        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(font);
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

void SynthortionLookAndFeel::drawComboBox(juce::Graphics &g, int width, int height, bool,
                                          int buttonX, int buttonY, int buttonW, int buttonH,
                                          juce::ComboBox &box)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, kSmallCornerRadius);

    drawInnerShadow(g, bounds, kSmallCornerRadius);

    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds.reduced(kButtonReduction), kSmallCornerRadius, kToggleBorderWidth);

    const auto arrowZone = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH).toFloat();
    g.setColour(box.findColour(juce::ComboBox::buttonColourId));
    g.fillRoundedRectangle(arrowZone, kSmallCornerRadius);

    const float arrowX = arrowZone.getCentreX();
    const float arrowY = arrowZone.getCentreY();

    juce::Path arrow;
    arrow.startNewSubPath(arrowX - kComboBoxArrowSize, arrowY - kComboBoxArrowSize * 0.5f);
    arrow.lineTo(arrowX, arrowY + kComboBoxArrowSize * 0.5f);
    arrow.lineTo(arrowX + kComboBoxArrowSize, arrowY - kComboBoxArrowSize * 0.5f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(arrow, juce::PathStrokeType(kComboBoxArrowStrokeWidth));
}

void SynthortionLookAndFeel::positionComboBoxText(juce::ComboBox &box, juce::Label &label)
{
    label.setBounds(kComboBoxTextPaddingLeft, kComboBoxTextPaddingTop, 
                   box.getWidth() - kComboBoxTextPaddingRight, 
                   box.getHeight() - kComboBoxTextPaddingBottom);
    label.setFont(getComboBoxFont(box));
}

void SynthortionLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                  const juce::Colour &backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(kButtonReduction);

    auto btnColour = shouldDrawButtonAsDown ? PURPLE_DARK : MID_GREY.interpolatedWith(backgroundColour, kButtonMixFactor);
    if (shouldDrawButtonAsHighlighted)
        btnColour = btnColour.brighter(kButtonBrightenFactor);

    g.setColour(btnColour);
    g.fillRoundedRectangle(bounds, kSmallCornerRadius);

    g.setColour(BLACK.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, kSmallCornerRadius, kToggleBorderWidth);
}

void SynthortionLookAndFeel::drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(kToggleBorderWidth);

    const bool isToggleOn = button.getToggleState();
    auto btnColour = isToggleOn ? PURPLE : MID_GREY;
    const auto textColour = isToggleOn ? BLACK : LIGHT_GREY;

    if (shouldDrawButtonAsHighlighted)
        btnColour = btnColour.brighter(0.3f);
    if (shouldDrawButtonAsDown)
        btnColour = btnColour.darker(kButtonDarkenFactor);

    g.setColour(btnColour);
    g.fillRoundedRectangle(bounds, kToggleCornerRadius);

    g.setColour(isToggleOn ? PURPLE_DARK : DARK_GREY);
    g.drawRoundedRectangle(bounds, kToggleCornerRadius, kToggleBorderWidth);

    g.setColour(textColour);
    g.setFont(juce::Font(juce::FontOptions().withHeight(kFontHeightMedium).withStyle(kFontStyleBold)));
    g.drawFittedText(button.getButtonText(), bounds.toNearestInt(),
                     juce::Justification::centred, 1);

    if (isToggleOn)
    {
        g.setColour(PURPLE.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.expanded(kToggleGlowExpansion), 
                              kToggleCornerRadius + kToggleGlowExpansion, 
                              kToggleGlowWidth);
    }
}

void SynthortionLookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    g.fillAll(DARK_GREY);
    g.setColour(PURPLE.withAlpha(0.5f));
    g.drawRect(0, 0, width, height, static_cast<int>(kPopupBorderWidth));
}

void SynthortionLookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                                               const juce::String &text, const juce::String &shortcutKeyText,
                                               const juce::Drawable *, const juce::Colour *textColour)
{
    juce::ignoreUnused(hasSubMenu);

    auto r = area.toFloat();

    if (isSeparator)
    {
        g.setColour(BLACK.withAlpha(0.4f));
        g.fillRect(r.withHeight(1.0f));
        return;
    }

    if (isHighlighted)
    {
        const auto highlightBounds = r.reduced(kPopupItemReduction);
        g.setColour(PURPLE.withAlpha(0.18f));
        g.fillRoundedRectangle(highlightBounds, kPopupItemCornerRadius);
        g.setColour(PURPLE.withAlpha(0.35f));
        g.drawRoundedRectangle(highlightBounds, kPopupItemCornerRadius, kToggleBorderWidth);
    }

    if (isTicked)
    {
        const auto tickBounds = r.removeFromLeft(kPopupTickRadius).reduced(kPopupTickPadding);
        g.setColour(PURPLE);
        g.fillEllipse(tickBounds);
        r.removeFromLeft(kPopupTickSpacing);
    }

    g.setFont(juce::FontOptions().withHeight(kFontHeightMedium));
    const auto col = textColour != nullptr ? *textColour : (isActive ? LIGHT_GREY : LIGHT_GREY.withAlpha(0.5f));
    g.setColour(col);

    const auto textArea = r.reduced(kPopupTextPadding, 0.0f).toNearestInt();
    g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour(LIGHT_GREY.withAlpha(isActive ? 0.6f : 0.3f));
        g.drawText(shortcutKeyText, textArea, juce::Justification::centredRight, true);
    }
}

juce::Font SynthortionLookAndFeel::getLabelFont(juce::Label &)
{
    auto font = juce::Font(juce::FontOptions().withHeight(kFontHeightDefault));
    font.setExtraKerningFactor(kFontKerningFactor);
    return font;
}

juce::Font SynthortionLookAndFeel::getComboBoxFont(juce::ComboBox &)
{
    return juce::Font(juce::FontOptions().withHeight(kFontHeightMedium));
}

juce::Font SynthortionLookAndFeel::getPopupMenuFont()
{
    return juce::Font(juce::FontOptions().withHeight(kFontHeightMedium));
}

void SynthortionLookAndFeel::drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                             float cornerRadius, float shadowSize) const
{
    const auto shadowBounds = bounds.reduced(kToggleBorderWidth);
    const auto shadowColour = BLACK.withAlpha(0.7f);

    juce::Path path;
    path.addRoundedRectangle(shadowBounds, cornerRadius);

    g.setColour(shadowColour);
    g.strokePath(path, juce::PathStrokeType(shadowSize, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SynthortionLookAndFeel::drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                            const juce::Colour &glowColour, float intensity) const
{
    const float stepSize = bounds.getWidth() * kGlowStepSizeFactor;

    g.setColour(glowColour.withAlpha(intensity / kGlowNumSteps));

    for (int i = 1; i <= kGlowNumSteps; ++i)
    {
        g.drawEllipse(bounds.expanded(stepSize * i), kComboBoxArrowStrokeWidth);
    }
}

// Section panels and labels
void SynthortionLookAndFeel::drawSectionPanel(juce::Graphics &g, const juce::Rectangle<float> &b, float r) const
{
    juce::ColourGradient grad(
        DARK_GREY, b.getTopLeft(),
        MID_GREY.darker(0.2f), b.getBottomRight(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(b, r);

    g.setColour(BLACK.withAlpha(0.6f));
    g.drawRoundedRectangle(b.reduced(kSectionPanelReduction1), r, kToggleBorderWidth);
    g.setColour(LIGHT_GREY.withAlpha(0.05f));
    g.drawRoundedRectangle(b.reduced(kSectionPanelReduction2), r, kToggleBorderWidth);
}

void SynthortionLookAndFeel::drawFrameLabel(juce::Graphics &g, const juce::Rectangle<float> &b, 
                                            const juce::String &text) const
{
    g.setColour(BLACK);
    g.fillRoundedRectangle(b, kLargeLabelCornerRadius);
    g.setColour(PURPLE);
    g.drawRoundedRectangle(b, kLargeLabelCornerRadius, kToggleBorderWidth);

    auto labelBounds = b;
    const auto led = labelBounds.removeFromLeft(kLabelLedWidth).reduced(kLabelLedPadding);
    g.setColour(PURPLE.withAlpha(0.9f));
    g.fillEllipse(led);
    g.setColour(PURPLE_DARK);
    g.drawEllipse(led, kLabelLedBorderWidth);

    g.setColour(LIGHT_GREY);
    g.setFont(juce::Font(juce::FontOptions().withHeight(kFontHeightMedium).withStyle(kFontStyleBold)));
    g.drawFittedText(text, labelBounds.toNearestInt(), juce::Justification::centred, 1);
}

void SynthortionLookAndFeel::drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                              float angle, bool isMouseOver, bool isMouseDown) const
{
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float diameter = radius * 2.0f;
    const auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

    g.setColour(BLACK);
    g.fillEllipse(knobBounds);

    juce::ColourGradient gradient(
        MID_GREY.brighter(0.1f), knobBounds.getTopLeft(),
        DARK_GREY, knobBounds.getBottomRight(), false);
    g.setGradientFill(gradient);
    g.fillEllipse(knobBounds.reduced(kKnobFaceReduction));

    const float tickStart = radius - kKnobTickStartOffset;
    g.setColour(LIGHT_GREY.withAlpha(0.3f));
    for (int i = 0; i < kKnobNumTicks; ++i)
    {
        const float tickAngle = juce::MathConstants<float>::twoPi * (static_cast<float>(i) / static_cast<float>(kKnobNumTicks));
        const juce::Point<float> start(centreX + tickStart * std::cos(tickAngle), 
                                      centreY + tickStart * std::sin(tickAngle));
        const juce::Point<float> end(centreX + (tickStart - kKnobTickLength) * std::cos(tickAngle), 
                                    centreY + (tickStart - kKnobTickLength) * std::sin(tickAngle));
        g.drawLine(start.x, start.y, end.x, end.y, kToggleBorderWidth);
    }

    const float pointerLength = radius * kKnobPointerLength;
    const float endX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float endY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);

    g.setColour(WHITE);
    g.drawLine(centreX, centreY, endX, endY, kKnobPointerThickness);

    g.setColour(WHITE);
    const float dotDiameter = kKnobCenterDotRadius * 2.0f;
    g.fillEllipse(centreX - kKnobCenterDotRadius, centreY - kKnobCenterDotRadius, 
                  dotDiameter, dotDiameter);

    if (isMouseOver || isMouseDown)
    {
        g.setColour(PURPLE.withAlpha(isMouseDown ? 0.4f : 0.2f));
        g.fillEllipse(knobBounds.reduced(kToggleBorderWidth));
    }
}
