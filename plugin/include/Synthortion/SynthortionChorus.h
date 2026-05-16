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
    juce::dsp::IIR::Filter<float> feedbackFilter[2];
    juce::dsp::LinkwitzRileyFilter<float> crossoverFilter[2];

    juce::SmoothedValue<float> smoothedMix;
    

    double sampleRate = 44100.0;
    float lfoPhase = 0.0f;

    static constexpr float baseDelayMs = 15.0f;
    static constexpr int numVoices = 3;

    // Fixed engine constants
    static constexpr float targetDelayMs = 5.1f;
    static constexpr float targetRateHz = 1.1f;
    static constexpr float crossoverFreq = 400.0f;
    static constexpr float targetPhaseOffsetDeg = 45.0f;

    // Interpolated state based on CHORUS_MIX (0-1)
    float interpolatedDepth = 0.0f;
    float interpolatedPhaseOffsetRad = 0.0f;

    // Precomputed samples for base delay
    float baseSamples = 0.0f;
};
