#pragma once

#include "Synthortion/AudioScopeRingBuffer.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace synthortion
{
    /** Real-time dual-trace oscilloscope with ghost trails and idle breathing animation. */
    class OscilloscopeComponent final : public juce::Component,
                                        private juce::Timer
    {
    public:
        explicit OscilloscopeComponent (AudioScopeRingBuffer& ringBuffer);

        void paint (juce::Graphics& g) override;
        void resized() override;

        void setBypassed (bool bypassed) noexcept;

        const juce::AudioBuffer<float>& getCurrentInputBuffer() const noexcept { return scratchInput; }
        const juce::AudioBuffer<float>& getCurrentOutputBuffer() const noexcept { return scratchOutput; }

    private:
        void timerCallback() override;

        void drawScanlines (juce::Graphics& g);
        void drawTrace (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                        juce::Colour colour, float alpha);

        AudioScopeRingBuffer& ringBuffer;

        static constexpr int kHistoryFrames = 3;
        static constexpr int kWindowSize = 512;
        static constexpr int kTimerHz = 60;

        std::array<juce::AudioBuffer<float>, kHistoryFrames> history;
        int historyIndex = 0;

        juce::AudioBuffer<float> scratchInput { 2, kWindowSize };
        juce::AudioBuffer<float> scratchOutput { 2, kWindowSize };

        float breathPhase = 0.0f;
        bool bypassed = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeComponent)
    };
}
