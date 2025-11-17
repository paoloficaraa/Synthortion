#include "Synthortion/BitCrusher.h"

void BitCrusher::prepare(const juce::dsp::ProcessSpec&)
{
    updateQuantizationStep();
}

void BitCrusher::process(juce::AudioBuffer<float>& buffer)
{
    if (bitDepth >= 15.9f)
        return;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Quantize
            float input = channelData[sample];
            float quantized = std::floor(input / quantizationStep + 0.5f) * quantizationStep;
            channelData[sample] = juce::jlimit(-1.0f, 1.0f, quantized);
        }
    }
}

void BitCrusher::reset()
{
}

void BitCrusher::setBitDepth(float bits)
{
    bitDepth = juce::jlimit(1.0f, 16.0f, bits);
    updateQuantizationStep();
}

void BitCrusher::updateQuantizationStep()
{
    int levels = static_cast<int>(std::pow(2.0f, bitDepth));
    quantizationStep = 2.0f / static_cast<float>(levels);
}