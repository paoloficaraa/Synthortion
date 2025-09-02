#include "Synthortion/SynthortionLookAndFeel.h"

SynthortionLookAndFeel::SynthortionLookAndFeel()
{
    // Set Fire-inspired color scheme
    setColour(juce::ResizableWindow::backgroundColourId, FIRE_DARK);
    setColour(juce::DocumentWindow::backgroundColourId, FIRE_DARK);
    setColour(juce::DialogWindow::backgroundColourId, FIRE_DARK);

    setColour(juce::Slider::backgroundColourId, FIRE_MID);
    setColour(juce::Slider::thumbColourId, FIRE_PRIMARY);
    setColour(juce::Slider::trackColourId, FIRE_ACCENT);
    setColour(juce::Slider::rotarySliderFillColourId, FIRE_PRIMARY);
    setColour(juce::Slider::rotarySliderOutlineColourId, FIRE_MID);

    setColour(juce::TextButton::buttonColourId, FIRE_MID);
    setColour(juce::TextButton::buttonOnColourId, FIRE_PRIMARY);
    setColour(juce::TextButton::textColourOffId, FIRE_LIGHT);
    setColour(juce::TextButton::textColourOnId, FIRE_DARK);

    setColour(juce::Label::textColourId, FIRE_LIGHT);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ComboBox::backgroundColourId, FIRE_MID);
    setColour(juce::ComboBox::textColourId, FIRE_LIGHT);
    setColour(juce::ComboBox::outlineColourId, FIRE_PRIMARY);
    setColour(juce::ComboBox::buttonColourId, FIRE_PRIMARY);
    setColour(juce::ComboBox::arrowColourId, FIRE_DARK);
}

SynthortionLookAndFeel::~SynthortionLookAndFeel()
{
}

void SynthortionLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider &slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();

    bool isMouseOver = slider.isMouseOverOrDragging();
    bool isMouseDown = slider.isMouseButtonDown();

    // Draw the knob using Fire-inspired gradient
    drawGradientKnob(g, bounds, toAngle, isMouseOver, isMouseDown);
}

void SynthortionLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(getLabelFont(label));
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)(textArea.getHeight() / getLabelFont(label).getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

void SynthortionLookAndFeel::drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                                          int buttonX, int buttonY, int buttonW, int buttonH,
                                          juce::ComboBox &box)
{
    auto cornerSize = 3.0f;
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    // Background
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, cornerSize);

    // Inner shadow for depth
    drawInnerShadow(g, bounds, cornerSize);

    // Outline
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

    // Arrow button area
    auto arrowZone = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH).toFloat();
    g.setColour(box.findColour(juce::ComboBox::buttonColourId));
    g.fillRoundedRectangle(arrowZone, cornerSize);

    // Arrow
    auto arrowX = arrowZone.getCentreX();
    auto arrowY = arrowZone.getCentreY();
    auto arrowSize = 3.0f;

    juce::Path arrow;
    arrow.startNewSubPath(arrowX - arrowSize, arrowY - arrowSize * 0.5f);
    arrow.lineTo(arrowX, arrowY + arrowSize * 0.5f);
    arrow.lineTo(arrowX + arrowSize, arrowY - arrowSize * 0.5f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(arrow, juce::PathStrokeType(1.5f));
}

