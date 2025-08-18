#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class WarmDistortion
{
public:
    WarmDistortion();
    ~WarmDistortion() = default;

    void setSampleRate(double sampleRate);
    void reset();
    void prepare(const juce::dsp::ProcessSpec &spec);

    void setDrive(float drive); // 0.0 - 1.0
    void setMix(float mix);     // 0.0 - 1.0 (dry/wet)

    void process(juce::AudioBuffer<float> &buffer);

private:
    float applySaturation(float input, float drive);
    float tanhSaturation(float input, float drive);

    void addDenormalizationNoise(float &sample);

    float driveAmount = 0.5f;
    float mixAmount = 1.0f;

    double sampleRate = 44100.0;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    int oversamplingFactor = 3; // 2^3 = 8x oversampling

    juce::Random noiseGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};