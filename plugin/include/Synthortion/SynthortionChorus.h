#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>


class SynthortionChorus
{
public:
    SynthortionChorus() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void setChorusMix(float mix);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };
    juce::dsp::LinkwitzRileyFilter<float> crossoverLP[2];
    juce::dsp::LinkwitzRileyFilter<float> crossoverHP[2];

    juce::SmoothedValue<float> smoothedMix;

    double sampleRate = 44100.0;

    // Fixed Chorus constants
    static constexpr float kDelayMs = 15.025f;
    static constexpr float kDepthMs = 2.25f;
    static constexpr float kLfo1FreqHz = 0.45f;
    static constexpr float kLfo2FreqHz = 1.25f;
    static constexpr float kLfo3FreqHz = 2.45f;
    static constexpr float kCrossoverFreq = 320.24371f;
    static constexpr float kStereoPhaseOffsetDeg = 59.0f;

    // State variables
    float lfo1Phase = 0.0f;
    float lfo2Phase = 0.0f;
    float lfo3Phase = 0.0f;

    float baseDelaySamples = 0.0f;
    float depthSamples = 0.0f;
    float stereoPhaseOffsetRad = 0.0f;
};
