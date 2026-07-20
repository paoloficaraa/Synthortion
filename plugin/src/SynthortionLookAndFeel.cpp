#include "Synthortion/SynthortionLookAndFeel.h"
#include <BinaryData.h>

namespace
{
    const juce::Colour CREAM(0xFF000000);
    const juce::Colour CREAM_DARK(0xFF000000);
    const juce::Colour CREAM_SHADOW(0xFFFFFFFF);
    const juce::Colour VIOLET(0xFFFFFFFF);
    const juce::Colour COPPER(0xFFFFFFFF);
    const juce::Colour COPPER_BRIGHT(0xFFFFFFFF);
    const juce::Colour GRAPHITE(0xFFFFFFFF);
    const juce::Colour WARM_GRAY(0xFFFFFFFF);
    const juce::Colour TEXT_DARK(0xFFFFFFFF);
    const juce::Colour LED_GREEN(0xFFFFFFFF);
    const juce::Colour LED_OFF(0xFF000000);

}

SynthortionLookAndFeel::SynthortionLookAndFeel()
    : sectionHeadingFont (juce::FontOptions().withName("BebasNeue").withHeight(18.0f).withStyle("Regular")),
      parameterLabelFont (juce::FontOptions().withName("Montserrat").withHeight(13.0f).withStyle("Medium")),
      parameterValueFont (juce::FontOptions().withName("Montserrat").withHeight(12.0f).withStyle("Regular")),
      bypassLabelFont (juce::FontOptions().withName("Montserrat").withHeight(13.0f).withStyle("Medium"))
{
    setColour(juce::ResizableWindow::backgroundColourId, CREAM);
    setColour(juce::DocumentWindow::backgroundColourId, CREAM);
    setColour(juce::DialogWindow::backgroundColourId, CREAM);

    setColour(juce::Slider::backgroundColourId, CREAM_DARK);
    setColour(juce::Slider::thumbColourId, VIOLET);
    setColour(juce::Slider::trackColourId, VIOLET);
    setColour(juce::Slider::rotarySliderFillColourId, VIOLET);
    setColour(juce::Slider::rotarySliderOutlineColourId, CREAM_SHADOW);

    setColour(juce::TextButton::buttonColourId, CREAM_DARK);
    setColour(juce::TextButton::buttonOnColourId, VIOLET);
    setColour(juce::TextButton::textColourOffId, TEXT_DARK);
    setColour(juce::TextButton::textColourOnId, CREAM);

    setColour(juce::ToggleButton::textColourId, TEXT_DARK);
    setColour(juce::ToggleButton::tickColourId, VIOLET);
    setColour(juce::ToggleButton::tickDisabledColourId, WARM_GRAY);

    setColour(juce::Label::textColourId, TEXT_DARK);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(backgroundColourId, CREAM);
    setColour(panelFillColourId, CREAM_DARK);
    setColour(panelOutlineColourId, CREAM_SHADOW);
    setColour(accentColourId, VIOLET);
    setColour(accentBrightColourId, COPPER_BRIGHT);
    setColour(textColourId, TEXT_DARK);
    setColour(knobFillColourId, GRAPHITE);

    bebasNeueTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::BebasNeueRegular_ttf,
                                                                 BinaryData::BebasNeueRegular_ttfSize);
    montserratTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratVariableFont_wght_ttf,
                                                                  BinaryData::MontserratVariableFont_wght_ttfSize);
}

juce::Typeface::Ptr SynthortionLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    auto fontName = font.getTypefaceName();

    if (fontName.containsIgnoreCase("Bebas"))
        return bebasNeueTypeface;

    if (fontName.containsIgnoreCase("Montserrat"))
        return montserratTypeface;

    return montserratTypeface;
}

void SynthortionLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(kKnobReduction);
    const float knobAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging();
    const bool isMouseDown = slider.isMouseButtonDown();

    draw3DKnob(g, bounds, knobAngle, sliderPos, isMouseOver, isMouseDown, rotaryStartAngle, rotaryEndAngle);
}

