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
    void setRate(float rateHz);
    void setDepth(float depth);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };
    juce::dsp::IIR::Filter<float> feedbackFilter[2];

    juce::SmoothedValue<float> smoothedMix;
    
    float currentRate = 1.0f;
    float currentDepth = 0.5f;
    double sampleRate = 44100.0;
    float lfoPhase = 0.0f;

    static constexpr float baseDelayMs = 15.0f;
    static constexpr int numVoices = 3;
};