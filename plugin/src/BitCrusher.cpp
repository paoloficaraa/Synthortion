#include "Synthortion/BitCrusher.h"

BitCrusher::BitCrusher()
{
    randomEngine.seed(std::random_device{}());
    updateParameters();
}

void BitCrusher::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    dryWetMixer.prepare(spec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);

    updateParameters();
    reset();
}

void BitCrusher::process(juce::AudioBuffer<float> &buffer)
{
    const int numSamples = buffer.getNumSamples();
    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float>(buffer));

    for (int i = 0; i < numSamples; ++i)
    {
        float leftSample = leftChannel[i];
        float rightSample = rightChannel[i];

        // 1. ADC Quality: add minimal noise to simulate low-quality analog-to-digital conversion
        leftSample = applyADCNoise(leftSample);
        rightSample = applyADCNoise(rightSample);

        // 2. Sample Rate Reduction: downsample (hold samples)
        if (sampleCounter % downsampleRatio == 0)
        {
            holdSampleLeft = leftSample;
            holdSampleRight = rightSample;
        }
        leftSample = holdSampleLeft;
        rightSample = holdSampleRight;
        sampleCounter++;

        // 3. Bit Depth Reduction with Dither
        leftSample = applyDither(leftSample);
        rightSample = applyDither(rightSample);
        leftSample = applyBitReduction(leftSample);
        rightSample = applyBitReduction(rightSample);

        // 4. DAC Noise: controllable noise per simulare conversione digitale-analogico
        leftSample = applyDACNoise(leftSample);
        rightSample = applyDACNoise(rightSample);

        leftChannel[i] = leftSample;
        rightChannel[i] = rightSample;
    }

    // Apply overall wet/dry mix
    dryWetMixer.setWetMixProportion(bitCrushMix);
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
}

void BitCrusher::reset()
{
    holdSampleLeft = 0.0f;
    holdSampleRight = 0.0f;
    sampleCounter = 0;
    dryWetMixer.reset();
}

void BitCrusher::setBitCrushMix(float mix)
{
    bitCrushMix = juce::jlimit(0.0f, 1.0f, mix);
}

void BitCrusher::setDACNoise(float noise)
{
    dacNoiseAmount = juce::jlimit(0.0f, 1.0f, noise);
}

void BitCrusher::updateParameters()
{
    // Calculate quantization step for bit depth
    float levels = std::pow(2.0f, bitDepth);
    quantizationStep = 2.0f / levels;

    // Calculate downsample ratio
    downsampleRatio = static_cast<int>(sampleRate / sampleRateReduction);
    downsampleRatio = juce::jmax(1, downsampleRatio);
}

float BitCrusher::applyBitReduction(float sample)
{
    // Quantize sample based on bit depth
    float quantized = std::floor(sample / quantizationStep) * quantizationStep;
    return juce::jlimit(-1.0f, 1.0f, quantized);
}

float BitCrusher::applyDither(float sample)
{
    // Add dithering noise before quantization to reduce harsh artifacts
    float ditherNoise = distribution(randomEngine) * ditherAmount * quantizationStep;
    return sample + ditherNoise;
}

float BitCrusher::applyADCNoise(float sample)
{
    float noiseAmount = (1.0f - adcQuality) * 0.005f;
    float noise = distribution(randomEngine) * noiseAmount;
    return sample + noise;
}

float BitCrusher::applyDACNoise(float sample)
{
    float noise = distribution(randomEngine) * dacNoiseAmount * 0.08f;
    return sample + noise;
}