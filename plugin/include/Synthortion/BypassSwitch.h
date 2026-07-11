#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_animation/juce_animation.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <melatonin_blur/melatonin_blur.h>
#include <optional>

namespace synthortion
{
    /** Custom hardware-style bypass toggle with a snappily animated vertical lever and violet LED glow.

        The lever slides up (ON) and down (OFF) with a short ease-out cubic curve that snaps into the
        end stop without overshooting. A violet LED to the right of the track fades in and out, rendered
        with a Melatonin Blur glow.
    */
    class BypassSwitch final : public juce::Button
    {
    public:
        explicit BypassSwitch (AnimationController* animationController = nullptr);
        ~BypassSwitch() override;

        bool isLedOn() const noexcept;

    private:
        void paintButton (juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
        void resized() override;
        void buttonStateChanged() override;

        void startAnimation();

        AnimationController* controller;
        float animationProgress { 0.0f };
        float animationStart { 0.0f };
        float animationTarget { 0.0f };
        bool animationRunning { false };

        std::optional<juce::Animator> currentAnimator;
        melatonin::DropShadow ledGlow;

        static constexpr float kTrackWidth = 28.0f;
        static constexpr float kTrackHeight = 56.0f;
        static constexpr float kTrackCornerRadius = 14.0f;
        static constexpr float kHandleSize = 22.0f;
        static constexpr float kHandleCornerRadius = 11.0f;
        static constexpr float kLedSize = 8.0f;
        static constexpr float kGlowRadius = 10.0f;
        static constexpr float kTrackInset = 3.0f;
        static constexpr double kAnimationDurationMs = 120.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassSwitch)
    };
}
