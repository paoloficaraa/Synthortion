#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <concepts>

namespace synthortion::dsp {

// POD parameter structs for modules
struct WarmDistortionParams {
    float drive = 0.0f;               // 0.0 .. 1.0 (COLOR)
    bool volumeCompensation = false;  // internal auto-gain compensation toggle
};

struct BitCrusherParams {
    float mix = 0.0f;                 // 0.0 .. 1.0 (BITCRUSH)
    float bitDepth = 8.0f;            // 1.0 .. 16.0 bits
    float sampleRateReduction = 6000.0f; // target downsample rate in Hz
};

struct PingPongDelayParams {
    float delayTimeMs = 250.0f;       // 1.0 .. 2000.0 ms
    float mix = 0.0f;                 // 0.0 .. 1.0
    float feedback = 0.4f;            // 0.0 .. 0.95
    float dampingFrequency = 12000.0f;// Hz (high-cut damping in feedback loop)
};

struct ChorusParams {
    float mix = 0.0f;                 // 0.0 .. 1.0
    bool wide = false;                // stereo phase widening mode
};

// DspModule concept
template <typename T, typename Params>
concept DspModule = requires(T& m, const T& cm, juce::AudioBuffer<float>& buffer, const juce::dsp::ProcessSpec& spec, const Params& params) {
    { m.prepare(spec) } -> std::same_as<void>;
    { m.reset() } noexcept -> std::same_as<void>;
    { m.process(buffer, params) } -> std::same_as<void>;
    { cm.getLatencySamples() } noexcept -> std::convertible_to<int>;
};

} // namespace synthortion::dsp
