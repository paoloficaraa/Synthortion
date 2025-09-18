#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void timerCallback() override;

    void pushNextSampleIntoFifo(float sample) noexcept;
    void drawNextFrameOfSpectrum();
    void drawFrame(juce::Graphics &g);

    // Configure analyzer with host sample rate for correct frequency axis
    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }

private:
    enum
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];
    float smoothedScopeData[scopeSize];           // For temporal smoothing
    juce::Array<juce::Point<float>> cachedPoints; // Cached points for performance
    double sampleRate = 44100.0;                  // used for frequency mapping
    static constexpr float minFreq = 20.0f;
    static constexpr float smoothingFactor = 0.8f; // Smoothing coefficient

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