void SynthortionLookAndFeel::draw3DKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                    float angle, float sliderPos, bool isMouseOver, bool isMouseDown,
                                    float rotaryStartAngle, float rotaryEndAngle) const
{
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float diameter = radius * 2.0f;
    const auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

    drawCopperRim(g, knobBounds);

    juce::ColourGradient faceGrad(
        GRAPHITE.brighter(0.15f), knobBounds.getTopLeft().translated(0, -radius * 0.1f),
        GRAPHITE.darker(0.1f), knobBounds.getBottomRight().translated(0, radius * 0.05f),
        false);
    g.setGradientFill(faceGrad);
    g.fillEllipse(knobBounds.reduced(kKnobFaceReduction + 2.0f));

    const float tickStart = radius - kKnobTickStartOffset - 2.0f;
    const float majorTickLength = kKnobTickLength + 1.0f;
    const float minorTickLength = kKnobTickLength;

    g.setColour(WARM_GRAY.withAlpha(0.4f));
    for (int i = 0; i < kKnobNumTicks; ++i)
    {
        const float tickAngle = rotaryStartAngle + (static_cast<float>(i) / static_cast<float>(kKnobNumTicks - 1)) *
                               (rotaryEndAngle - rotaryStartAngle);
        const bool isMajor = (i == 0 || i == kKnobNumTicks - 1 || i == kKnobNumTicks / 2);
        const float tickLen = isMajor ? majorTickLength : minorTickLength;

        const juce::Point<float> start(centreX + tickStart * std::cos(tickAngle),
                                        centreY + tickStart * std::sin(tickAngle));
        const juce::Point<float> end(centreX + (tickStart - tickLen) * std::cos(tickAngle),
                                      centreY + (tickStart - tickLen) * std::sin(tickAngle));
        g.drawLine(start.x, start.y, end.x, end.y, isMajor ? 1.5f : 0.8f);
    }

    drawLEDArc(g, knobBounds, rotaryStartAngle, rotaryEndAngle, sliderPos);

    const float pointerLength = radius * kKnobPointerLength;
    const float pointerX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float pointerY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);

    g.setColour(COPPER_BRIGHT);
    g.drawLine(centreX, centreY, pointerX, pointerY, kKnobPointerThickness);

    g.setColour(CREAM);
    const float dotDiameter = kKnobCenterDotRadius * 2.0f;
    g.fillEllipse(centreX - kKnobCenterDotRadius, centreY - kKnobCenterDotRadius, dotDiameter, dotDiameter);

    if (isMouseOver || isMouseDown)
    {
        g.setColour(COPPER.withAlpha(isMouseDown ? 0.3f : 0.15f));
        g.fillEllipse(knobBounds.reduced(kKnobFaceReduction + 2.0f));
    }
}

void SynthortionLookAndFeel::drawCopperRim(juce::Graphics& g, const juce::Rectangle<float>& bounds) const
{
    const float radius = bounds.getWidth() * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();

    juce::Path shadow;
    shadow.addEllipse(bounds.reduced(1.0f).translated(1.5f, 2.0f));
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillPath(shadow);

    juce::ColourGradient rimGrad(
        WARM_GRAY.brighter(0.2f), centreX - radius, centreY - radius,
        GRAPHITE, centreX + radius, centreY + radius,
        true);
    g.setGradientFill(rimGrad);
    g.fillEllipse(bounds);
}

void SynthortionLookAndFeel::drawLEDArc(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                    float startAngle, float endAngle, float sliderPos) const
{
    const float radius = bounds.getWidth() * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();

    const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);

    const float arcRadius = radius - 6.0f;
    const float arcThickness = 2.5f;
    const float activeLevel = 1.0f - bypassMix;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                 startAngle, endAngle, true);
    g.setColour(COPPER.withAlpha(0.2f * activeLevel));
    g.strokePath(backgroundArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (activeLevel <= 0.0f)
        return;

    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                            startAngle, currentAngle, true);
    g.setColour(COPPER.withAlpha(0.7f * activeLevel));
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SynthortionLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool isHighlighted, bool isDown)
{
    const bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat();

    auto switchBounds = bounds;

    if (button.getButtonText().isNotEmpty())
    {
        auto textArea = bounds;
        textArea.removeFromLeft(kSwitchWidth + 8.0f);
        g.setColour(TEXT_DARK);
        g.setFont(bypassLabelFont);
        g.drawFittedText(button.getButtonText(), textArea.toNearestInt(),
                         juce::Justification::centredLeft, 1);
    }

    drawSwitchHandle(g, switchBounds, isOn, isHighlighted, isDown);

    juce::Rectangle<float> ledBounds(switchBounds.getX() + 4.0f, switchBounds.getCentreY() - 3.0f, 6.0f, 6.0f);
    g.setColour(isOn ? LED_GREEN : LED_OFF);
    g.fillEllipse(ledBounds);
    if (isOn)
    {
        g.setColour(LED_GREEN.withAlpha(0.4f));
        g.fillEllipse(ledBounds.expanded(2.0f));
    }
}

