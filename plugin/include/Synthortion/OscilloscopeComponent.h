#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/AudioScopeRingBuffer.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <optional>

namespace synthortion
{
    /** Real-time dual-trace oscilloscope with ghost trails and idle breathing animation.

        Reads pre-DSP (input) and post-DSP (output) samples from the lock-free
        AudioScopeRingBuffer and renders them as two overlaid traces. Updates are
        driven by the AnimationController's VBlank timeline for fluid motion.
    */
    class OscilloscopeComponent final : public juce::Component
    {
    public:
        explicit OscilloscopeComponent (AudioScopeRingBuffer& ringBuffer,
                                        AnimationController* animationController = nullptr);
        ~OscilloscopeComponent() override;

        void paint (juce::Graphics& g) override;
        void resized() override;

        void setBypassed (bool bypassed) noexcept;

        /** Manual refresh entry point for tests and non-VBlank environments. */
        void refresh();

        const juce::AudioBuffer<float>& getCurrentInputBuffer() const noexcept { return scratchInput; }
        const juce::AudioBuffer<float>& getCurrentOutputBuffer() const noexcept { return scratchOutput; }

        bool isSilent() const noexcept { return silent; }
        bool isBypassed() const noexcept { return bypassed; }

        static constexpr int kWindowSize = 512;

    private:
        void startAnimation();
        void updateFrame();

        void drawScanlines (juce::Graphics& g);
        void drawTrace (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                        juce::Colour colour, float alpha, float amplitudeScale);
        void drawGhostTrails (juce::Graphics& g);
        void drawBreathLine (juce::Graphics& g);

        bool detectSilence (const juce::AudioBuffer<float>& buffer) const noexcept;
        void pushHistory (const juce::AudioBuffer<float>& buffer);

        AudioScopeRingBuffer& ringBuffer;
        AnimationController* controller;
        std::optional<juce::Animator> currentAnimator;
        bool animationRunning = false;

        static constexpr int kHistoryFrames = 3;
        static constexpr float kSilenceThreshold = 1.0e-4f;
        static constexpr float kBreathIncrement = 0.04f;
        static constexpr float kFlatlineDecay = 0.92f;
        static constexpr double kFrameDurationMs = 1000.0 / 60.0;

        std::array<juce::AudioBuffer<float>, kHistoryFrames> history;
        int historyIndex = 0;

        juce::AudioBuffer<float> scratchInput { 2, kWindowSize };
        juce::AudioBuffer<float> scratchOutput { 2, kWindowSize };

        float breathPhase = 0.0f;
        bool bypassed = false;
        bool silent = true;
        float flatlineAmplitude = 0.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeComponent)
    };
}
