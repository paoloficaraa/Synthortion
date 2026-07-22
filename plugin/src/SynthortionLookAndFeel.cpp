#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/AnimatedKnob.h"
#include <BinaryData.h>

namespace
{
    const juce::Colour BLACK(0xFF000000);
    const juce::Colour WHITE(0xFFFFFFFF);
}

SynthortionLookAndFeel::SynthortionLookAndFeel()
    : sectionHeadingFont (juce::FontOptions().withName("BebasNeue").withHeight(kSectionTitleHeight).withStyle("Regular").withKerningFactor(kTightKerning)),
      parameterLabelFont (juce::FontOptions().withName("BebasNeue").withHeight(14.0f).withStyle("Regular").withKerningFactor(kTightKerning)),
      parameterValueFont (juce::FontOptions().withName("Montserrat").withHeight(12.0f).withStyle("Regular")),
      bypassLabelFont (juce::FontOptions().withName("BebasNeue").withHeight(16.0f).withStyle("Regular").withKerningFactor(kTightKerning))
{
    setColour(juce::ResizableWindow::backgroundColourId, BLACK);
    setColour(juce::DocumentWindow::backgroundColourId, BLACK);
    setColour(juce::DialogWindow::backgroundColourId, BLACK);

    setColour(juce::Slider::backgroundColourId, BLACK);
    setColour(juce::Slider::thumbColourId, WHITE);
    setColour(juce::Slider::trackColourId, WHITE);
    setColour(juce::Slider::rotarySliderFillColourId, WHITE);
    setColour(juce::Slider::rotarySliderOutlineColourId, WHITE);

    setColour(juce::TextButton::buttonColourId, BLACK);
    setColour(juce::TextButton::buttonOnColourId, WHITE);
    setColour(juce::TextButton::textColourOffId, WHITE);
    setColour(juce::TextButton::textColourOnId, BLACK);

    setColour(juce::ToggleButton::textColourId, WHITE);
    setColour(juce::ToggleButton::tickColourId, WHITE);
    setColour(juce::ToggleButton::tickDisabledColourId, WHITE);

    setColour(juce::Label::textColourId, WHITE);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(backgroundColourId, BLACK);
    setColour(panelFillColourId, juce::Colour (0xFF0D0D0E));
    setColour(panelOutlineColourId, WHITE);
    setColour(accentColourId, WHITE);
    setColour(accentBrightColourId, WHITE);
    setColour(textColourId, WHITE);
    setColour(knobFillColourId, WHITE);
    setColour(surfaceAltColourId, juce::Colour (0xFF121214));
    setColour(dimmedColourId, juce::Colour (0x66FFFFFF));

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
    const bool isHoverOrDrag = slider.isMouseOverOrDragging() || slider.isMouseButtonDown() || slider.getThumbBeingDragged() != -1;

    auto knobStyle = synthortion::AnimatedKnob::KnobStyle::Canonical;
    int steps = synthortion::AnimatedKnob::kCanonicalSteps;

    if (auto* animatedKnob = dynamic_cast<synthortion::AnimatedKnob*>(&slider))
    {
        knobStyle = animatedKnob->getKnobStyle();
        steps = animatedKnob->getStepCount();
    }

    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float diameter = radius * 2.0f;
    const auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

    if (isHoverOrDrag)
        drawTwinShadow(g, knobBounds);

    if (knobStyle == synthortion::AnimatedKnob::KnobStyle::Outline)
        drawOutlineKnob(g, bounds, knobAngle, sliderPos, isHoverOrDrag, rotaryStartAngle, rotaryEndAngle, steps);
    else
        drawCanonicalKnob(g, bounds, knobAngle, sliderPos, isHoverOrDrag, rotaryStartAngle, rotaryEndAngle, steps);
}

void SynthortionLookAndFeel::drawTwinShadow(juce::Graphics& g, const juce::Rectangle<float>& knobBounds) const
{
    const juce::Rectangle<float> offset = knobBounds.translated(1.0f, 1.0f);

    g.setColour(WHITE);
    g.fillEllipse(offset);

    g.setColour(BLACK);
    g.fillEllipse(offset.reduced(1.0f));
}

