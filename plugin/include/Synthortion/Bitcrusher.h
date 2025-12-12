#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <random>

class BitCrusher
{
public:
    BitCrusher();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    
    void setBitCrushMix(float mix);
    void setDACNoise(float noise);
    
private:
    float sampleRateReduction = 6000.0f;  // Rate: downsample to 6kHz
    float bitDepth = 8.0f;                // Bits: 8-bit quantization
    float ditherAmount = 0.4f;            // Dither: noise to reduce quantization
    float adcQuality = 0.95f;             // ADC Q: analog-to-digital quality (0-1)
    float dacNoiseAmount = 0.0f;          // DAC Q: digital-to-analog quality (0-1)
    
    float bitCrushMix = 0.0f;
    
    double sampleRate = 0.0;
    float quantizationStep = 1.0f;
    
    // For sample rate reduction
    float holdSampleLeft = 0.0f;
    float holdSampleRight = 0.0f;
    int sampleCounter = 0;
    int downsampleRatio = 1;
    
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    // Random generator for dither
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<float> distribution{-1.0f, 1.0f};
    
    void updateParameters();
    float applyBitReduction(float sample);
    float applyDither(float sample);
    float applyADCNoise(float sample);
    float applyDACNoise(float sample);
};