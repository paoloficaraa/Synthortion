#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// Lo-fi bit reduction effect with sample rate and bit depth reduction
class BitCrusher
{
public:
    BitCrusher();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    
    void setBitCrushMix(float mix);
    
private:
    float sampleRateReduction = 6000.0f;
    float bitDepth = 8.0f;
    float ditherAmount = 0.4f;
    float adcQuality = 0.95f;
    float bitCrushMix = 0.0f;
    
    double sampleRate = 44100.0;
    
    float holdSampleLeft = 0.0f;
    float holdSampleRight = 0.0f;
    int holdCounterLeft = 0;
    int holdCounterRight = 0;
    int downsampleRatio = 1;
    
    float cachedAdcNoiseAmount = 0.0f;
    float cachedDitherScale = 0.0f;
    float quantizationStep = 0.0f;
    
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    juce::Random randomGenerator;
    
    void updateParameters();
};