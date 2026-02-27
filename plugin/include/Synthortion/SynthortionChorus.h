#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class SynthortionChorus
{
public:
    SynthortionChorus();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(const juce::dsp::ProcessContextReplacing<float>& context);
    void reset();

    void setChorusMix(float mix);
    void setCentreDelay(float delayMs);
    void setDepth(float depth);
    void setRate(float rateHz);
    void setFeedback(float feedback);

private:
    static constexpr float kDefaultCentreDelayMs = 15.0f;
    static constexpr float kDefaultDepth = 2.25f;
    static constexpr float kDefaultRateHz = 0.45f;
    static constexpr float kDefaultFeedback = 0.0f;
    static constexpr float kChorusInternalMix = 1.0f;

    static constexpr float kMinMix = 0.0f;
    static constexpr float kMaxMix = 1.0f;
    static constexpr float kMinDelay = 1.0f;
    static constexpr float kMaxDelay = 100.0f;
    static constexpr float kMinDepth = 0.0f;
    static constexpr float kMaxDepth = 10.0f;
    static constexpr float kMinRate = 0.01f;
    static constexpr float kMaxRate = 10.0f;
    static constexpr float kMinFeedback = 0.0f;
    static constexpr float kMaxFeedback = 0.95f;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::DryWetMixer<float> dryWetMixer;

    float chorusMix = 0.0f;
};