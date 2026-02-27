#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ParametricEQ
{
public:
    ParametricEQ();
    ~ParametricEQ() = default;

    void prepare(const juce::dsp::ProcessSpec &spec);
    void process(const juce::dsp::ProcessContextReplacing<float> &context);

    void setLowCut(float frequency, float q, bool enabled = true);
    void setLowMid(float frequency, float gain, float q);
    void setHighMid(float frequency, float gain, float q);
    void setHighCut(float frequency, float q, bool enabled = true);

    int getLatencySamples() const;
    std::vector<float> getFrequencyResponse(const std::vector<float> &frequencies) const;

private:
    static constexpr float kMinFrequency = 20.0f;
    static constexpr float kMaxFrequency = 20000.0f;
    static constexpr float kNyquistFactor = 0.45f;
    static constexpr float kMinQ = 0.1f;
    static constexpr float kButterworthQ = 0.707f;
    static constexpr float kGainThreshold = 0.001f;
    static constexpr float kQTolerance = 0.05f;
    static constexpr float kHighCutMinFreq = 5000.0f;

    using FilterType = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                                        juce::dsp::IIR::Coefficients<float>>;

    // Filter instances (12dB/octave for cuts using cascaded 6dB stages)
    FilterType lowCutFilter;
    FilterType lowCutFilter2;
    FilterType lowMidFilter;
    FilterType highMidFilter;
    FilterType highCutFilter;
    FilterType highCutFilter2;

    // Sample rate and state
    double sampleRate = 44100.0;
    bool isPrepared = false;

    // Filter parameters - Low Cut
    float lowCutFreq = 100.0f;
    float lowCutQ = kButterworthQ;
    bool lowCutEnabled = false;

    // Filter parameters - Low Mid
    float lowMidFreq = 500.0f;
    float lowMidGain = 0.0f;
    float lowMidQ = kButterworthQ;

    // Filter parameters - High Mid
    float highMidFreq = 2000.0f;
    float highMidGain = 0.0f;
    float highMidQ = kButterworthQ;

    // Filter parameters - High Cut
    float highCutFreq = kMaxFrequency;
    float highCutQ = kButterworthQ;
    bool highCutEnabled = false;

    // Cached state for optimization
    bool lowMidActive = false;
    bool highMidActive = false;

    void updateFilters();
    void updateActiveStates();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};
