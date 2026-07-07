#include "Synthortion/BypassSwitch.h"

namespace synthortion
{
    BypassSwitch::BypassSwitch (AnimationController* animationController)
        : juce::Button ("BYPASS"),
          controller (animationController)
    {
        setOpaque (false);
        setClickingTogglesState (true);
    }

    BypassSwitch::~BypassSwitch()
    {
        if (controller != nullptr && animationRunning && currentAnimator.has_value())
            controller->removeAnimator (*currentAnimator);
    }

    bool BypassSwitch::isLedOn() const noexcept
    {
        return getToggleState();
    }

    void BypassSwitch::paintButton (juce::Graphics& g, bool /*isMouseOver*/, bool /*isMouseDown*/)
    {
        auto bounds = getLocalBounds().toFloat();

        const auto* laf = dynamic_cast<const SynthortionLookAndFeel*> (&getLookAndFeel());
        auto getColour = [laf] (int colourId, juce::Colour fallback)
        {
            return laf != nullptr ? laf->findColour (colourId) : fallback;
        };

        const auto violet = getColour (SynthortionLookAndFeel::copperAccentColourId, juce::Colour (0xFF7C3AED));
        const auto violetBright = getColour (SynthortionLookAndFeel::copperBrightColourId, juce::Colour (0xFFFF2D78));
        const auto cream = getColour (SynthortionLookAndFeel::backgroundColourId, juce::Colour (0xFFF5F0EB));
        const auto panelRecessed = getColour (SynthortionLookAndFeel::panelRecessedColourId, juce::Colour (0xFFDCD5CE));
        const auto warmGray = juce::Colour (0xFF6B6570);
        const auto textDark = juce::Colour (0xFF2E2A33);

        // Label
        const auto labelFont = laf != nullptr
                                   ? laf->getBypassLabelFont()
                                   : juce::FontOptions().withName ("Montserrat").withHeight (13.0f);
        g.setColour (textDark);
        g.setFont (labelFont);
        g.drawFittedText (getButtonText(), bounds.removeFromTop (18.0f).toNearestInt(),
                          juce::Justification::centred, 1);

        bounds = bounds.withTrimmedTop (4.0f);

        const float centreX = bounds.getCentreX();
        const float trackX = centreX - kTrackWidth * 0.5f;
        const float trackY = bounds.getCentreY() - kTrackHeight * 0.5f;
        const auto trackBounds = juce::Rectangle<float> (trackX, trackY, kTrackWidth, kTrackHeight);

        // Track background
        g.setColour (panelRecessed.darker (0.15f));
        g.fillRoundedRectangle (trackBounds, kTrackCornerRadius);

        // Active fill on the top portion of the track, grows as the lever springs upward
        const float activeHeight = kTrackHeight * animationProgress;
        if (activeHeight > 1.0f)
        {
            auto activeBounds = trackBounds.withHeight (activeHeight);
            g.setColour (violet.withAlpha (0.35f));
            g.fillRoundedRectangle (activeBounds, kTrackCornerRadius);
        }

        // Track border
        g.setColour (warmGray.withAlpha (0.4f));
        g.drawRoundedRectangle (trackBounds, kTrackCornerRadius, 1.0f);

        // Handle / lever
        const float handleTravel = kTrackHeight - kHandleSize - kTrackInset * 2.0f;
        const float handleX = trackX + (kTrackWidth - kHandleSize) * 0.5f;
        const float handleY = trackY + kTrackHeight - kTrackInset - kHandleSize - animationProgress * handleTravel;
        const auto handleBounds = juce::Rectangle<float> (handleX, handleY, kHandleSize, kHandleSize);

        juce::ColourGradient handleGrad (
            cream, handleBounds.getTopLeft(),
            panelRecessed, handleBounds.getBottomRight(),
            false);
        g.setGradientFill (handleGrad);
        g.fillRoundedRectangle (handleBounds, kHandleCornerRadius);

        g.setColour (warmGray.withAlpha (0.5f));
        g.drawRoundedRectangle (handleBounds, kHandleCornerRadius, 0.75f);

        // LED and violet glow
        const float ledX = trackBounds.getRight() + 10.0f;
        const float ledY = trackBounds.getCentreY() - kLedSize * 0.5f;
        const auto ledBounds = juce::Rectangle<float> (ledX, ledY, kLedSize, kLedSize);

        const float activeLevel = controller != nullptr ? 1.0f - controller->getBypassMix() : 1.0f;
        const auto ledOffColour = warmGray.brighter (0.3f);
        const auto ledColour = ledOffColour.interpolatedWith (violetBright, animationProgress * activeLevel);

        ledGlow.setColor (violet);
        ledGlow.setOpacity (static_cast<double> (animationProgress * 0.85f * activeLevel));
        ledGlow.setRadius (static_cast<double> (kGlowRadius * animationProgress));

        juce::Path ledPath;
        ledPath.addEllipse (ledBounds);
        ledGlow.render (g, ledPath);

        g.setColour (ledColour);
        g.fillEllipse (ledBounds);

        g.setColour (warmGray.darker (0.2f));
        g.drawEllipse (ledBounds, 0.75f);
    }

    void BypassSwitch::resized()
    {
        repaint();
    }

    void BypassSwitch::buttonStateChanged()
    {
        Button::buttonStateChanged();
        startAnimation();
    }

    void BypassSwitch::startAnimation()
    {
        const float target = getToggleState() ? 1.0f : 0.0f;

        if (controller == nullptr)
        {
            animationProgress = target;
            repaint();
            return;
        }

        if (animationRunning && currentAnimator.has_value())
            controller->removeAnimator (*currentAnimator);

        animationRunning = false;
        animationStart = animationProgress;
        animationTarget = target;

        currentAnimator = controller->runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (kAnimationDurationMs)
                .withEasing (juce::Easings::createSpring (
                    juce::SpringEasingOptions().withFrequency (3.5f).withAttenuation (3.0f)))
                .withValueChangedCallback ([this] (float t)
                                           {
                                               animationProgress = juce::jlimit (
                                                   0.0f, 1.0f, animationStart + (animationTarget - animationStart) * t);
                                               repaint();
                                           })
                .withOnCompleteCallback ([this]
                                         {
                                             animationRunning = false;
                                         }));

        animationRunning = true;
    }
}
