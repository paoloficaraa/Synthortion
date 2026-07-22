#pragma once

#include "Synthortion/AnimationController.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Modern Deadlock rotary knob: a dark matte (#121214) domed cap with a
        radial interior gradient, a constant 2 px elevation shadow beneath the
        cap, a white pointer needle and a segmented value arc that are always
        visible (no hover gating). Hover adds a subtle outer ring glow on the
        cap rim (no colour inversion). During an active drag the arc segments
        gain a halo glow and the pointer thickens; both fade back to flat white
        over 300 ms after the mouse is released. A 50 ms haloing pulse fires at
        the pointer tip each time a step boundary is crossed (detent notch
        simulation). Value changes drive a 250 ms Step animation quantised to
        the style's step count.

        Two rendering styles are supported via SynthortionLookAndFeel:
        - Canonical (large): 16-step segmented arc.
        - Outline (small): 8-step segmented arc + 1 px #FFF cap outline.
    */
    class AnimatedKnob final : public juce::Slider
    {
    public:
        enum class KnobStyle
        {
            Canonical,
            Outline
        };

        explicit AnimatedKnob (AnimationController& controller);
        ~AnimatedKnob() override;

        void paint (juce::Graphics& g) override;
        void valueChanged() override;

        void mouseEnter (const juce::MouseEvent& e) override;
        void mouseExit (const juce::MouseEvent& e) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseUp (const juce::MouseEvent& e) override;

        /** Set the brutalist rendering style (Canonical large / Outline small). */
        void setKnobStyle (KnobStyle style) noexcept { knobStyle = style; }
        KnobStyle getKnobStyle() const noexcept { return knobStyle; }

        /** Number of discrete steps the displayed value snaps to (16 Canonical / 8 Outline). */
        int getStepCount() const noexcept { return knobStyle == KnobStyle::Canonical ? kCanonicalSteps : kOutlineSteps; }

        /** Quantised-step easing used by startArcAnimation. Maps a continuous
            progress in [0, 1] to the nearest hard step boundary in [0, 1].
        */
        static float quantizeStepProgress (float progress, int steps) noexcept;

        /** Quantised-step easing used by the drag-glow fade. Maps a continuous
            progress in [0, 1] to the nearest hard step boundary in [0, 1]
            across kDragGlowFadeSteps steps.
        */
        static float quantizeDragGlowProgress (float progress) noexcept;

        /** Current displayed value proportion in [0, 1]. */
        float getDisplayProportion() const noexcept { return displayProportion; }

        /** Jump the animated arc to the current slider value without animating. */
        void snapToCurrentValue();

        /** Update the displayed value proportion in [0, 1] and evaluate the
            detent step: if the new proportion snaps to a different step index
            than the last one, a 50 ms detent pulse fires at the pointer tip.
            This is the single entry point used by both the active-drag path and
            the arc animation callback, and is exposed so the detent behaviour
            can be exercised deterministically without a host VBlank. */
        void applyDisplayProportion (float proportion);

        /** True while the mouse is over the cap (and not dragging). Drives the
            subtle outer ring glow on the cap rim. */
        bool isHovering() const noexcept { return hovering; }

        /** True while the pointer is actively dragged. The drag-glow mix is
            held at 1 during a drag and fades back to 0 over kDragGlowFadeMs
            after release. */
        bool isDragging() const noexcept { return dragging; }

        /** Current drag-glow mix in [0, 1]. 1 during a drag, decaying to 0 over
            kDragGlowFadeMs after release. Drives the arc halo glow and the
            pointer-thickening boost. */
        float getDragGlowMix() const noexcept { return dragGlowMix; }

        /** True while the drag-glow fade animator is registered and not yet
            complete (i.e. the 300 ms post-release fade is in flight). */
        bool isDragGlowFading() const noexcept;

        /** Current detent-pulse progress in [0, 1]. 1 the instant a step
            boundary is crossed, decaying to 0 over kDetentPulseMs. Drives the
            haloing pulse rendered at the pointer tip. */
        float getDetentPulseProgress() const noexcept { return detentPulseProgress; }

        /** True while a detent-pulse animator is registered and not yet
            complete (i.e. the 50 ms pointer-tip halo is in flight). */
        bool isDetentPulseActive() const noexcept;

        /** Last step index the displayed value snapped to (for detent tests). */
        int getLastStepIndex() const noexcept { return lastStepIndex; }

        static constexpr int kCanonicalSteps = 16;
        static constexpr int kOutlineSteps = 8;

        /** Drag-glow fade duration in ms (arc halo + pointer thickening fade
            back to flat white after mouse release). */
        static constexpr int kDragGlowFadeMs = 300;
        /** Step count for the drag-glow fade quantisation. */
        static constexpr int kDragGlowFadeSteps = 8;
        /** Detent-pulse duration in ms (haloing pulse at the pointer tip on
            each step boundary crossing). */
        static constexpr int kDetentPulseMs = 50;
        /** Pointer thickness boost (in px) applied at full drag-glow mix. */
        static constexpr float kDragPointerThicknessBoost = 2.0f;

    private:
        void startArcAnimation();
        void startDragGlowFade();
        void triggerDetentPulse();
        void checkDetentStep();

        AnimationController& animationController;
        std::optional<juce::Animator> currentAnimator;
        std::optional<juce::Animator> dragGlowAnimator;
        std::optional<juce::Animator> detentPulseAnimator;
        float displayProportion = 0.0f;
        float dragGlowMix = 0.0f;
        float detentPulseProgress = 0.0f;
        int lastStepIndex = 0;
        bool hovering = false;
        bool dragging = false;
        KnobStyle knobStyle = KnobStyle::Canonical;

        static constexpr float kStartAngle = juce::MathConstants<float>::pi * 1.25f;
        static constexpr float kEndAngle = juce::MathConstants<float>::pi * 2.75f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedKnob)
    };
}
