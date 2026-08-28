#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include "Synthortion/DspModule.h"

namespace synthortion::dsp {

// 3-voice BBD chorus topology with decoupled multi-rate LFOs,
// Linkwitz-Riley 4th-order crossover at 320 Hz for mono low-end preservation,
// continuous stereo width scaling (0..60 deg phase spread),
// and volume-normalized multi-tap summing (G_norm = 1.0 / 3^0.75).
class SynthortionChorus
{
public:
    SynthortionChorus() = default;
    ~SynthortionChorus() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer, const ChorusParams& params);
    void reset() noexcept;
    int getLatencySamples() const noexcept { return 0; }

    void setChorusMix(float mix);

    static constexpr int kNumVoices = 3;
    static constexpr int kNumChannels = 2;
    static constexpr float kSmoothingTimeSeconds = 0.05f;

    // Fixed specification constants
    static constexpr float kBaseDelayMs = 5.0f;
    static constexpr float kDepthMs = 1.5f;
    static constexpr float kLfo1FreqHz = 0.45f;
    static constexpr float kLfo2FreqHz = 1.25f;
    static constexpr float kLfo3FreqHz = 2.45f;
    static constexpr float kCrossoverFreqHz = 320.0f;
    static constexpr float kMaxStereoPhaseSpreadDeg = 60.0f;

    // Mathematical specification methods
    static float getLfoRate(int voiceIndex) noexcept
    {
        static constexpr std::array<float, kNumVoices> kLfoRates = { kLfo1FreqHz, kLfo2FreqHz, kLfo3FreqHz };
        if (voiceIndex >= 0 && voiceIndex < kNumVoices)
            return kLfoRates[static_cast<size_t>(voiceIndex)];
        return 0.0f;
    }

    static float getVoicePhaseOffsetRad(int voiceIndex) noexcept
    {
        static constexpr std::array<float, kNumVoices> kPhaseOffsets = {
            0.0f,
            2.0f * juce::MathConstants<float>::pi / 3.0f,
            4.0f * juce::MathConstants<float>::pi / 3.0f
        };
        if (voiceIndex >= 0 && voiceIndex < kNumVoices)
            return kPhaseOffsets[static_cast<size_t>(voiceIndex)];
        return 0.0f;
    }

    static float calculateBaseDelaySamples(double sampleRate) noexcept
    {
        const double fs = (sampleRate > 0.0) ? sampleRate : 44100.0;
        return static_cast<float>(fs * (static_cast<double>(kBaseDelayMs) / 1000.0));
    }

    static float calculateDepthSamples(double sampleRate) noexcept
    {
        const double fs = (sampleRate > 0.0) ? sampleRate : 44100.0;
        return static_cast<float>(fs * (static_cast<double>(kDepthMs) / 1000.0));
    }

    static float calculateStereoPhaseOffsetRad(float width) noexcept
    {
        const float w = juce::jlimit(0.0f, 1.0f, width);
        return w * (kMaxStereoPhaseSpreadDeg * (juce::MathConstants<float>::pi / 180.0f));
    }

    static float calculateNormalizedGain() noexcept
    {
        return 1.0f / std::pow(3.0f, 0.75f);
    }

    static float getCrossoverFrequency() noexcept
    {
        return kCrossoverFreqHz;
    }

private:
    double sampleRate = 44100.0;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };

    juce::dsp::LinkwitzRileyFilter<float> crossoverFilter;

    juce::LinearSmoothedValue<float> smoothedMix { 0.0f };
    juce::LinearSmoothedValue<float> smoothedWidth { 0.5f };

    std::array<float, kNumVoices> lfoPhases { 0.0f, 0.0f, 0.0f };

    float baseDelaySamples = 0.0f;
    float depthSamples = 0.0f;
    float normalizedGain = 0.43869134f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthortionChorus)
};

static_assert(DspModule<SynthortionChorus, ChorusParams>);

} // namespace synthortion::dsp

namespace synthortion {
using SynthortionChorus = dsp::SynthortionChorus;
}
