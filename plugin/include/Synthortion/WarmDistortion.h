#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class WarmDistortion
{
public:
    enum class SaturationType
    {
        SMOOTH,
        TUBE,
        TAPE
    };

    WarmDistortion();
    ~WarmDistortion() = default;

    void setSampleRate(double sampleRate);
    void reset();
    void prepare(const juce::dsp::ProcessSpec &spec);

    void setDrive(float drive); // 0.0 - 1.0
    void setMix(float mix);     // 0.0 - 1.0 (dry/wet)
    void setSaturationType(SaturationType type);

    void process(juce::AudioBuffer<float> &buffer);

private:
    float applySaturation(float input, float drive);
    float smoothSaturation(float input, float drive);
    float tubeSaturation(float input, float drive);
    float tapeSaturation(float input, float drive);
    float applyBitCrush(float input);

    void addDenormalizationNoise(float &sample);

    // Main parameters
    float driveAmount = 0.5f;
    float mixAmount = 1.0f;
    SaturationType saturationType = SaturationType::SMOOTH;

    double sampleRate = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    int oversamplingFactor = 3; // 2^3 = 8x oversampling

    juce::Random noiseGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};