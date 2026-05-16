#include "Synthortion/ParametricEQ.h"

ParametricEQ::ParametricEQ()
{
    updateActiveStates();
}

void ParametricEQ::prepare(const juce::dsp::ProcessSpec &spec)
{
    jassert(spec.sampleRate > 0.0);
    jassert(spec.maximumBlockSize > 0);
    jassert(spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    lowCutFilter.prepare(spec);
    lowCutFilter2.prepare(spec);
    lowMidFilter.prepare(spec);
    highMidFilter.prepare(spec);
    highCutFilter.prepare(spec);
    highCutFilter2.prepare(spec);

    updateFilters();
    isPrepared = true;
}

void ParametricEQ::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
    if (!isPrepared)
        return;

    if (lowCutEnabled)
    {
        lowCutFilter.process(context);
        lowCutFilter2.process(context);
    }

    if (lowMidActive)
        lowMidFilter.process(context);

    if (highMidActive)
        highMidFilter.process(context);

    if (highCutEnabled)
    {
        highCutFilter.process(context);
        highCutFilter2.process(context);
    }
}

void ParametricEQ::setLowCut(float frequency, float q, bool enabled)
{
    lowCutFreq = frequency;
    lowCutQ = q;
    lowCutEnabled = enabled;
    updateFilters();
}

void ParametricEQ::setLowMid(float frequency, float gain, float q)
{
    lowMidFreq = frequency;
    lowMidGain = gain;
    lowMidQ = q;
    updateActiveStates();
    updateFilters();
}

void ParametricEQ::setHighMid(float frequency, float gain, float q)
{
    highMidFreq = frequency;
    highMidGain = gain;
    highMidQ = q;
    updateActiveStates();
    updateFilters();
}

void ParametricEQ::setHighCut(float frequency, float q, bool enabled)
{
    highCutFreq = frequency;
    highCutQ = q;
    highCutEnabled = enabled;
    updateFilters();
}

void ParametricEQ::updateActiveStates()
{
    lowMidActive = std::abs(lowMidGain) > kGainThreshold;
    highMidActive = std::abs(highMidGain) > kGainThreshold;
}

void ParametricEQ::updateFilters()
{
    if (!isPrepared || sampleRate <= 0.0)
        return;

    const float maxFreq = static_cast<float>(sampleRate * kNyquistFactor);

    const float validLowCutFreq = juce::jlimit(kMinFrequency, maxFreq, lowCutFreq);
    const float validLowMidFreq = juce::jlimit(kMinFrequency, maxFreq, lowMidFreq);
    const float validHighMidFreq = juce::jlimit(kMinFrequency, maxFreq, highMidFreq);
    const float validHighCutFreq = juce::jlimit(kHighCutMinFreq, juce::jmin(maxFreq, kMaxFrequency), highCutFreq);

    const float validLowCutQ = juce::jmax(kMinQ, lowCutQ);
    const float validLowMidQ = juce::jmax(kMinQ, lowMidQ);
    const float validHighMidQ = juce::jmax(kMinQ, highMidQ);
    const float validHighCutQ = juce::jmax(kMinQ, highCutQ);

    const float correctedLowCutQ = (std::abs(validLowCutQ - 1.0f) < kQTolerance)
                                    ? kButterworthQ : validLowCutQ;
    const float correctedHighCutQ = (std::abs(validHighCutQ - 1.0f) < kQTolerance)
                                     ? kButterworthQ : validHighCutQ;

    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, validLowCutFreq, correctedLowCutQ);
    *lowCutFilter2.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, validLowCutFreq, correctedLowCutQ);

    *lowMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, validLowMidFreq, validLowMidQ,
        juce::Decibels::decibelsToGain(lowMidGain));

    *highMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, validHighMidFreq, validHighMidQ,
        juce::Decibels::decibelsToGain(highMidGain));

    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate, validHighCutFreq, correctedHighCutQ);
    *highCutFilter2.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate, validHighCutFreq, correctedHighCutQ);
}

std::vector<float> ParametricEQ::getFrequencyResponse(const std::vector<float> &frequencies) const
{
    std::vector<float> response;
    response.reserve(frequencies.size());

    for (float freq : frequencies)
    {
        float magnitude = 1.0f;

        if (isPrepared)
        {
            if (lowCutEnabled)
            {
                magnitude *= static_cast<float>(lowCutFilter.state->getMagnitudeForFrequency(freq, sampleRate));
                magnitude *= static_cast<float>(lowCutFilter2.state->getMagnitudeForFrequency(freq, sampleRate));
            }

            if (lowMidActive)
                magnitude *= static_cast<float>(lowMidFilter.state->getMagnitudeForFrequency(freq, sampleRate));

            if (highMidActive)
                magnitude *= static_cast<float>(highMidFilter.state->getMagnitudeForFrequency(freq, sampleRate));

            if (highCutEnabled)
            {
                magnitude *= static_cast<float>(highCutFilter.state->getMagnitudeForFrequency(freq, sampleRate));
                magnitude *= static_cast<float>(highCutFilter2.state->getMagnitudeForFrequency(freq, sampleRate));
            }
        }

        response.push_back(juce::Decibels::gainToDecibels(magnitude));
    }

    return response;
}

int ParametricEQ::getLatencySamples() const
{
    return 0;
}