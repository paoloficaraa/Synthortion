#pragma once

#include "Synthortion/AnimationController.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** A rotary slider with a hybrid flat/relief look, animated LED value arc and accent glow.

        The knob inherits from juce::Slider so standard APVTS SliderAttachments work unchanged.
        Value changes drive a short ease-out animation on the displayed LED arc via the shared
        AnimationController. A blurred accent glow is rendered around the active arc using
        Melatonin Blur.
    */
    class AnimatedKnob final : public juce::Slider
    {
    public:
        explicit AnimatedKnob (AnimationController& controller);
        ~AnimatedKnob() override;

        void paint (juce::Graphics& g) override;
        void valueChanged() override;

        /** Jump the animated arc to the current slider value without animating. */
        void snapToCurrentValue();

    private:
        void startArcAnimation();
        void drawArcGlow (juce::Graphics& g);

        AnimationController& animationController;
        std::optional<juce::Animator> currentAnimator;
        float displayProportion = 0.0f;

        static constexpr float kStartAngle = juce::MathConstants<float>::pi * 1.25f;
        static constexpr float kEndAngle = juce::MathConstants<float>::pi * 2.75f;
        static constexpr float kArcInset = 6.0f;
        static constexpr float kArcThickness = 2.5f;
        static constexpr int kGlowRadius = 6;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedKnob)
    };
}
