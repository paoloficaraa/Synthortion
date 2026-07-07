#pragma once

#include <juce_animation/juce_animation.h>

namespace synthortion
{
    /** Centralized VBlank-driven animator registry.

        Owns a single juce::VBlankAnimatorUpdater that is synchronised to the refresh rate of the
        monitor displaying the provided Component. All animated UI elements register their
        juce::Animator instances here so that motion is orchestrated from one timeline.
    */
    class AnimationController final
    {
    public:
        explicit AnimationController (juce::Component* vBlankComponent);
        ~AnimationController();

        /** Build a ValueAnimator from a builder and register it with the VBlank updater. */
        juce::Animator runAnimator (juce::ValueAnimatorBuilder builder);

        /** Register an already-built Animator with the VBlank updater. */
        void addAnimator (const juce::Animator& animator);

        /** Remove an Animator from the VBlank updater. */
        void removeAnimator (const juce::Animator& animator);

    private:
        juce::VBlankAnimatorUpdater updater;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationController)
    };
}
