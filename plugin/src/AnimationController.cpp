#include "Synthortion/AnimationController.h"

namespace synthortion
{
    AnimationController::AnimationController (juce::Component* vBlankComponent)
        : updater (vBlankComponent)
    {
    }

    AnimationController::~AnimationController()
    {
        clearAllAnimators();
    }

    juce::Animator AnimationController::runAnimator (juce::ValueAnimatorBuilder builder)
    {
        auto animator = builder.build();
        updater.addAnimator (animator);
        animator.start();
        return animator;
    }

    void AnimationController::addAnimator (const juce::Animator& animator)
    {
        updater.addAnimator (animator);
    }

    void AnimationController::removeAnimator (const juce::Animator& animator)
    {
        updater.removeAnimator (animator);
    }

    void AnimationController::clearAllAnimators() noexcept
    {
        if (bypassAnimator.has_value())
        {
            if (! bypassAnimator->isComplete())
                updater.removeAnimator (*bypassAnimator);

            bypassAnimator.reset();
        }
    }

    bool AnimationController::hasActiveBypassAnimator() const noexcept
    {
        return bypassAnimator.has_value();
    }

    void AnimationController::startBypassTransition (bool bypassed)
    {
        bypassTarget = bypassed ? 1.0f : 0.0f;
        const float start = bypassMix.load();

        if (bypassAnimator.has_value())
        {
            updater.removeAnimator (*bypassAnimator);
            bypassAnimator.reset();
        }

        if (std::abs (bypassTarget - start) < 0.001f)
            return;

        bypassAnimator = runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (300.0)
                .withEasing ([] (float progress)
                             {
                                 return quantizeBypassProgress (progress);
                             })
                .withValueChangedCallback ([this, start] (float progress)
                                           {
                                               bypassMix.store (start + (bypassTarget - start) * progress);
                                           }));
    }

    float AnimationController::quantizeBypassProgress (float progress) noexcept
    {
        constexpr int steps = kBypassTransitionSteps;
        const float clamped = juce::jlimit (0.0f, 1.0f, progress);
        const int stepIndex = juce::jlimit (0, steps, juce::roundToInt (clamped * static_cast<float> (steps)));
        return static_cast<float> (stepIndex) / static_cast<float> (steps);
    }

    void AnimationController::setBypassMix (float value) noexcept
    {
        bypassMix.store (juce::jlimit (0.0f, 1.0f, value));
    }
}
