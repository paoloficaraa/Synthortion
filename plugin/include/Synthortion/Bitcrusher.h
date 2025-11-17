#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class BitCrusher
{
public:
    BitCrusher() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    
    void setBitDepth(float bits);
    
private:
    float bitDepth = 16.0f;
    float quantizationStep = 1.0f;
    
    void updateQuantizationStep();
};