void SynthortionLookAndFeel::drawCanonicalKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                           float knobAngle, float sliderPos, bool isInverted,
                                           float rotaryStartAngle, float rotaryEndAngle, int steps) const
{
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float diameter = radius * 2.0f;
    const auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

    const juce::Colour faceColour = isInverted ? BLACK : WHITE;
    const juce::Colour pointerColour = isInverted ? WHITE : BLACK;

    g.setColour(faceColour);
    g.fillEllipse(knobBounds);

    const float arcRadius = radius - kKnobArcInset;
    const int litCount = juce::jlimit(0, steps, juce::roundToInt(sliderPos * static_cast<float>(steps)));
    const float angularSpan = rotaryEndAngle - rotaryStartAngle;
    const float litAlpha = juce::jlimit(0.0f, 1.0f, 1.0f - bypassMix);

    if (litAlpha > 0.0f)
    {
        g.setColour(pointerColour.withAlpha(litAlpha));

        for (int i = 0; i < litCount; ++i)
        {
            const float segStart = rotaryStartAngle + (static_cast<float>(i) / static_cast<float>(steps)) * angularSpan;
            const float segEnd = rotaryStartAngle + (static_cast<float>(i + 1) / static_cast<float>(steps)) * angularSpan;

            juce::Path segment;
            segment.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, segStart, segEnd, true);
            g.strokePath(segment, juce::PathStrokeType(kKnobArcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
        }
    }

    const float pointerLength = radius * kKnobPointerLength;
    const float pointerX = centreX + pointerLength * std::cos(knobAngle - juce::MathConstants<float>::halfPi);
    const float pointerY = centreY + pointerLength * std::sin(knobAngle - juce::MathConstants<float>::halfPi);

    g.setColour(pointerColour);
    g.drawLine(centreX, centreY, pointerX, pointerY, kKnobPointerThickness);
}

void SynthortionLookAndFeel::drawOutlineKnob(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                         float knobAngle, float sliderPos, bool isInverted,
                                         float rotaryStartAngle, float rotaryEndAngle, int steps) const
{
    juce::ignoreUnused (sliderPos);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float diameter = radius * 2.0f;
    const auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

    const juce::Colour faceColour = isInverted ? WHITE : BLACK;
    const juce::Colour outlineColour = isInverted ? BLACK : WHITE;
    const juce::Colour pointerColour = isInverted ? BLACK : WHITE;

    g.setColour(faceColour);
    g.fillEllipse(knobBounds);

    g.setColour(outlineColour);
    g.drawEllipse(knobBounds, 1.0f);

    const float rimRadius = radius - kKnobTickStartOffset;
    const float tickInner = rimRadius - kKnobTickLength;
    const float angularSpan = rotaryEndAngle - rotaryStartAngle;

    for (int i = 0; i < steps; ++i)
    {
        const float tickAngle = rotaryStartAngle + (static_cast<float>(i) / static_cast<float>(steps)) * angularSpan;

        const juce::Point<float> start(centreX + rimRadius * std::cos(tickAngle),
                                        centreY + rimRadius * std::sin(tickAngle));
        const juce::Point<float> end(centreX + tickInner * std::cos(tickAngle),
                                      centreY + tickInner * std::sin(tickAngle));
        g.drawLine(start.x, start.y, end.x, end.y, 1.0f);
    }

    const float pointerLength = radius * kKnobPointerLength;
    const float pointerX = centreX + pointerLength * std::cos(knobAngle - juce::MathConstants<float>::halfPi);
    const float pointerY = centreY + pointerLength * std::sin(knobAngle - juce::MathConstants<float>::halfPi);

    g.setColour(pointerColour);
    g.drawLine(centreX, centreY, pointerX, pointerY, kKnobPointerThickness);
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
        g.setColour(WHITE);
        g.setFont(bypassLabelFont);
        g.drawFittedText(button.getButtonText(), textArea.toNearestInt(),
                         juce::Justification::centredLeft, 1);
    }

    drawSwitchHandle(g, switchBounds, isOn, isHighlighted, isDown);

    juce::Rectangle<float> ledBounds(switchBounds.getX() + 4.0f, switchBounds.getCentreY() - 3.0f, 6.0f, 6.0f);
    g.setColour(isOn ? WHITE : BLACK);
    g.fillEllipse(ledBounds);
    if (isOn)
    {
        g.setColour(WHITE.withAlpha(0.4f));
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

    g.setColour(WHITE);
    g.fillRoundedRectangle(switchBounds.expanded(1.0f), kSwitchCornerRadius);

    juce::ColourGradient bgGrad(
        isOn ? WHITE.darker(0.3f) : BLACK.darker(0.2f),
        switchBounds.getTopLeft(),
        isOn ? WHITE.darker(0.5f) : BLACK,
        switchBounds.getBottomRight(),
        false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(switchBounds, kSwitchCornerRadius);

    g.setColour(isOn ? WHITE : WHITE.withAlpha(0.3f));
    g.drawRoundedRectangle(switchBounds, kSwitchCornerRadius, 1.0f);

    const float handleWidth = w * 0.4f;
    const float handleHeight = h - 6.0f;
    const float handleX = isOn ? x + w - handleWidth - 4.0f : x + 4.0f;
    const float handleY = y + 3.0f;

    auto handleBounds = juce::Rectangle<float>(handleX, handleY, handleWidth, handleHeight);

    juce::ColourGradient handleGrad(
        isHighlighted ? BLACK.brighter(0.2f) : BLACK,
        handleBounds.getTopLeft(),
        isHighlighted ? WHITE : WHITE.darker(0.2f),
        handleBounds.getBottomRight(),
        false);
    g.setGradientFill(handleGrad);
    g.fillRoundedRectangle(handleBounds, 2.0f);

    g.setColour(WHITE.darker(0.3f));
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
    drawPanelBackground(g, bounds, isRecessed, title, findColour(panelFillColourId));
}

void SynthortionLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                               bool isRecessed, const juce::String& title,
                                               const juce::Colour& bgColour)
{
    drawPanelBackground(g, bounds, isRecessed, title, bgColour, kSectionTitleHeight, kSectionTitleInset);
}

void SynthortionLookAndFeel::drawPanelBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                               bool /*isRecessed*/, const juce::String& title,
                                               const juce::Colour& bgColour,
                                               float titleFontHeight, float titleInset)
{
    const auto r = bounds.toFloat();

    g.setColour(bgColour);
    g.fillRect(r);

    g.setColour(WHITE);
    g.drawRect(r, 1.0f);

    auto titleRect = r;
    drawSectionTitle(g, titleRect, title, titleFontHeight, titleInset);

    if (title.isNotEmpty())
    {
        const float ruleY = r.getY() + kSectionTitleHeight;
        g.setColour(WHITE);
        g.drawHorizontalLine(static_cast<int>(ruleY), r.getX(), r.getRight());
    }

    g.setColour(WHITE);
    g.fillRect(r.getX(), r.getY(), 2.0f, 2.0f);
    g.fillRect(r.getRight() - 2.0f, r.getY(), 2.0f, 2.0f);
    g.fillRect(r.getX(), r.getBottom() - 2.0f, 2.0f, 2.0f);
    g.fillRect(r.getRight() - 2.0f, r.getBottom() - 2.0f, 2.0f, 2.0f);
}

void SynthortionLookAndFeel::drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                                           const juce::String& title) const
{
    drawSectionTitle(g, r, title, kSectionTitleHeight, kSectionTitleInset);
}

void SynthortionLookAndFeel::drawSectionTitle(juce::Graphics& g, juce::Rectangle<float>& r,
                                           const juce::String& title,
                                           float titleFontHeight, float titleInset) const
{
    if (title.isNotEmpty())
    {
        auto labelArea = r.removeFromTop(kSectionTitleHeight).reduced(titleInset, 0.0f);

        const juce::Font titleFont(juce::FontOptions().withName("BebasNeue")
                                   .withHeight(titleFontHeight).withStyle("Regular")
                                   .withKerningFactor(kTightKerning));

        g.setColour(WHITE);
        g.setFont(titleFont);
        g.drawFittedText(title, labelArea.toNearestInt(),
                         juce::Justification::centredLeft, 1);
    }
}
