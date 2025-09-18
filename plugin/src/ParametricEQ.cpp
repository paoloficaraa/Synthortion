#include "Synthortion/ParametricEQ.h"
#include <algorithm>

ParametricEQ::ParametricEQ()
    : sampleRate(44100.0), lowCutFreq(100.0f), lowCutQ(0.707f), lowMidFreq(500.0f), lowMidQ(0.707f), lowMidGain(0.0f), highMidFreq(2000.0f), highMidQ(0.707f), highMidGain(0.0f), highCutFreq(8000.0f), highCutQ(0.707f), isPrepared(false), linearPhaseEnabled(false), firNeedsUpdate(true)
{
    firBuffer.setSize(1, firLength);
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

    // Prepare convolution for linear phase mode
    convolution.prepare(spec);

    // Ensure firBuffer has correct number of channels
    firBuffer.setSize(static_cast<int>(spec.numChannels), firLength, false, true, true);

    updateFilters();
    isPrepared = true;
}

void ParametricEQ::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
    if (!isPrepared)
        return;

    if (linearPhaseEnabled)
    {
        // Use FIR convolution for linear phase
        if (firNeedsUpdate)
        {
            generateFIRResponse();
            firNeedsUpdate = false;
        }
        convolution.process(context);
    }
    else
    {
        // Use IIR filters for minimum phase (lower latency)
        if (lowCutEnabled && lowCutFreq > 20.0f)
            lowCutFilter.process(context);

        if (std::abs(lowMidGain) > 0.001f)
            lowMidFilter.process(context);

        if (std::abs(highMidGain) > 0.001f)
            highMidFilter.process(context);

        if (highCutEnabled && highCutFreq > 20.0f)
            highCutFilter.process(context);
    }
}

void ParametricEQ::setLowCut(float frequency, float q, bool enabled)
{
    lowCutFreq = frequency;
    lowCutQ = q;
    lowCutEnabled = enabled;
    updateFilters();
    firNeedsUpdate = true;
}

void ParametricEQ::setLowMid(float frequency, float gain, float q)
{
    lowMidFreq = frequency;
    lowMidGain = gain;
    lowMidQ = q;
    updateFilters();
    firNeedsUpdate = true;
}

void ParametricEQ::setHighMid(float frequency, float gain, float q)
{
    highMidFreq = frequency;
    highMidGain = gain;
    highMidQ = q;
    updateFilters();
    firNeedsUpdate = true;
}

void ParametricEQ::setHighCut(float frequency, float q, bool enabled)
{
    highCutFreq = frequency;
    highCutQ = q;
    highCutEnabled = enabled;
    updateFilters();
    firNeedsUpdate = true;
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
    float validHighCutFreq = juce::jlimit(20.0f, maxFreq, highCutFreq);

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

void ParametricEQ::setLinearPhase(bool enabled)
{
    if (linearPhaseEnabled != enabled)
    {
        linearPhaseEnabled = enabled;
        firNeedsUpdate = true;

        if (enabled && isPrepared)
        {
            generateFIRResponse();
        }
    }
}

int ParametricEQ::getLatencySamples() const
{
    return linearPhaseEnabled ? firLength / 2 : 0;
}

void ParametricEQ::generateFIRResponse()
{
    if (!isPrepared || !linearPhaseEnabled)
        return;

    // Clear all channels of the FIR buffer
    for (int channel = 0; channel < firBuffer.getNumChannels(); ++channel)
    {
        juce::FloatVectorOperations::clear(firBuffer.getWritePointer(channel), firLength);
        // Generate impulse response by creating an impulse and processing it through IIR filters
        firBuffer.setSample(channel, firLength / 2, 1.0f); // Delta function at center
    }

    // Create temporary audio block for processing
    juce::dsp::AudioBlock<float> block(firBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Apply each filter in sequence to generate the combined impulse response
    if (lowCutEnabled && lowCutFreq > 20.0f)
        lowCutFilter.process(context);

    if (std::abs(lowMidGain) > 0.001f)
        lowMidFilter.process(context);

    if (std::abs(highMidGain) > 0.001f)
        highMidFilter.process(context);

    if (highCutEnabled && highCutFreq > 20.0f)
        highCutFilter.process(context);

    // Apply windowing to reduce artifacts using optimized operations
    const float windowSize = static_cast<float>(firLength);

    // Pre-calculate Hann window for all samples at once
    std::vector<float> windowData(firLength);
    for (int i = 0; i < firLength; ++i)
    {
        windowData[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (windowSize - 1.0f)));
    }

    // Apply window to all channels
    for (int channel = 0; channel < firBuffer.getNumChannels(); ++channel)
    {
        auto *firData = firBuffer.getWritePointer(channel);
        juce::FloatVectorOperations::multiply(firData, windowData.data(), firLength);
    }

    // Load the FIR response into the convolution
    convolution.loadImpulseResponse(std::move(firBuffer), sampleRate,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::no,
                                    juce::dsp::Convolution::Normalise::no);
}