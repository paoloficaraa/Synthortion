#include "Synthortion/BitCrusher.h"

BitCrusher::BitCrusher()
{
    updateParameters();
}

void BitCrusher::prepare(const juce::dsp::ProcessSpec &spec)
{
    jassert(spec.sampleRate > 0.0);
    jassert(spec.maximumBlockSize > 0);
    jassert(spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    dryWetMixer.prepare(spec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);

    updateParameters();
    reset();
}

void BitCrusher::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float>(buffer));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        float& holdSample = (ch == 0) ? holdSampleLeft : holdSampleRight;
        int& holdCounter = (ch == 0) ? holdCounterLeft : holdCounterRight;

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = channelData[i];

            // Sample & Hold (downsampling)
            if (holdCounter == 0)
            {
                holdSample = sample;
                holdCounter = downsampleRatio;
            }
            sample = holdSample;
            holdCounter--;

            // Dithering + Quantization (bit depth reduction)
            sample += randomGenerator.nextFloat() * cachedDitherScale;
            sample = std::floor(sample / quantizationStep) * quantizationStep;
            sample = juce::jlimit(-1.0f, 1.0f, sample);

            channelData[i] = sample;
        }
    }

    dryWetMixer.setWetMixProportion(bitCrushMix);
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
}

void BitCrusher::reset()
{
    holdSampleLeft = 0.0f;
    holdSampleRight = 0.0f;
    holdCounterLeft = 0;
    holdCounterRight = 0;
    dryWetMixer.reset();
}

void BitCrusher::setBitCrushMix(float mix)
{
    bitCrushMix = juce::jlimit(0.0f, 1.0f, mix);
}

void BitCrusher::updateParameters()
{
    float levels = std::pow(2.0f, bitDepth);
    quantizationStep = 2.0f / levels;

    downsampleRatio = static_cast<int>(sampleRate / sampleRateReduction);
    downsampleRatio = juce::jmax(1, downsampleRatio);

    cachedDitherScale = ditherAmount * quantizationStep * 2.0f;
}