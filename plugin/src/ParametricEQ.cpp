#include "Synthortion/ParametricEQ.h"

//==============================================================================
// EQBand Implementation
//==============================================================================

void EQBand::prepare(const juce::dsp::ProcessSpec &spec)
{
    duplicatedFilter.prepare(spec);
    updateCoefficients(spec.sampleRate);
}

void EQBand::reset()
{
    duplicatedFilter.reset();
}

void EQBand::updateCoefficients(double sampleRate)
{
    if (!needsUpdate)
        return;

    juce::dsp::IIR::Coefficients<float>::Ptr coefficients;

    switch (filterType)
    {
    case LowCut:
        coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, frequency, quality);
        break;

    case LowShelf:
        coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, frequency, quality, juce::Decibels::decibelsToGain(gain));
        break;

    case Peak:
        coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, frequency, quality, juce::Decibels::decibelsToGain(gain));
        break;

    case HighShelf:
        coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, frequency, quality, juce::Decibels::decibelsToGain(gain));
        break;

    case HighCut:
        coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, frequency, quality);
        break;

    default:
        coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, frequency, quality, 1.0f);
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

void ParametricEQ::prepare(const juce::dsp::ProcessSpec &spec)
{
    currentSampleRate = spec.sampleRate;

    for (auto &band : bands)
        band.prepare(spec);
}

void ParametricEQ::reset()
{
    for (auto &band : bands)
        band.reset();
}

void ParametricEQ::process(juce::AudioBuffer<float> &buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Aggiorna i coefficienti se necessario
    for (auto &band : bands)
        band.updateCoefficients(currentSampleRate);

    // Processa ogni banda in sequenza
    for (auto &band : bands)
    {
        if (band.isEnabled())
            band.process(context);
    }
}

void ParametricEQ::setDefaultBandConfiguration()
{
    // Banda 1: Low Cut
    bands[0].setFilterType(EQBand::LowCut);
    bands[0].setFrequency(20.0f); // Corrisponde al minimo della UI
    bands[0].setQ(0.707f);
    bands[0].setGain(0.0f); // Non applicabile per LowCut

    // Banda 2: Low Mid
    bands[1].setFilterType(EQBand::Peak);
    bands[1].setFrequency(250.0f); // Corrisponde al minimo della UI
    bands[1].setQ(1.0f);
    bands[1].setGain(0.0f);

    // Banda 3: High Mid
    bands[2].setFilterType(EQBand::Peak);
    bands[2].setFrequency(2000.0f); // Corrisponde al minimo della UI
    bands[2].setQ(1.0f);
    bands[2].setGain(0.0f);

    // Banda 4: High Cut
    bands[3].setFilterType(EQBand::HighCut);
    bands[3].setFrequency(20000.0f); // Corrisponde al massimo della UI
    bands[3].setQ(0.707f);
    bands[3].setGain(0.0f); // Non applicabile per HighCut
}