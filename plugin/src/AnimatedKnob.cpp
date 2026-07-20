#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/SynthortionLookAndFeel.h"

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

        if (getThumbBeingDragged() != -1)
        {
            if (currentAnimator.has_value())
            {
                animationController.removeAnimator (*currentAnimator);
                currentAnimator.reset();
            }

            displayProportion = juce::jlimit (0.0f, 1.0f, static_cast<float> (valueToProportionOfLength (getValue())));
            repaint();
        }
        else
        {
            startArcAnimation();
        }
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

    float AnimatedKnob::quantizeStepProgress (float progress, int steps) noexcept
    {
        if (steps <= 0)
            return progress;

        const float clamped = juce::jlimit (0.0f, 1.0f, progress);
        const int stepIndex = juce::jlimit (0, steps, juce::roundToInt (clamped * static_cast<float> (steps)));
        return static_cast<float> (stepIndex) / static_cast<float> (steps);
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

        const int steps = getStepCount();

        currentAnimator = animationController.runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (250.0)
                .withEasing ([steps](float progress)
                             {
                                 return quantizeStepProgress (progress, steps);
                             })
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
}
