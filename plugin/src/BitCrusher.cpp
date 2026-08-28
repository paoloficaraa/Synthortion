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
    if (targetCrush <= 0.0f && smoothedCrush.getCurrentValue() <= 0.0f && !smoothedCrush.isSmoothing())
        return;

    const bool isSmoothing = smoothedCrush.isSmoothing();
    const double fs = (sampleRate > 0.0) ? sampleRate : 44100.0;
    const float fsFloat = static_cast<float>(fs);

    float staticPhaseStep = 1.0f;
    float staticDeltaQ = 0.0f;
    if (!isSmoothing)
    {
        const float c = smoothedCrush.getCurrentValue();
        if (c <= 1.0e-5f)
        {
            smoothedCrush.skip(numSamples);
            return;
        }

        const float targetRate = calculateTargetSampleRate(c, fs);
        staticPhaseStep = juce::jlimit(0.0f, 1.0f, targetRate / fsFloat);
        const float bitDepth = calculateBitDepth(c);
        staticDeltaQ = calculateQuantizationStep(bitDepth);
    }

    for (int s = 0; s < numSamples; ++s)
    {
        const float c = isSmoothing ? smoothedCrush.getNextValue() : smoothedCrush.getCurrentValue();
        if (c <= 1.0e-5f)
            continue;

        const float phaseStep = isSmoothing ? juce::jlimit(0.0f, 1.0f, calculateTargetSampleRate(c, fs) / fsFloat) : staticPhaseStep;
        const float deltaQ = isSmoothing ? calculateQuantizationStep(calculateBitDepth(c)) : staticDeltaQ;

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
                phase -= 1.0f;
                if (phase >= 1.0f)
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
            float* channelData = buffer.getWritePointer(ch);

            // Fractional Sample & Hold with linear interpolation
            const float interpolated = previousSample[safeCh] + phase * (currentSample[safeCh] - previousSample[safeCh]);

            // Triangular PDF dynamic dither: (r1 - r2) * deltaQ
            const float r1 = randomGenerators[safeCh].nextFloat();
            const float r2 = randomGenerators[safeCh].nextFloat();
            const float dither = (r1 - r2) * deltaQ;

            // Quantization
            const float dithered = interpolated + dither;
            const float quantized = std::round(dithered / deltaQ) * deltaQ;

            channelData[s] = juce::jlimit(-1.0f, 1.0f, quantized);
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
