#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_animation/juce_animation.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>

namespace synthortion
{
    class GlitchOverlay;

    /** Brutalist Block toggle. The bypass state is encoded entirely by colour
        inversion: the active (signal flows) state renders a solid #FFF block
        with a #000 BYPASS label centered inside; the bypassed state renders a
        solid #000 block with a #FFF BYPASS label and a 1 px #FFF outline around
        the block. No LED, no rounded track and no lever survive.

        State transitions drive a Step-quantised easing (N = 8) preserving the
        existing 120 ms duration, and fire a GlitchOverlay horizontal Slice
        glitch burst that visibly slides across the editor background for the
        ~150 ms burst window matched to the step duration.
    */
    class BypassSwitch final : public juce::Button
    {
    public:
        explicit BypassSwitch (AnimationController* animationController = nullptr);
        ~BypassSwitch() override;

        bool isBypassed() const noexcept;

        void setGlitchOverlay (GlitchOverlay* overlay) noexcept { glitchOverlay = overlay; }

        float getAnimationProgress() const noexcept { return animationProgress; }

        static constexpr int kBypassSteps = 8;

    private:
        void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
        void resized() override;
        void buttonStateChanged() override;

        void startAnimation();

        AnimationController* controller;
        GlitchOverlay* glitchOverlay = nullptr;
        float animationProgress { 0.0f };
        float animationStart { 0.0f };
        float animationTarget { 0.0f };
        bool animationRunning { false };

        std::optional<juce::Animator> currentAnimator;

        static constexpr int kBypassLabelHeight = 16;
        static constexpr double kAnimationDurationMs = 120.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassSwitch)
    };
}