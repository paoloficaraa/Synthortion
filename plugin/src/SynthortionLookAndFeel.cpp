#include "Synthortion/SynthortionLookAndFeel.h"

SynthortionLookAndFeel::SynthortionLookAndFeel()
{
    // Set Alkane-inspired color scheme
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

SynthortionLookAndFeel::~SynthortionLookAndFeel()
{
}

void SynthortionLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider &slider)
{
    // Build an explicit float rect to avoid implicit int->float narrowing warnings
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    const float knobAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    bool isMouseOver = slider.isMouseOverOrDragging();
    bool isMouseDown = slider.isMouseButtonDown();

    // Draw the knob using Alkane-inspired style
    drawGradientKnob(g, bounds, knobAngle, isMouseOver, isMouseDown);
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
    juce::ignoreUnused(isButtonDown);
    auto cornerSize = 3.0f;
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);

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

    auto btnColour = shouldDrawButtonAsDown ? PURPLE_DARK : MID_GREY.interpolatedWith(backgroundColour, 0.15f);
    if (shouldDrawButtonAsHighlighted)
        btnColour = btnColour.brighter(0.2f);

    g.setColour(btnColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    g.setColour(BLACK.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void SynthortionLookAndFeel::drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto cornerSize = 4.0f;

    // Determine button state and colors
    auto isToggleOn = button.getToggleState();
    auto btnColour = isToggleOn ? PURPLE : MID_GREY;
    auto textColour = isToggleOn ? BLACK : LIGHT_GREY;

    if (shouldDrawButtonAsHighlighted)
        btnColour = btnColour.brighter(0.3f);
    if (shouldDrawButtonAsDown)
        btnColour = btnColour.darker(0.2f);

    // Draw button background
    g.setColour(btnColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Draw border
    g.setColour(isToggleOn ? PURPLE_DARK : DARK_GREY);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

    // Draw text
    g.setColour(textColour);
    juce::Font font(juce::FontOptions().withHeight(11.0f).withStyle("Bold"));
    g.setFont(font);
    g.drawFittedText(button.getButtonText(), bounds.toNearestInt(),
                     juce::Justification::centred, 1);

    // Add subtle glow when active
    if (isToggleOn)
    {
        g.setColour(PURPLE.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.expanded(1.0f), cornerSize + 1.0f, 2.0f);
    }
}

void SynthortionLookAndFeel::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    g.fillAll(DARK_GREY);
    g.setColour(PURPLE.withAlpha(0.5f));
    g.drawRect(0, 0, width, height, 1);
}

void SynthortionLookAndFeel::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                                               const juce::String &text, const juce::String &shortcutKeyText,
                                               const juce::Drawable *icon, const juce::Colour *textColour)
{
    juce::ignoreUnused(icon, hasSubMenu);

    auto r = area.toFloat();

    if (isSeparator)
    {
        g.setColour(BLACK.withAlpha(0.4f));
        g.fillRect(r.withHeight(1.0f));
        return;
    }

    // Row background
    if (isHighlighted)
    {
        g.setColour(PURPLE.withAlpha(0.18f));
        g.fillRoundedRectangle(r.reduced(2.0f), 4.0f);
        g.setColour(PURPLE.withAlpha(0.35f));
        g.drawRoundedRectangle(r.reduced(2.0f), 4.0f, 1.0f);
    }

    // Tick mark
    if (isTicked)
    {
        g.setColour(PURPLE);
        g.fillEllipse(r.removeFromLeft(14.0f).reduced(4.0f));
        r.removeFromLeft(4.0f);
    }

    // Text
    {
        g.setFont(juce::FontOptions().withHeight(11.0f));
    }
    auto col = textColour != nullptr ? *textColour : (isActive ? LIGHT_GREY : LIGHT_GREY.withAlpha(0.5f));
    g.setColour(col);

    auto textArea = r.reduced(10.0f, 0.0f).toNearestInt();
    g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour(LIGHT_GREY.withAlpha(isActive ? 0.6f : 0.3f));
        g.drawText(shortcutKeyText, textArea, juce::Justification::centredRight, true);
    }
}

juce::Font SynthortionLookAndFeel::getLabelFont(juce::Label &label)
{
    juce::ignoreUnused(label);
    auto font = juce::Font(juce::FontOptions().withHeight(12.0f));
    font.setExtraKerningFactor(0.1f);
    return font;
}

juce::Font SynthortionLookAndFeel::getComboBoxFont(juce::ComboBox &box)
{
    juce::ignoreUnused(box);
    return juce::Font(juce::FontOptions().withHeight(11.0f));
}

juce::Font SynthortionLookAndFeel::getPopupMenuFont()
{
    return juce::Font(juce::FontOptions().withHeight(11.0f));
}

