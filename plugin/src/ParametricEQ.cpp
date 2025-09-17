#include "Synthortion/ParametricEQ.h"
#include <algorithm>

ParametricEQ::ParametricEQ()
    : sampleRate(44100.0), lowCutFreq(100.0f), lowCutQ(0.707f), lowMidFreq(500.0f), lowMidQ(0.707f), lowMidGain(0.0f), highMidFreq(2000.0f), highMidQ(0.707f), highMidGain(0.0f), highCutFreq(8000.0f), highCutQ(0.707f), isPrepared(false)
{
}

ParametricEQ::~ParametricEQ()
{
}

void ParametricEQ::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    lowCutFilter.prepare(spec);
    lowMidFilter.prepare(spec);
    highMidFilter.prepare(spec);
    highCutFilter.prepare(spec);

    updateFilters();
    isPrepared = true;
}

void ParametricEQ::process(juce::AudioBuffer<float> &buffer)
{
    if (!isPrepared)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    if (lowCutEnabled && lowCutFreq > 20.0f)
        lowCutFilter.process(context);

    if (std::abs(lowMidGain) > 0.001f)
        lowMidFilter.process(context);

    if (std::abs(highMidGain) > 0.001f)
        highMidFilter.process(context);

    if (highCutEnabled && highCutFreq > 20.0f)
        highCutFilter.process(context);
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
    updateFilters();
}

void ParametricEQ::setHighMid(float frequency, float gain, float q)
{
    highMidFreq = frequency;
    highMidGain = gain;
    highMidQ = q;
    updateFilters();
}

void ParametricEQ::setHighCut(float frequency, float q, bool enabled)
{
    highCutFreq = frequency;
    highCutQ = q;
    highCutEnabled = enabled;
    updateFilters();
}

void ParametricEQ::updateFilters()
{
    if (!isPrepared || sampleRate <= 0.0)
        return;

    // Validate frequencies - ensure they're within valid range
    float maxFreq = static_cast<float>(sampleRate * 0.45);
    float validLowCutFreq = std::max(20.0f, std::min(maxFreq, lowCutFreq));
    float validLowMidFreq = std::max(20.0f, std::min(maxFreq, lowMidFreq));
    float validHighMidFreq = std::max(20.0f, std::min(maxFreq, highMidFreq));
    float validHighCutFreq = std::max(20.0f, std::min(maxFreq, highCutFreq));

    // Validate Q values - ensure they're positive
    float validLowCutQ = juce::jmax(0.1f, lowCutQ);
    float validLowMidQ = juce::jmax(0.1f, lowMidQ);
    float validHighMidQ = juce::jmax(0.1f, highMidQ);
    float validHighCutQ = juce::jmax(0.1f, highCutQ);

    // Low Cut - High Pass Filter
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, validLowCutFreq, validLowCutQ);

    // Low Mid - Peaking Filter
    *lowMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, validLowMidFreq, validLowMidQ, juce::Decibels::decibelsToGain(lowMidGain));

    // High Mid - Peaking Filter
    *highMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, validHighMidFreq, validHighMidQ, juce::Decibels::decibelsToGain(highMidGain));

    // High Cut - Low Pass Filter
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, validHighCutFreq, validHighCutQ);
}

std::vector<float> ParametricEQ::getFrequencyResponse(const std::vector<float> &frequencies)
{
    std::vector<float> response;
    response.reserve(frequencies.size());

    for (auto freq : frequencies)
    {
        auto magnitude = 1.0f;

        if (isPrepared)
        {
            // Calcola risposta solo per filtri attivi
            if (lowCutEnabled && lowCutFreq > 20.0f)
                magnitude *= static_cast<float>(std::abs(lowCutFilter.state->getMagnitudeForFrequency(freq, sampleRate)));

            if (std::abs(lowMidGain) > 0.001f)
                magnitude *= static_cast<float>(std::abs(lowMidFilter.state->getMagnitudeForFrequency(freq, sampleRate)));

            if (std::abs(highMidGain) > 0.001f)
                magnitude *= static_cast<float>(std::abs(highMidFilter.state->getMagnitudeForFrequency(freq, sampleRate)));

            if (highCutEnabled && highCutFreq > 20.0f)
                magnitude *= static_cast<float>(std::abs(highCutFilter.state->getMagnitudeForFrequency(freq, sampleRate)));
        }

        response.push_back(juce::Decibels::gainToDecibels(magnitude));
    }

    return response;
}
