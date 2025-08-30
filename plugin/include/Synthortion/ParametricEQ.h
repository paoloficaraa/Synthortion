#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class EQBand
{
public:
    enum FilterType
    {
        LowCut = 0,
        LowShelf,
        Peak,
        HighShelf,
        HighCut,
        NumFilterTypes
    };

    EQBand() = default;

    void prepare(const juce::dsp::ProcessSpec &spec);
    void reset();
    void updateCoefficients(double sampleRate);

    template <typename ProcessContext>
    void process(const ProcessContext &context)
    {
        // Usa ProcessorDuplicator per gestire automaticamente i canali multipli
        duplicatedFilter.process(context);
    }

    // Setters per i parametri
    void setFrequency(float frequency)
    {
        this->frequency = frequency;
        needsUpdate = true;
    }
    void setGain(float gainDb)
    {
        this->gain = gainDb;
        needsUpdate = true;
    }
    void setQ(float q)
    {
        this->quality = q;
        needsUpdate = true;
    }
    void setFilterType(FilterType type)
    {
        this->filterType = type;
        needsUpdate = true;
    }
    void setEnabled(bool enabled) { this->enabled = enabled; }

    // Getters
    float getFrequency() const { return frequency; }
    float getGain() const { return gain; }
    float getQ() const { return quality; }
    FilterType getFilterType() const { return filterType; }
    bool isEnabled() const { return enabled; }

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> duplicatedFilter;

    float frequency = 1000.0f;
    float gain = 0.0f;
    float quality = 0.707f;
    FilterType filterType = Peak;
    bool enabled = true;
    bool needsUpdate = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQBand)
};

//==============================================================================
/**
 * Equalizzatore parametrico multi-banda
 */
class ParametricEQ
{
public:
    static constexpr int NumBands = 4;

    ParametricEQ();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void reset();
    void process(juce::AudioBuffer<float> &buffer);

    // Accesso alle bande
    EQBand &getBand(int bandIndex)
    {
        jassert(bandIndex >= 0 && bandIndex < NumBands);
        return bands[bandIndex];
    }

    const EQBand &getBand(int bandIndex) const
    {
        jassert(bandIndex >= 0 && bandIndex < NumBands);
        return bands[bandIndex];
    }

    // Configurazione predefinita delle bande
    void setDefaultBandConfiguration();

    // Analisi per il display dello spettrogramma (opzionale)
    void setAnalysisEnabled(bool enabled) { analysisEnabled = enabled; }
    bool isAnalysisEnabled() const { return analysisEnabled; }

private:
    std::array<EQBand, NumBands> bands;
    double currentSampleRate = 44100.0;
    bool analysisEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};