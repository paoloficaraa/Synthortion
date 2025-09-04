#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ParametricEQ
{
public:
    ParametricEQ();
    ~ParametricEQ();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void process(juce::AudioBuffer<float> &buffer);

    // EQ Band setters
    void setLowCut(float frequency, float q);
    void setLowMid(float frequency, float gain, float q);
    void setHighMid(float frequency, float gain, float q);
    void setHighCut(float frequency, float q);

    // Get frequency response for visualization
    std::vector<float> getFrequencyResponse(const std::vector<float> &frequencies);

private:
    // 4-band EQ filters - using ProcessorDuplicator for stereo support
    using FilterType = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    FilterType lowCutFilter;
    FilterType lowMidFilter;
    FilterType highMidFilter;
    FilterType highCutFilter;

    double sampleRate = 44100.0;
    bool isPrepared = false;

    void updateFilters();

    // Filter parameters
    float lowCutFreq = 20.0f, lowCutQ = 0.7f;
    float lowMidFreq = 350.0f, lowMidGain = 0.0f, lowMidQ = 1.0f;
    float highMidFreq = 3800.0f, highMidGain = 0.0f, highMidQ = 1.0f;
    float highCutFreq = 20000.0f, highCutQ = 0.7f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};
