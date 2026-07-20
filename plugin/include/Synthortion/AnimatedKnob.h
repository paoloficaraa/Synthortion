#pragma once

#include "Synthortion/AnimationController.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Brutalist rotary knob: a solid #FFF disc (Canonical) or #000 outlined
        disc (Outline) with a thick single-line pointer, an optional segmented
        value arc / rim ticks, a hard twin shadow on hover/drag, and a
        VBlank-driven Step animation on the displayed value.

        Two rendering styles are supported via SynthortionLookAndFeel:
        - Canonical (large): #FFF disc, thick #000 pointer, 16-step LED arc.
        - Outline (small): #000 disc, 1px #FFF outline, 8 rim ticks, thick #FFF pointer.
        Hover or drag Inverts the disc/pointer colours and draws a 1 px-offset
        twin shadow. Value changes drive a 250 ms Step animation quantised to
        the style's step count.
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

        /** Set the brutalist rendering style (Canonical large / Outline small). */
        void setKnobStyle (KnobStyle style) noexcept { knobStyle = style; }
        KnobStyle getKnobStyle() const noexcept { return knobStyle; }

        /** Number of discrete steps the displayed value snaps to (16 Canonical / 8 Outline). */
        int getStepCount() const noexcept { return knobStyle == KnobStyle::Canonical ? kCanonicalSteps : kOutlineSteps; }

        /** Quantised-step easing used by startArcAnimation. Maps a continuous
            progress in [0, 1] to the nearest hard step boundary in [0, 1].
        */
        static float quantizeStepProgress (float progress, int steps) noexcept;

        /** Current displayed value proportion in [0, 1]. */
        float getDisplayProportion() const noexcept { return displayProportion; }

        /** Jump the animated arc to the current slider value without animating. */
        void snapToCurrentValue();

        static constexpr int kCanonicalSteps = 16;
        static constexpr int kOutlineSteps = 8;

    private:
        void startArcAnimation();

        AnimationController& animationController;
        std::optional<juce::Animator> currentAnimator;
        float displayProportion = 0.0f;
        KnobStyle knobStyle = KnobStyle::Canonical;

        static constexpr float kStartAngle = juce::MathConstants<float>::pi * 1.25f;
        static constexpr float kEndAngle = juce::MathConstants<float>::pi * 2.75f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedKnob)
    };
}
