#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <melatonin_blur/melatonin_blur.h>

namespace synthortion
{
    AnimatedKnob::AnimatedKnob (AnimationController& controller)
        : animationController (controller)
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setRotaryParameters (kStartAngle, kEndAngle, true);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setVelocityBasedMode (true);
        setVelocityModeParameters (0.5f, 1, 0.1f, false);
    }

    AnimatedKnob::~AnimatedKnob()
    {
        if (currentAnimator.has_value() && ! currentAnimator->isComplete())
            animationController.removeAnimator (*currentAnimator);
    }

    void AnimatedKnob::paint (juce::Graphics& g)
    {
        if (auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel()))
        {
            drawArcGlow (g);

            laf->drawRotarySlider (g,
                                   getLocalBounds().getX(),
                                   getLocalBounds().getY(),
                                   getLocalBounds().getWidth(),
                                   getLocalBounds().getHeight(),
                                   displayProportion,
                                   kStartAngle,
                                   kEndAngle,
                                   *this);
        }
        else
        {
            Slider::paint (g);
        }
    }

    void AnimatedKnob::valueChanged()
    {
        Slider::valueChanged();
        startArcAnimation();
    }

    void AnimatedKnob::snapToCurrentValue()
    {
        if (currentAnimator.has_value())
        {
            animationController.removeAnimator (*currentAnimator);
            currentAnimator.reset();
        }

        displayProportion = juce::jlimit (0.0f, 1.0f, static_cast<float> (valueToProportionOfLength (getValue())));
        repaint();
    }

    void AnimatedKnob::startArcAnimation()
    {
        const float target = juce::jlimit (0.0f, 1.0f, static_cast<float> (valueToProportionOfLength (getValue())));
        const float start = displayProportion;

        if (std::abs (target - start) < 0.001f)
            return;

        if (currentAnimator.has_value())
        {
            animationController.removeAnimator (*currentAnimator);
            currentAnimator.reset();
        }

        currentAnimator = animationController.runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (250.0)
                .withEasing (juce::Easings::createEaseOut())
                .withOnStartReturningValueChangedCallback (
                    [this, start, target]() -> juce::ValueAnimatorBuilder::ValueChangedCallback
                    {
                        return [this, start, target](float progress)
                        {
                            displayProportion = start + (target - start) * progress;
                            repaint();
                        };
                    }));
    }

    void AnimatedKnob::drawArcGlow (juce::Graphics& g)
    {
        if (displayProportion <= 0.0f)
            return;

        const float activeLevel = 1.0f - animationController.getBypassMix();
        if (activeLevel <= 0.0f)
            return;

        const auto bounds = getLocalBounds().toFloat();
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();

        const float currentAngle = kStartAngle + displayProportion * (kEndAngle - kStartAngle);
        const float arcRadius = radius - kArcInset;

        juce::Path valueArc;
        valueArc.addCentredArc (centreX, centreY, arcRadius, arcRadius, 0.0f,
                                kStartAngle, currentAngle, true);

        const auto glowColour = findColour (juce::Slider::rotarySliderFillColourId);
        if (glowColour.isTransparent())
            return;

        melatonin::DropShadow glow (glowColour, kGlowRadius);
        glow.setOpacity (static_cast<double> (activeLevel));
        glow.render (g, valueArc, juce::PathStrokeType (kArcThickness,
                                                        juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
    }
}
