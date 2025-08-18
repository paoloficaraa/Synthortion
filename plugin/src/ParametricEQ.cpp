#include "Synthortion/ParametricEQ.h"

//==============================================================================
// EQBand Implementation
//==============================================================================

void EQBand::prepare (const juce::dsp::ProcessSpec& spec)
{
    duplicatedFilter.prepare (spec);
    updateCoefficients (spec.sampleRate);
}

void EQBand::reset()
{
    duplicatedFilter.reset();
}

void EQBand::updateCoefficients (double sampleRate)
{
    if (!needsUpdate)
        return;

    juce::dsp::IIR::Coefficients<float>::Ptr coefficients;

    switch (filterType)
    {
        case LowCut:
            coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, frequency, quality);
            break;

        case LowShelf:
            coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, frequency, quality, juce::Decibels::decibelsToGain (gain));
            break;

        case Peak:
            coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, frequency, quality, juce::Decibels::decibelsToGain (gain));
            break;

        case HighShelf:
            coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, frequency, quality, juce::Decibels::decibelsToGain (gain));
            break;

        case HighCut:
            coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequency, quality);
            break;

        default:
            coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, frequency, quality, 1.0f);
            break;
    }

    *duplicatedFilter.state = *coefficients;
    needsUpdate = false;
}

//==============================================================================
// ParametricEQ Implementation
//==============================================================================

ParametricEQ::ParametricEQ()
{
    setDefaultBandConfiguration();
}

void ParametricEQ::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    for (auto& band : bands)
        band.prepare (spec);
}

void ParametricEQ::reset()
{
    for (auto& band : bands)
        band.reset();
}

void ParametricEQ::process (juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // Aggiorna i coefficienti se necessario
    for (auto& band : bands)
        band.updateCoefficients (currentSampleRate);

    // Processa ogni banda in sequenza
    for (auto& band : bands)
    {
        if (band.isEnabled())
            band.process (context);
    }
}

void ParametricEQ::setDefaultBandConfiguration()
{
    // Banda 1: Low Cut
    bands[0].setFilterType (EQBand::LowCut);
    bands[0].setFrequency (80.0f);
    bands[0].setQ (0.707f);
    bands[0].setGain (0.0f);

    // Banda 2: Low Mid
    bands[1].setFilterType (EQBand::Peak);
    bands[1].setFrequency (200.0f);
    bands[1].setQ (1.0f);
    bands[1].setGain (0.0f);

    // Banda 3: Mid
    bands[2].setFilterType (EQBand::Peak);
    bands[2].setFrequency (800.0f);
    bands[2].setQ (1.0f);
    bands[2].setGain (0.0f);

    // Banda 4: High Mid
    bands[3].setFilterType (EQBand::Peak);
    bands[3].setFrequency (3200.0f);
    bands[3].setQ (1.0f);
    bands[3].setGain (0.0f);

    // Banda 5: High
    bands[4].setFilterType (EQBand::Peak);
    bands[4].setFrequency (8000.0f);
    bands[4].setQ (1.0f);
    bands[4].setGain (0.0f);

    // Banda 6: High Cut
    bands[5].setFilterType (EQBand::HighCut);
    bands[5].setFrequency (12000.0f);
    bands[5].setQ (0.707f);
    bands[5].setGain (0.0f);
}