#include "Synthortion/ParametricEQ.h"
#include <algorithm>

ParametricEQ::ParametricEQ()
    : sampleRate(44100.0), lowCutFreq(100.0f), lowCutQ(0.707f), lowMidFreq(500.0f), lowMidQ(0.707f), lowMidGain(0.0f), highMidFreq(2000.0f), highMidQ(0.707f), highMidGain(0.0f), highCutFreq(20000.0f), highCutQ(0.707f), isPrepared(false)
{
}

ParametricEQ::~ParametricEQ()
{
}

void ParametricEQ::prepare(const juce::dsp::ProcessSpec &spec)
{
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

    {
        // Use IIR filters for minimum phase (lower latency)
        if (lowCutEnabled && lowCutFreq > 20.0f)
        {
            lowCutFilter.process(context);  // First stage: 6dB/octave
            lowCutFilter2.process(context); // Second stage: additional 6dB/octave = total 12dB/octave
        }

        if (std::abs(lowMidGain) > 0.001f)
            lowMidFilter.process(context);

        if (std::abs(highMidGain) > 0.001f)
            highMidFilter.process(context);

        if (highCutEnabled && highCutFreq > 20.0f)
        {
            highCutFilter.process(context);  // First stage: 6dB/octave
            highCutFilter2.process(context); // Second stage: additional 6dB/octave = total 12dB/octave
        }
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
    float validLowCutFreq = juce::jlimit(20.0f, maxFreq, lowCutFreq);
    float validLowMidFreq = juce::jlimit(20.0f, maxFreq, lowMidFreq);
    float validHighMidFreq = juce::jlimit(20.0f, maxFreq, highMidFreq);
    float validHighCutFreq = juce::jlimit(5000.0f, juce::jmin(maxFreq, 20000.0f), highCutFreq);

    // Validate Q values - ensure they're positive
    float validLowCutQ = juce::jmax(0.1f, lowCutQ);
    float validLowMidQ = juce::jmax(0.1f, lowMidQ);
    float validHighMidQ = juce::jmax(0.1f, highMidQ);
    float validHighCutQ = juce::jmax(0.1f, highCutQ);

    // Low Cut - High Pass Filter (12dB/octave using cascade of two 6dB filters)
    // For proper Butterworth response at Q=1.0, use 0.707. For other values, allow the curve to change.
    float correctedLowCutQ = (std::abs(validLowCutQ - 1.0f) < 0.05f) ? 0.707f : validLowCutQ;

    // First stage: 6dB/octave high-pass
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, validLowCutFreq, correctedLowCutQ);
    // Second stage: another 6dB/octave high-pass for total 12dB/octave
    *lowCutFilter2.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, validLowCutFreq, correctedLowCutQ);

    // Low Mid - Peaking Filter
    *lowMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, validLowMidFreq, validLowMidQ, juce::Decibels::decibelsToGain(lowMidGain));

    // High Mid - Peaking Filter
    *highMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, validHighMidFreq, validHighMidQ, juce::Decibels::decibelsToGain(highMidGain));

    // High Cut - Low Pass Filter (12dB/octave using cascade of two 6dB filters)
    // For proper Butterworth response at Q=1.0, use 0.707. For other values, allow the curve to change.
    float correctedHighCutQ = (std::abs(validHighCutQ - 1.0f) < 0.05f) ? 0.707f : validHighCutQ;

    // First stage: 6dB/octave low-pass
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, validHighCutFreq, correctedHighCutQ);
    // Second stage: another 6dB/octave low-pass for total 12dB/octave
    *highCutFilter2.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, validHighCutFreq, correctedHighCutQ);
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
            // Calculate response for active filters with cascade for cut filters (12dB/octave)
            if (lowCutEnabled && lowCutFreq > 20.0f)
            {
                magnitude *= static_cast<float>(std::abs(lowCutFilter.state->getMagnitudeForFrequency(freq, sampleRate)));
                magnitude *= static_cast<float>(std::abs(lowCutFilter2.state->getMagnitudeForFrequency(freq, sampleRate)));
            }

            if (std::abs(lowMidGain) > 0.001f)
                magnitude *= static_cast<float>(std::abs(lowMidFilter.state->getMagnitudeForFrequency(freq, sampleRate)));

            if (std::abs(highMidGain) > 0.001f)
                magnitude *= static_cast<float>(std::abs(highMidFilter.state->getMagnitudeForFrequency(freq, sampleRate)));

            if (highCutEnabled && highCutFreq > 20.0f)
            {
                magnitude *= static_cast<float>(std::abs(highCutFilter.state->getMagnitudeForFrequency(freq, sampleRate)));
                magnitude *= static_cast<float>(std::abs(highCutFilter2.state->getMagnitudeForFrequency(freq, sampleRate)));
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