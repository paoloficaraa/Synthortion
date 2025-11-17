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
    
private:
    juce::dsp::Chorus<float> chorus;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    float chorusMix = 0.0f;
};