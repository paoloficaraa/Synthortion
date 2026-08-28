#include "Synthortion/Bitcrusher.h"

namespace synthortion::dsp {

BitCrusher::BitCrusher()
{
    reset();
}

void BitCrusher::prepare(const juce::dsp::ProcessSpec& spec)
{
    jassert(spec.sampleRate > 0.0);
    jassert(spec.maximumBlockSize > 0);
    jassert(spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    smoothedCrush.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedCrush.setCurrentAndTargetValue(0.0f);

    reset();
}

void BitCrusher::process(juce::AudioBuffer<float>& buffer, const BitCrusherParams& params)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0)
        return;

    const float targetCrush = juce::jlimit(0.0f, 1.0f, params.crush);
    smoothedCrush.setTargetValue(targetCrush);

    // 100% transparent identity output at c = 0
    if (targetCrush <= 1.0e-5f && smoothedCrush.getCurrentValue() <= 1.0e-5f && !smoothedCrush.isSmoothing())
        return;

    const bool isSmoothing = smoothedCrush.isSmoothing();
    const double fs = (sampleRate > 0.0) ? sampleRate : 44100.0;
    const float fsFloat = static_cast<float>(fs);

    for (int s = 0; s < numSamples; ++s)
    {
        const float c = isSmoothing ? smoothedCrush.getNextValue() : smoothedCrush.getCurrentValue();
        if (c <= 1.0e-5f)
            continue;

        const float targetRate = calculateTargetSampleRate(c, fs);
        const float phaseStep = juce::jlimit(0.0001f, 1.0f, targetRate / fsFloat);
        const float bitDepth = calculateBitDepth(c);
        const float deltaQ = calculateQuantizationStep(bitDepth);

        if (isFirstSample)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const size_t safeCh = static_cast<size_t>(ch < kNumChannels ? ch : 0);
                const float inSample = buffer.getSample(ch, s);
                previousSample[safeCh] = inSample;
                currentSample[safeCh] = inSample;
            }
            phase = 0.0f;
            isFirstSample = false;
        }
        else
        {
            phase += phaseStep;
            if (phase >= 1.0f)
            {
                phase = std::fmod(phase, 1.0f);
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const size_t safeCh = static_cast<size_t>(ch < kNumChannels ? ch : 0);
                    previousSample[safeCh] = currentSample[safeCh];
                    currentSample[safeCh] = buffer.getSample(ch, s);
                }
            }
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const size_t safeCh = static_cast<size_t>(ch < kNumChannels ? ch : 0);
            const float inSample = buffer.getSample(ch, s);
            float* channelData = buffer.getWritePointer(ch);

            // Fractional sample & hold: at phaseStep ~1.0 (no downsample), use original inSample directly
            const float downsampled = (phaseStep >= 0.999f)
                ? inSample
                : (previousSample[safeCh] + phase * (currentSample[safeCh] - previousSample[safeCh]));

            // Quantization with gentle scaled TPDF dither only when bit reduction is active
            float processed = downsampled;
            if (bitDepth < 15.99f && deltaQ > 0.0f)
            {
                const float r1 = randomGenerators[safeCh].nextFloat();
                const float r2 = randomGenerators[safeCh].nextFloat();
                const float dither = (r1 - r2) * deltaQ * 0.5f * c;
                const float dithered = downsampled + dither;
                processed = std::round(dithered / deltaQ) * deltaQ;
            }

            // Smoothly blend dry input and processed signal proportional to crush
            const float wet = juce::jlimit(-1.0f, 1.0f, processed);
            channelData[s] = (1.0f - c) * inSample + c * wet;
        }
    }

    if (!isSmoothing)
        smoothedCrush.skip(numSamples);
}

void BitCrusher::reset() noexcept
{
    phase = 0.0f;
    isFirstSample = true;
    for (size_t ch = 0; ch < kNumChannels; ++ch)
    {
        previousSample[ch] = 0.0f;
        currentSample[ch] = 0.0f;
    }
    smoothedCrush.setCurrentAndTargetValue(0.0f);
}

} // namespace synthortion::dsp
