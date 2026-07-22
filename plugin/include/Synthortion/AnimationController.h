#pragma once

#include <juce_animation/juce_animation.h>
#include <atomic>

namespace synthortion
{
    /** Centralized VBlank-driven animator registry.

        Owns a single juce::VBlankAnimatorUpdater that is synchronised to the refresh rate of the
        monitor displaying the provided Component. All animated UI elements register their
        juce::Animator instances here so that motion is orchestrated from one timeline.

        Also manages a shared global bypass transition value in [0, 1] (active -> bypassed) so that
        all UI elements can dim or fade simultaneously when bypass is engaged.
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

        /** Stop and clear every Animator owned by this controller.

            Called from the editor destructor (before any child components or
            parameter attachments are destroyed) so that no VBlank-driven
            callback can fire into a half-destroyed UI. The per-component
            Animators registered via addAnimator/runAnimator are held by their
            owning components as strong references; the VBlank updater stores
            only weak references and auto-removes them once those components
            are destroyed, so only the controller-owned bypass animator needs
            explicit clearing here.
        */
        void clearAllAnimators() noexcept;

        /** True while a bypass transition animator is registered and not yet
            cleared. */
        bool hasActiveBypassAnimator() const noexcept;

        /** Begin the global bypass fade transition.
            @param bypassed true fades everything to the dimmed bypassed state,
                            false restores the active state.
        */
        void startBypassTransition (bool bypassed);

        /** Current global bypass mix in [0, 1].
            0 = fully active, 1 = fully bypassed/dimmed.
        */
        float getBypassMix() const noexcept { return bypassMix.load(); }

        /** Directly set the bypass mix. Intended for tests and one-off initialisation. */
        void setBypassMix (float value) noexcept;

        /** Step count for bypass transition quantisation (N = 8). */
        static constexpr int kBypassTransitionSteps = 8;

        /** Quantises a continuous progress in [0, 1] to the nearest hard step. */
        static float quantizeBypassProgress (float progress) noexcept;

    private:
        juce::VBlankAnimatorUpdater updater;
        std::atomic<float> bypassMix { 0.0f };
        std::optional<juce::Animator> bypassAnimator;
        float bypassTarget = 0.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationController)
    };
}
