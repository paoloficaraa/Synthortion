#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class PingPongDelay
{
public:
    PingPongDelay();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void process(juce::AudioBuffer<float> &buffer);
    void reset();

    void setDelayTime(float timeMs);
    void setDelayMix(float mix);
    void setFeedback(float fb);
    void setDampingFrequency(float frequency);

private:
    static constexpr float kDefaultDelayTimeMs = 250.0f;
    static constexpr float kDefaultFeedback = 0.4f;
    static constexpr float kDefaultDampingFreq = 12000.0f;
    static constexpr float kMaxDelaySeconds = 2.0f;
    static constexpr float kSmoothingTimeSeconds = 0.05f;
    static constexpr float kMsToSeconds = 0.001f;
    static constexpr float kMinDelaySamples = 1.0f;
    static constexpr float kDelaySamplesSafetyMargin = 2.0f;

    static constexpr float kMinDelayTimeMs = 1.0f;
    static constexpr float kMaxDelayTimeMs = 2000.0f;
    static constexpr float kMinMix = 0.0f;
    static constexpr float kMaxMix = 1.0f;
    static constexpr float kMinFeedback = 0.0f;
    static constexpr float kMaxFeedback = 0.95f;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine{2048};

    juce::dsp::IIR::Filter<float> dampingFilterLeft;
    juce::dsp::IIR::Filter<float> dampingFilterRight;

    juce::SmoothedValue<float> smoothedDelayTime;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedFeedback;

    juce::dsp::DryWetMixer<float> dryWetMixer;

    float delayTimeMs = kDefaultDelayTimeMs;
    float delayMix = 0.0f;
    float feedback = kDefaultFeedback;
    float dampingFrequency = kDefaultDampingFreq;
    double sampleRate = 44100.0;
    bool isDelayLineClear = false;

    void updateDampingFilters();
};