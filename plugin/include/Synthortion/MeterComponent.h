#pragma once

#include "Synthortion/AnimationController.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>

namespace synthortion
{
    /** Vertical RMS/peak-hold meter rendered as a 16-segment LED ladder.

        The meter is drawn as 16 hard-threshold vertical LED blocks stacked
        from the bottom up. Each lit block's brightness is gated by its dB
        position: lit segments at or above kThresholdSegmentIndex (the -12 dB
        band) render as pure #FFF, lit segments below the threshold render
        at kDimmedAlpha (~0.4 alpha white) per issue #31, and unlit segments
        render as pure #000. The substrate is a dead-flat #000 fill.

        A 1px #FFF outline frames the bar with notched reference ticks at
        -6/-12/-24 dB carved into the outline. The peak-hold marker is a
        single 1px #FFF line that jumps in discrete 1/16 steps; its decay
        uses a quantized Step easing (N = kSegmentCount) so it falls one
        segment at a time via the shared AnimationController.
    */
    class MeterComponent final : public juce::Component
    {
    public:
        explicit MeterComponent (AnimationController& controller);
        ~MeterComponent() override;

        void paint (juce::Graphics& g) override;
        void resized() override;

        /** Push a stereo buffer to update the meter. */
        void updateFromBuffer (const juce::AudioBuffer<float>& buffer);

        void setBypassed (bool bypassed) noexcept;

        float getRmsDb() const noexcept { return rmsDb; }
        float getPeakDb() const noexcept { return peakDb; }
        float getAnimatedPeakDb() const noexcept { return animatedPeakDb; }
        bool isPeakHoldAnimating() const noexcept { return decayAnimator.has_value(); }

        /** Number of LED segments the meter is split into (hardcoded per spec). */
        static constexpr int kSegmentCount = 16;

        /** Segment index corresponding to the -12 dB threshold: lit segments
            with index >= kThresholdSegmentIndex render at pure #FFF; lit
            segments with index < kThresholdSegmentIndex render dimmed at
            kDimmedAlpha (~0.4 alpha) per issue #31.
        */
        static constexpr int kThresholdSegmentIndex = 12;

        /** Alpha (0..1) used for lit LED segments below the -12 dB threshold
            per issue #31 (dimmed white over a black substrate).
        */
        static constexpr float kDimmedAlpha = 0.4f;

        /** Quantized-step easing used by startPeakDecay. Maps a continuous
            progress in [0, 1] to the nearest hard step boundary in [0, 1].
        */
        static float quantizeStepProgress (float progress, int steps) noexcept;

        /** Map a dB level to the index of the highest lit segment (0..kSegmentCount). */
        int dbToSegmentIndex (float db) const noexcept;

    private:
        void startPeakDecay (float fromDb, float durationMs);
        float computeRMS (const juce::AudioBuffer<float>& buffer) const;
        float levelToHeight (float db) const;

        static constexpr float kNormalDecayMs = 1200.0f;
        static constexpr float kBypassDecayMs = 250.0f;
        static constexpr float kMinDb = -60.0f;
        static constexpr float kMaxDb = 6.0f;

        AnimationController& animationController;
        std::optional<juce::Animator> decayAnimator;

        float rmsDb = kMinDb;
        float peakDb = kMinDb;
        float animatedPeakDb = kMinDb;
        bool bypassed = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
    };
}
