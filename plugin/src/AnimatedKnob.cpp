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

        if (dragGlowAnimator.has_value() && ! dragGlowAnimator->isComplete())
            animationController.removeAnimator (*dragGlowAnimator);

        if (detentPulseAnimator.has_value() && ! detentPulseAnimator->isComplete())
            animationController.removeAnimator (*detentPulseAnimator);
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

            applyDisplayProportion (juce::jlimit (0.0f, 1.0f, static_cast<float> (valueToProportionOfLength (getValue()))));
        }
        else
        {
            startArcAnimation();
        }
    }

    void AnimatedKnob::mouseEnter (const juce::MouseEvent& e)
    {
        Slider::mouseEnter (e);
        hovering = true;
        repaint();
    }

    void AnimatedKnob::mouseExit (const juce::MouseEvent& e)
    {
        Slider::mouseExit (e);
        hovering = false;
        repaint();
    }

    void AnimatedKnob::mouseDown (const juce::MouseEvent& e)
    {
        Slider::mouseDown (e);
        dragging = true;

        if (dragGlowAnimator.has_value())
        {
            if (! dragGlowAnimator->isComplete())
                animationController.removeAnimator (*dragGlowAnimator);
            dragGlowAnimator.reset();
        }

        dragGlowMix = 1.0f;
        repaint();
    }

    void AnimatedKnob::mouseUp (const juce::MouseEvent& e)
    {
        Slider::mouseUp (e);
        dragging = false;
        startDragGlowFade();
    }

    void AnimatedKnob::snapToCurrentValue()
    {
        if (currentAnimator.has_value())
        {
            animationController.removeAnimator (*currentAnimator);
            currentAnimator.reset();
        }

        displayProportion = juce::jlimit (0.0f, 1.0f, static_cast<float> (valueToProportionOfLength (getValue())));
        lastStepIndex = juce::jlimit (0, getStepCount(), juce::roundToInt (displayProportion * static_cast<float> (getStepCount())));
        repaint();
    }

    void AnimatedKnob::applyDisplayProportion (float proportion)
    {
        displayProportion = juce::jlimit (0.0f, 1.0f, proportion);
        checkDetentStep();
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

    float AnimatedKnob::quantizeDragGlowProgress (float progress) noexcept
    {
        const float clamped = juce::jlimit (0.0f, 1.0f, progress);
        const int stepIndex = juce::jlimit (0, kDragGlowFadeSteps, juce::roundToInt (clamped * static_cast<float> (kDragGlowFadeSteps)));
        return static_cast<float> (stepIndex) / static_cast<float> (kDragGlowFadeSteps);
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
                            applyDisplayProportion (start + (target - start) * progress);
                        };
                    }));
    }

    void AnimatedKnob::startDragGlowFade()
    {
        if (dragGlowAnimator.has_value())
        {
            if (! dragGlowAnimator->isComplete())
                animationController.removeAnimator (*dragGlowAnimator);
            dragGlowAnimator.reset();
        }

        const float start = dragGlowMix;

        if (start <= 0.001f)
        {
            dragGlowMix = 0.0f;
            repaint();
            return;
        }

        dragGlowAnimator = animationController.runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (static_cast<double> (kDragGlowFadeMs))
                .withEasing ([] (float progress)
                             {
                                 return quantizeDragGlowProgress (progress);
                             })
                .withValueChangedCallback ([this, start] (float progress)
                                           {
                                               dragGlowMix = juce::jlimit (0.0f, 1.0f, start * (1.0f - progress));
                                               repaint();
                                           }));
    }

    void AnimatedKnob::triggerDetentPulse()
    {
        if (detentPulseAnimator.has_value())
        {
            if (! detentPulseAnimator->isComplete())
                animationController.removeAnimator (*detentPulseAnimator);
            detentPulseAnimator.reset();
        }

        detentPulseProgress = 1.0f;

        detentPulseAnimator = animationController.runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (static_cast<double> (kDetentPulseMs))
                .withValueChangedCallback ([this] (float progress)
                                           {
                                               detentPulseProgress = juce::jlimit (0.0f, 1.0f, 1.0f - progress);
                                               repaint();
                                           }));
    }

    void AnimatedKnob::checkDetentStep()
    {
        const int newStep = juce::jlimit (0, getStepCount(), juce::roundToInt (displayProportion * static_cast<float> (getStepCount())));

        if (newStep != lastStepIndex)
        {
            lastStepIndex = newStep;
            triggerDetentPulse();
        }
    }

    bool AnimatedKnob::isDragGlowFading() const noexcept
    {
        return dragGlowAnimator.has_value() && ! dragGlowAnimator->isComplete();
    }

    bool AnimatedKnob::isDetentPulseActive() const noexcept
    {
        return detentPulseAnimator.has_value() && ! detentPulseAnimator->isComplete();
    }
}
