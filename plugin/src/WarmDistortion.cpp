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
            channelData[sample] = applySaturation(inputSample, driveAmount);
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
    return tanhSaturation(input, drive);
}

float WarmDistortion::tanhSaturation(float input, float drive)
{
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 20.0f);
    float driven = input * driveMapped;
    return std::tanh(driven);
}

void WarmDistortion::addDenormalizationNoise(float &sample)
{
    if (std::abs(sample) < 1.0e-15f)
    {
        sample += noiseGenerator.nextFloat() * 1.0e-30f;
    }
}