#pragma once

#include "Synthortion/AnimationController.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>

namespace synthortion
{
    /** Vertical RMS/peak-hold meter with reference tick marks.

        The peak-hold marker jumps instantly to new transient peaks and then decays
        smoothly to the meter floor via the shared AnimationController.
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
