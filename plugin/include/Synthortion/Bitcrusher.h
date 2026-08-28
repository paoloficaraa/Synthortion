#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include "Synthortion/DspModule.h"

namespace synthortion::dsp {

// Lo-fi bit reduction effect with progressive coupled trajectory,
// fractional phase accumulator with linear interpolation, and TPDF dither.
class BitCrusher
{
public:
    BitCrusher();
    ~BitCrusher() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer, const BitCrusherParams& params);
    void reset() noexcept;
    int getLatencySamples() const noexcept { return 0; }

    static constexpr int kNumChannels = 2;
    static constexpr float kSmoothingTimeSeconds = 0.02f;

    // Mathematical specification methods
    static float calculateBitDepth(float crush) noexcept
    {
        const float c = juce::jlimit(0.0f, 1.0f, crush);
        return 16.0f - 12.0f * std::pow(c, 1.5f);
    }

    static float calculateTargetSampleRate(float crush, double sampleRate) noexcept
    {
        const float c = juce::jlimit(0.0f, 1.0f, crush);
        const double fs = (sampleRate > 0.0) ? sampleRate : 44100.0;
        if (c <= 0.0f || fs <= 1500.0)
            return static_cast<float>(fs);

        const double ratio = 1500.0 / fs;
        const double exponent = std::pow(static_cast<double>(c), 1.8);
        return static_cast<float>(fs * std::pow(ratio, exponent));
    }

    static float calculateQuantizationStep(float bitDepth) noexcept
    {
        const float b = juce::jlimit(1.0f, 32.0f, bitDepth);
        return 2.0f / std::pow(2.0f, b);
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    bool isFirstSample = true;

    std::array<float, kNumChannels> previousSample{};
    std::array<float, kNumChannels> currentSample{};
    std::array<juce::Random, kNumChannels> randomGenerators;

    juce::LinearSmoothedValue<float> smoothedCrush{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitCrusher)
};

static_assert(DspModule<BitCrusher, BitCrusherParams>);

} // namespace synthortion::dsp

namespace synthortion {
using BitCrusher = dsp::BitCrusher;
}
