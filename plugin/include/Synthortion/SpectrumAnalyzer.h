#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ParametricEQ;

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

    // EQ curve visualization
    void setEQReference(ParametricEQ *eq) { eqReference = eq; }
    void drawEQCurve(juce::Graphics &g);
    void setEQBypass(bool bypassed) { eqBypassed = bypassed; }

private:
    enum
    {
        fftOrder = 12,           // Increased from 11 for better frequency resolution
        fftSize = 1 << fftOrder, // 4096 samples for higher resolution
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
    float targetScopeData[scopeSize];             // Target values for interpolation
    float currentScopeData[scopeSize];            // Current interpolated values
    float peakHoldData[scopeSize];                // Peak hold values like professional analyzers
    int peakHoldTimer[scopeSize];                 // Timer for peak hold decay
    juce::Array<juce::Point<float>> cachedPoints; // Cached points for performance
    double sampleRate = 0.0;                      // used for frequency mapping (set by host)

    // EQ curve visualization
    ParametricEQ *eqReference = nullptr;
    bool eqBypassed = false;
    std::vector<juce::Point<float>> eqCurvePoints;

    static constexpr float minFreq = 20.0f;
    static constexpr float smoothingFactor = 0.85f;    // Smoothing molto forte per interpolazione fluida
    static constexpr int peakHoldTime = 45;            // Peak hold più longo (1.5 secondi at 30fps)
    static constexpr float interpolationSpeed = 0.15f; // Velocità di interpolazione per fluidità perfetta

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
