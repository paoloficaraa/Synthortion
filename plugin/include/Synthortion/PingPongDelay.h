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

private:
    juce::dsp::DelayLine<float> delayLineLeft;
    juce::dsp::DelayLine<float> delayLineRight;
    juce::dsp::IIR::Filter<float> dampingFilterLeft;
    juce::dsp::IIR::Filter<float> dampingFilterRight;

    float delayTimeMs = 250.0f;
    float delayMix = 0.0f;
    float feedback = 0.4f;
    double sampleRate = 0.0; // Initialize to 0 until prepare() is called

    void updateDelayTime();
};