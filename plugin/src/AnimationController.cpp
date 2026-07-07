#include "Synthortion/AnimationController.h"

namespace synthortion
{
    AnimationController::AnimationController (juce::Component* vBlankComponent)
        : updater (vBlankComponent)
    {
    }

    AnimationController::~AnimationController()
    {
        if (bypassAnimator.has_value() && ! bypassAnimator->isComplete())
            updater.removeAnimator (*bypassAnimator);
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
                .withEasing (juce::Easings::createEaseOut())
                .withValueChangedCallback ([this, start] (float progress)
                                           {
                                               bypassMix.store (start + (bypassTarget - start) * progress);
                                           }));
    }

    void AnimationController::setBypassMix (float value) noexcept
    {
        bypassMix.store (juce::jlimit (0.0f, 1.0f, value));
    }
}
