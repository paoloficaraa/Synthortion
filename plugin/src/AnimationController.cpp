#include "Synthortion/AnimationController.h"

namespace synthortion
{
    AnimationController::AnimationController (juce::Component* vBlankComponent)
        : updater (vBlankComponent)
    {
    }

    AnimationController::~AnimationController() = default;

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
}