void SynthortionLookAndFeel::drawSwitchHandle(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                          bool isOn, bool isHighlighted, bool isDown) const
{
    const float w = kSwitchWidth;
    const float h = kSwitchHeight;
    const float x = bounds.getX() + (bounds.getWidth() - w) * 0.5f;
    const float y = bounds.getY() + (bounds.getHeight() - h) * 0.5f;

    auto switchBounds = juce::Rectangle<float>(x, y, w, h);

    g.setColour(CREAM_SHADOW);
    g.fillRoundedRectangle(switchBounds.expanded(1.0f), kSwitchCornerRadius);

    juce::ColourGradient bgGrad(
        isOn ? VIOLET.darker(0.3f) : CREAM_DARK.darker(0.2f),
        switchBounds.getTopLeft(),
        isOn ? VIOLET.darker(0.5f) : CREAM_DARK,
        switchBounds.getBottomRight(),
        false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(switchBounds, kSwitchCornerRadius);

    g.setColour(isOn ? VIOLET : WARM_GRAY.withAlpha(0.3f));
    g.drawRoundedRectangle(switchBounds, kSwitchCornerRadius, 1.0f);

    const float handleWidth = w * 0.4f;
    const float handleHeight = h - 6.0f;
    const float handleX = isOn ? x + w - handleWidth - 4.0f : x + 4.0f;
    const float handleY = y + 3.0f;

    auto handleBounds = juce::Rectangle<float>(handleX, handleY, handleWidth, handleHeight);

    juce::ColourGradient handleGrad(
        isHighlighted ? CREAM.brighter(0.2f) : CREAM,
        handleBounds.getTopLeft(),
        isHighlighted ? CREAM_SHADOW : CREAM_SHADOW.darker(0.2f),
        handleBounds.getBottomRight(),
        false);
    g.setGradientFill(handleGrad);
    g.fillRoundedRectangle(handleBounds, 2.0f);

    g.setColour(WARM_GRAY.darker(0.3f));
    g.drawRoundedRectangle(handleBounds, 2.0f, 0.5f);

    if (isDown)
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(handleBounds, 2.0f);
    }
}

void SynthortionLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                             bool isRecessed, const juce::String& title)
{
    auto r = bounds.toFloat();

    g.fillAll(isRecessed ? CREAM_SHADOW : CREAM_DARK);

    if (isRecessed)
    {
        g.setColour(CREAM_SHADOW);
        g.fillRoundedRectangle(r, 6.0f);

        g.setColour(CREAM);
        g.fillRoundedRectangle(r.reduced(2.0f), 5.0f);

        g.setColour(WARM_GRAY.withAlpha(0.1f));
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 0.5f);
    }
    else
    {
        juce::ColourGradient grad(
            CREAM_DARK.brighter(0.05f), r.getTopLeft(),
            CREAM_SHADOW.darker(0.1f), r.getBottomRight(),
            false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(r, 6.0f);

        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(r.reduced(1.0f), 5.0f, 0.5f);

        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(r.reduced(2.0f), 4.0f, 0.5f);
    }

    drawSectionTitle(g, r, title);
}

void SynthortionLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                             bool /*isRecessed*/, const juce::String& title,
                                             const juce::Colour& bgColour)
{
    auto r = bounds.toFloat();

    g.setColour(bgColour);
    g.fillRoundedRectangle(r.reduced(1.0f), 6.0f);

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(r.reduced(1.0f), 5.0f, 0.5f);

    drawSectionTitle(g, r, title);
}

void SynthortionLookAndFeel::drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                                          const juce::String& title) const
{
    if (title.isNotEmpty())
    {
        auto labelArea = r.removeFromTop(kSectionTitleHeight).reduced(kSectionTitleInset, 0.0f);

        g.setColour(TEXT_DARK.withAlpha(0.7f));
        g.setFont(sectionHeadingFont);
        g.drawFittedText(title, labelArea.toNearestInt(),
                         juce::Justification::centredLeft, 1);
    }
}
