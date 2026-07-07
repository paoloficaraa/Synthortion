#include "Synthortion/AnalogLookAndFeel.h"
#include <BinaryData.h>

namespace
{
    const juce::Colour ANTHRACITE(0xFF130D1A);
    const juce::Colour GUNMETAL(0xFF1A1325);
    const juce::Colour DEEP_SHADOW(0xFF0A070D);
    const juce::Colour COPPER(0xFF7C3AED);
    const juce::Colour COPPER_BRIGHT(0xFFFF2D78);
    const juce::Colour CREAM(0xFFE0E0E0);
    const juce::Colour WARM_GRAY(0xFF5A4A6A);
    const juce::Colour GRAPHITE(0xFF251A35);
    const juce::Colour BRUSHED_SILVER(0xFF8A7A9A);
    const juce::Colour LED_GREEN(0xFF00F5D4);
    const juce::Colour LED_OFF(0xFF1A1A1A);

}

AnalogLookAndFeel::AnalogLookAndFeel()
    : sectionHeadingFont (juce::FontOptions().withName("BebasNeue").withHeight(18.0f).withStyle("Regular")),
      parameterLabelFont (juce::FontOptions().withName("Montserrat").withHeight(13.0f).withStyle("Medium")),
      parameterValueFont (juce::FontOptions().withName("Montserrat").withHeight(12.0f).withStyle("Regular")),
      bypassLabelFont (juce::FontOptions().withName("Montserrat").withHeight(13.0f).withStyle("Medium"))
{
    setColour(juce::ResizableWindow::backgroundColourId, ANTHRACITE);
    setColour(juce::DocumentWindow::backgroundColourId, ANTHRACITE);
    setColour(juce::DialogWindow::backgroundColourId, ANTHRACITE);

    setColour(juce::Slider::backgroundColourId, GRAPHITE);
    setColour(juce::Slider::thumbColourId, COPPER);
    setColour(juce::Slider::trackColourId, COPPER);
    setColour(juce::Slider::rotarySliderFillColourId, COPPER);
    setColour(juce::Slider::rotarySliderOutlineColourId, GRAPHITE);

    setColour(juce::TextButton::buttonColourId, GRAPHITE);
    setColour(juce::TextButton::buttonOnColourId, COPPER);
    setColour(juce::TextButton::textColourOffId, CREAM);
    setColour(juce::TextButton::textColourOnId, ANTHRACITE);

    setColour(juce::ToggleButton::textColourId, CREAM);
    setColour(juce::ToggleButton::tickColourId, COPPER);
    setColour(juce::ToggleButton::tickDisabledColourId, WARM_GRAY);

    setColour(juce::Label::textColourId, CREAM);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(backgroundColourId, ANTHRACITE);
    setColour(panelColourId, GUNMETAL);
    setColour(panelRecessedColourId, DEEP_SHADOW);
    setColour(copperAccentColourId, COPPER);
    setColour(copperBrightColourId, COPPER_BRIGHT);
    setColour(creamTextColourId, CREAM);
    setColour(graphiteKnobColourId, GRAPHITE);

    bebasNeueTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::BebasNeueRegular_ttf,
                                                                 BinaryData::BebasNeueRegular_ttfSize);
    montserratTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratVariableFont_wght_ttf,
                                                                  BinaryData::MontserratVariableFont_wght_ttfSize);
}

juce::Typeface::Ptr AnalogLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    auto fontName = font.getTypefaceName();

    if (fontName.containsIgnoreCase("Bebas"))
        return bebasNeueTypeface;

    if (fontName.containsIgnoreCase("Montserrat"))
    {
        if (fontName.containsIgnoreCase("Regular"))
            return montserratRegularTypeface;
        return montserratTypeface;
    }

    return montserratTypeface;
}

void AnalogLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(kKnobReduction);
    const float knobAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging();
    const bool isMouseDown = slider.isMouseButtonDown();

    draw3DKnob(g, bounds, knobAngle, sliderPos, isMouseOver, isMouseDown, rotaryStartAngle, rotaryEndAngle);
}

void AnalogLookAndFeel::draw3DKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
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

void AnalogLookAndFeel::drawCopperRim(juce::Graphics& g, const juce::Rectangle<float>& bounds) const
{
    const float radius = bounds.getWidth() * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();

    juce::Path shadow;
    shadow.addEllipse(bounds.reduced(1.0f).translated(1.5f, 2.0f));
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillPath(shadow);

    juce::ColourGradient rimGrad(
        BRUSHED_SILVER.brighter(0.2f), centreX - radius, centreY - radius,
        GRAPHITE, centreX + radius, centreY + radius,
        true);
    g.setGradientFill(rimGrad);
    g.fillEllipse(bounds);
}

