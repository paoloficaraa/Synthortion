#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_animation/juce_animation.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>

namespace synthortion
{
    class GlitchOverlay;

    /** Tactile push-button bypass toggle. The button body is a slightly
        rounded cap with a 1 px dual-tone inset bezel (lighter outer edge,
        darker inner edge) and a vertical metallic gradient (charcoal at the
        top to near-black at the bottom) simulating a physical actuator. A
        small circular LED aperture sits on the cap: active (signal flows)
        emits a bright white glow with a subtle halo; bypassed is dark /
        near-invisible.

        State semantics: animationProgress 0 = active (LED bright, cap
        depressed 1-2 px), 1 = bypassed (LED dark, cap at rest). A toggle
        drives a 100 ms Step-quantised easing (N = 8) so the cap sinks on
        bypassed->active and lifts back on active->bypassed while the LED
        ramps, and fires a GlitchOverlay horizontal Slice glitch burst. The
        shared global bypass mix (AnimationController) fades every component
        in the same 8 hard steps during the transition.
    */
    class BypassSwitch final : public juce::Button
    {
    public:
        explicit BypassSwitch (AnimationController* animationController = nullptr);
        ~BypassSwitch() override;

        bool isBypassed() const noexcept;

        void setGlitchOverlay (GlitchOverlay* overlay) noexcept { glitchOverlay = overlay; }

        float getAnimationProgress() const noexcept { return animationProgress; }

        /** LED glow intensity in [0, 1]: 1 = fully active (bright white), 0 = bypassed (dark). */
        float getLedBrightness() const noexcept;

        /** Cap depress offset in pixels: kDepressPixels when active (sunk), 0 when bypassed (rest). */
        float getDepressOffset() const noexcept;

        /** The button cap rectangle in local coords, inset from the bounds and translated by the depress offset. */
        juce::Rectangle<float> getBodyBounds() const noexcept;

        /** The circular LED aperture bounds in local coords, riding on the cap (depress-applied). */
        juce::Rectangle<float> getLedBounds() const noexcept;

        static constexpr int kBypassSteps = 8;

        /** Depress/release animation duration per issue #30 (100 ms). */
        static constexpr double kAnimationDurationMs = 100.0;

        /** Cap travel distance when active (1-2 px sink per issue #30). */
        static constexpr float kDepressPixels = 2.0f;

        /** Slightly rounded cap corners, distinguishing the button from flat panels. */
        static constexpr float kButtonCornerRadius = 6.0f;

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
        static constexpr float kBezelInset = 3.0f;
        static constexpr float kLedRadius = 4.0f;
        static constexpr float kLedTopOffset = 15.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassSwitch)
    };
}