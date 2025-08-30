#include "Synthortion/WarmDistortion.h"

WarmDistortion::WarmDistortion()
{
    reset();
}

void WarmDistortion::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    if (oversampler)
    {
        oversampler->reset();
    }
}

void WarmDistortion::reset()
{
    noiseGenerator.setSeedRandomly();
}

void WarmDistortion::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        oversamplingFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    oversampler->initProcessing(spec.maximumBlockSize);
}

void WarmDistortion::setDrive(float newDrive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, newDrive);
}

void WarmDistortion::setMix(float newMix)
{
    mixAmount = juce::jlimit(0.0f, 1.0f, newMix);
}

void WarmDistortion::setSaturationType(SaturationType type)
{
    saturationType = type;
}

void WarmDistortion::process(juce::AudioBuffer<float> &buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0 || !oversampler)
        return;

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        float *channelData = oversampledBlock.getChannelPointer(channel);
        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            addDenormalizationNoise(inputSample);

            // Apply saturation
            float saturatedSample = applySaturation(inputSample, driveAmount);

            // Apply bit crush effect
            channelData[sample] = applyBitCrush(saturatedSample);
        }
    }

    oversampler->processSamplesDown(block);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float *channelData = buffer.getWritePointer(channel);
        const float *dryData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = (channelData[sample] * mixAmount) + (dryData[sample] * (1.0f - mixAmount));
        }
    }
}

float WarmDistortion::applySaturation(float input, float drive)
{
    switch (saturationType)
    {
    case SaturationType::SMOOTH:
        return smoothSaturation(input, drive);
    case SaturationType::TUBE:
        return tubeSaturation(input, drive);
    case SaturationType::TAPE:
        return tapeSaturation(input, drive);
    default:
        return smoothSaturation(input, drive);
    }
}

float WarmDistortion::smoothSaturation(float input, float drive)
{
    // Smooth tanh saturation (original behavior)
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 20.0f);
    float driven = input * driveMapped;
    return std::tanh(driven);
}

float WarmDistortion::tubeSaturation(float input, float drive)
{
    // Tube-style saturation with asymmetric clipping
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 15.0f);
    float driven = input * driveMapped;

    // Asymmetric saturation mimicking tube behavior
    if (driven > 0.0f)
    {
        // Softer saturation for positive signals
        return std::tanh(driven * 0.7f) * 1.2f;
    }
    else
    {
        // Harder clipping for negative signals
        return juce::jmax(-0.9f, driven * 0.9f);
    }
}

float WarmDistortion::tapeSaturation(float input, float drive)
{
    // Tape-style saturation with soft knee compression
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 8.0f);
    float driven = input * driveMapped;

    // Soft knee saturation characteristic of tape
    float abs_driven = std::abs(driven);
    if (abs_driven < 0.5f)
    {
        return driven;
    }
    else
    {
        float sign = (driven > 0.0f) ? 1.0f : -1.0f;
        float compressed = 0.5f + (abs_driven - 0.5f) / (1.0f + (abs_driven - 0.5f) * 2.0f);
        return sign * compressed;
    }
}

float WarmDistortion::applyBitCrush(float input)
{
    // Applica un leggero bit-crush fisso (14-bit con mix al 15%)
    const float fixedBits = 14.0f;
    const float fixedMix = 0.15f;

    // Calculate quantization levels
    float levels = std::pow(2.0f, fixedBits) - 1.0f;

    // Quantize the signal
    float crushed = std::round(input * levels) / levels;

    // Mix with original signal
    return input * (1.0f - fixedMix) + crushed * fixedMix;
}
void WarmDistortion::addDenormalizationNoise(float &sample)
{
    if (std::abs(sample) < 1.0e-15f)
    {
        sample += noiseGenerator.nextFloat() * 1.0e-30f;
    }
}