void SynthortionLookAndFeel::drawInnerShadow(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                             float cornerRadius, float shadowSize)
{
    auto shadowBounds = bounds.reduced(1.0f);
    auto shadowColour = BLACK.withAlpha(0.7f);

    juce::Path path;
    path.addRoundedRectangle(shadowBounds, cornerRadius);

    g.setColour(shadowColour);
    g.strokePath(path, juce::PathStrokeType(shadowSize, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SynthortionLookAndFeel::drawGlowEffect(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                            const juce::Colour &glowColour, float intensity)
{
    const int numSteps = 10;
    const float stepSize = bounds.getWidth() * 0.1f;

    g.setColour(glowColour.withAlpha(intensity / numSteps));

    for (int i = 1; i <= numSteps; ++i)
    {
        g.drawEllipse(bounds.expanded(stepSize * i), 1.5f);
    }
}

void SynthortionLookAndFeel::drawGradientKnob(juce::Graphics &g, const juce::Rectangle<float> &bounds,
                                              float angle, bool isMouseOver, bool isMouseDown)
{
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    auto knobBounds = juce::Rectangle<float>(rx, ry, rw, rw);

    // Main knob background
    g.setColour(BLACK);
    g.fillEllipse(knobBounds);

    // Knob face with gradient
    juce::ColourGradient gradient(MID_GREY.brighter(0.1f), juce::Point<float>(knobBounds.getX(), knobBounds.getY()),
                                  DARK_GREY, juce::Point<float>(knobBounds.getRight(), knobBounds.getBottom()), false);
    g.setGradientFill(gradient);
    g.fillEllipse(knobBounds.reduced(1.5f));

    // Tick ring around the edge
    const int numTicks = 24;
    const float tickLen = 3.0f;
    const float tickStart = radius - 2.0f;
    g.setColour(LIGHT_GREY.withAlpha(0.3f));
    for (int i = 0; i < numTicks; ++i)
    {
        const float tickAngle = juce::MathConstants<float>::twoPi * (static_cast<float>(i) / static_cast<float>(numTicks));
        const juce::Point<float> start(centreX + tickStart * std::cos(tickAngle), centreY + tickStart * std::sin(tickAngle));
        const juce::Point<float> end(centreX + (tickStart - tickLen) * std::cos(tickAngle), centreY + (tickStart - tickLen) * std::sin(tickAngle));
        g.drawLine(start.x, start.y, end.x, end.y, 1.0f);
    }

    // Value pointer - line from center to edge, properly positioned
    const float pointerLength = radius * 0.7f; // 70% of radius
    const float pointerThickness = 2.8f;

    // Calculate pointer endpoint based on angle
    const float endX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float endY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);

    // Draw pointer line from center
    g.setColour(WHITE);
    g.drawLine(centreX, centreY, endX, endY, pointerThickness);

    // Small center dot
    g.setColour(WHITE);
    g.fillEllipse(centreX - 2.0f, centreY - 2.0f, 4.0f, 4.0f);

    // Highlight
    if (isMouseOver || isMouseDown)
    {
        g.setColour(PURPLE.withAlpha(isMouseDown ? 0.4f : 0.2f));
        g.fillEllipse(knobBounds.reduced(1.0f));
    }
}

// Section panels and labels
void SynthortionLookAndFeel::drawSectionPanel(juce::Graphics &g, const juce::Rectangle<float> &b, float r) const
{
    auto bounds = b;
    // base fill
    juce::ColourGradient grad(DARK_GREY, juce::Point<float>(bounds.getX(), bounds.getY()),
                              MID_GREY.darker(0.2f), juce::Point<float>(bounds.getRight(), bounds.getBottom()), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, r);

    // inner bevel
    g.setColour(BLACK.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), r, 1.0f);
    g.setColour(LIGHT_GREY.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.reduced(2.0f), r, 1.0f);
}

void SynthortionLookAndFeel::drawFrameLabel(juce::Graphics &g, const juce::Rectangle<float> &b, const juce::String &text) const
{
    auto labelBounds = b;
    const float r = 4.f;
    g.setColour(BLACK);
    g.fillRoundedRectangle(labelBounds, r);
    g.setColour(PURPLE);
    g.drawRoundedRectangle(labelBounds, r, 1.0f);

    // Small LED chip on the left
    auto led = labelBounds.removeFromLeft(14.0f).reduced(3.0f);
    g.setColour(PURPLE.withAlpha(0.9f));
    g.fillEllipse(led);
    g.setColour(PURPLE_DARK);
    g.drawEllipse(led, 1.0f);

    g.setColour(LIGHT_GREY);
    juce::Font boldFont(juce::FontOptions().withHeight(11.0f).withStyle("Bold"));
    g.setFont(boldFont);
    g.drawFittedText(text, labelBounds.toNearestInt(), juce::Justification::centred, 1);
}
