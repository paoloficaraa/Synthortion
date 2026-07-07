#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace synthortion
{
    /** Vertical RMS/peak-hold meter with reference tick marks. */
    class MeterComponent final : public juce::Component,
                                 private juce::Timer
    {
    public:
        MeterComponent();

        void paint (juce::Graphics& g) override;
        void resized() override;

        /** Push a stereo buffer to update the meter. */
        void updateFromBuffer (const juce::AudioBuffer<float>& buffer);

        void setBypassed (bool bypassed) noexcept;

    private:
        void timerCallback() override;

        float computeRMS (const juce::AudioBuffer<float>& buffer) const;
        float levelToHeight (float db) const;

        static constexpr int kTimerHz = 60;
        static constexpr float kReleaseRate = 0.8f; // dB per tick
        static constexpr float kMinDb = -60.0f;
        static constexpr float kMaxDb = 6.0f;

        float rmsDb = kMinDb;
        float peakDb = kMinDb;
        float peakHoldDb = kMinDb;
        bool bypassed = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
    };
}