void AnalogLookAndFeel::drawLEDArc(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                    float startAngle, float endAngle, float sliderPos) const
{
    const float radius = bounds.getWidth() * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();

    const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);

    const float arcRadius = radius - 6.0f;
    const float arcThickness = 2.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                 startAngle, endAngle, true);
    g.setColour(COPPER.withAlpha(0.2f));
    g.strokePath(backgroundArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                            startAngle, currentAngle, true);
    g.setColour(COPPER.withAlpha(0.7f));
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void AnalogLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool isHighlighted, bool isDown)
{
    const bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat();

    auto switchBounds = bounds;

    if (button.getButtonText().isNotEmpty())
    {
        auto textArea = bounds;
        textArea.removeFromLeft(kSwitchWidth + 8.0f);
        g.setColour(CREAM);
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

void AnalogLookAndFeel::drawSwitchHandle(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                          bool isOn, bool isHighlighted, bool isDown) const
{
    const float w = kSwitchWidth;
    const float h = kSwitchHeight;
    const float x = bounds.getX() + (bounds.getWidth() - w) * 0.5f;
    const float y = bounds.getY() + (bounds.getHeight() - h) * 0.5f;

    auto switchBounds = juce::Rectangle<float>(x, y, w, h);

    g.setColour(DEEP_SHADOW);
    g.fillRoundedRectangle(switchBounds.expanded(1.0f), kSwitchCornerRadius);

    juce::ColourGradient bgGrad(
        isOn ? COPPER.darker(0.3f) : GRAPHITE.darker(0.2f),
        switchBounds.getTopLeft(),
        isOn ? COPPER.darker(0.5f) : GRAPHITE,
        switchBounds.getBottomRight(),
        false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(switchBounds, kSwitchCornerRadius);

    g.setColour(isOn ? COPPER : WARM_GRAY.withAlpha(0.3f));
    g.drawRoundedRectangle(switchBounds, kSwitchCornerRadius, 1.0f);

    const float handleWidth = w * 0.4f;
    const float handleHeight = h - 6.0f;
    const float handleX = isOn ? x + w - handleWidth - 4.0f : x + 4.0f;
    const float handleY = y + 3.0f;

    auto handleBounds = juce::Rectangle<float>(handleX, handleY, handleWidth, handleHeight);

    juce::ColourGradient handleGrad(
        isHighlighted ? BRUSHED_SILVER.brighter(0.2f) : BRUSHED_SILVER,
        handleBounds.getTopLeft(),
        isHighlighted ? GRAPHITE : GRAPHITE.darker(0.2f),
        handleBounds.getBottomRight(),
        false);
    g.setGradientFill(handleGrad);
    g.fillRoundedRectangle(handleBounds, 2.0f);

    g.setColour(BRUSHED_SILVER.darker(0.3f));
    g.drawRoundedRectangle(handleBounds, 2.0f, 0.5f);

    if (isDown)
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(handleBounds, 2.0f);
    }
}

void AnalogLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                             bool isRecessed, const juce::String& title)
{
    auto r = bounds.toFloat();

    g.fillAll(isRecessed ? DEEP_SHADOW : GUNMETAL);

    if (isRecessed)
    {
        g.setColour(DEEP_SHADOW);
        g.fillRoundedRectangle(r, 6.0f);

        g.setColour(ANTHRACITE);
        g.fillRoundedRectangle(r.reduced(2.0f), 5.0f);

        g.setColour(WARM_GRAY.withAlpha(0.1f));
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 0.5f);
    }
    else
    {
        juce::ColourGradient grad(
            GUNMETAL.brighter(0.05f), r.getTopLeft(),
            GRAPHITE.darker(0.1f), r.getBottomRight(),
            false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(r, 6.0f);

        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(r.reduced(1.0f), 5.0f, 0.5f);

        g.setColour(CREAM.withAlpha(0.05f));
        g.drawRoundedRectangle(r.reduced(2.0f), 4.0f, 0.5f);
    }

    drawSectionTitle(g, r, title);
}

void AnalogLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
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

void AnalogLookAndFeel::drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                                          const juce::String& title) const
{
    if (title.isNotEmpty())
    {
        auto labelArea = r.removeFromTop(kSectionTitleHeight).reduced(kSectionTitleInset, 0.0f);

        g.setColour(CREAM.withAlpha(0.6f));
        g.setFont(sectionHeadingFont);
        g.drawFittedText(title, labelArea.toNearestInt(),
                         juce::Justification::centredLeft, 1);
    }
}