void SynthortionLookAndFeel::positionComboBoxText(juce::ComboBox &box, juce::Label &label)
{
    label.setBounds(1, 1, box.getWidth() - 20, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

void SynthortionLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                  const juce::Colour &backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto cornerSize = 3.0f;

    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                          .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    // Fire gradient effect
    juce::ColourGradient gradient(FIRE_PRIMARY.brighter(0.3f), bounds.getX(), bounds.getY(),
                                  FIRE_ACCENT.darker(0.2f), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Inner shadow for depth
    drawInnerShadow(g, bounds, cornerSize);

    // Highlight when pressed or hovered
    if (shouldDrawButtonAsHighlighted)
    {
        drawGlowEffect(g, bounds, FIRE_HIGHLIGHT, 0.3f);
    }

    // Outline
    g.setColour(FIRE_ACCENT);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void SynthortionLookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    auto background = FIRE_DARK.darker(0.1f);
    g.fillAll(background);

    // Subtle border
    g.setColour(FIRE_PRIMARY.withAlpha(0.5f));
    g.drawRect(0, 0, width, height, 1);
}

juce::Font SynthortionLookAndFeel::getLabelFont(juce::Label &label)
{
    return juce::Font(12.0f, juce::Font::plain).withExtraKerningFactor(0.1f);
}

juce::Font SynthortionLookAndFeel::getComboBoxFont(juce::ComboBox &box)
{
    return juce::Font(11.0f, juce::Font::plain);
}

juce::Font SynthortionLookAndFeel::getPopupMenuFont()
{
    return juce::Font(11.0f, juce::Font::plain);
}

void SynthortionLookAndFeel::drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                             float cornerRadius, float shadowSize)
{
    auto shadowBounds = bounds.reduced(1.0f);

    // Create inner shadow using a gradient
    juce::ColourGradient shadow(FIRE_SHADOW.withAlpha(0.8f), shadowBounds.getX(), shadowBounds.getY(),
                                FIRE_SHADOW.withAlpha(0.0f), shadowBounds.getX() + shadowSize,
                                shadowBounds.getY() + shadowSize, false);

    g.setGradientFill(shadow);
    g.fillRoundedRectangle(shadowBounds.getX(), shadowBounds.getY(), shadowSize * 2, shadowSize * 2, cornerRadius);
}

void SynthortionLookAndFeel::drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                            const juce::Colour &glowColour, float intensity)
{
    auto glowBounds = bounds.expanded(2.0f);

    // Outer glow
    juce::ColourGradient glow(glowColour.withAlpha(intensity), bounds.getCentreX(), bounds.getCentreY(),
                              glowColour.withAlpha(0.0f), glowBounds.getRight(), glowBounds.getBottom(), true);

    g.setGradientFill(glow);
    g.fillEllipse(glowBounds);
}

void SynthortionLookAndFeel::drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                              float angle, bool isMouseOver, bool isMouseDown)
{
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto knobBounds = juce::Rectangle<float>(rx, ry, rw, rw);

    // Main knob gradient
    juce::ColourGradient gradient(FIRE_PRIMARY.brighter(0.4f), centreX, ry,
                                  FIRE_ACCENT.darker(0.3f), centreX, ry + rw, false);

    if (isMouseDown)
    {
        gradient = juce::ColourGradient(FIRE_ACCENT, centreX, ry,
                                        FIRE_PRIMARY.darker(0.4f), centreX, ry + rw, false);
    }
    else if (isMouseOver)
    {
        gradient = juce::ColourGradient(FIRE_PRIMARY.brighter(0.6f), centreX, ry,
                                        FIRE_ACCENT.darker(0.1f), centreX, ry + rw, false);
    }

    g.setGradientFill(gradient);
    g.fillEllipse(knobBounds);

    // Inner shadow
    drawInnerShadow(g, knobBounds, radius, 1.5f);

    // Glow effect when interacting
    if (isMouseOver || isMouseDown)
    {
        drawGlowEffect(g, knobBounds, FIRE_HIGHLIGHT, isMouseDown ? 0.6f : 0.4f);
    }

    // Outline
    g.setColour(FIRE_ACCENT.darker(0.2f));
    g.drawEllipse(knobBounds.reduced(1.0f), 2.0f);

    // Pointer/indicator
    juce::Path pointer;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 2.0f;

    pointer.addRectangle(-pointerThickness * 0.5f, -radius + 4.0f, pointerThickness, pointerLength);

    g.setColour(FIRE_LIGHT);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // Pointer shadow for depth
    g.setColour(FIRE_SHADOW.withAlpha(0.5f));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX + 1, centreY + 1));
}
