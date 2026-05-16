#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class ParametricEQ;

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override = default;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void timerCallback() override;

    void pushNextSampleIntoFifo(float sample) noexcept;

    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }
    void setEQReference(ParametricEQ *eq) { eqReference = eq; }
    void setEQBypass(bool bypassed) { eqBypassed = bypassed; }

private:
    static constexpr int kFftOrder = 12;
    static constexpr int kFftSize = 1 << kFftOrder;
    static constexpr int kScopeSize = 512;
    static constexpr int kTimerHz = 60;
    static constexpr int kEqCurvePoints = 200;
    
    static constexpr float kMinFreq = 20.0f;
    static constexpr float kMaxFreq = 20000.0f;
    static constexpr float kMinGainDb = -100.0f;
    static constexpr float kMaxGainDb = 0.0f;
    static constexpr float kGainRangeDb = 60.0f;
    
    static constexpr float kSmoothingFactor = 0.85f;
    static constexpr float kInterpolationSpeed = 0.15f;
    static constexpr float kTargetDecay = 0.995f;
    static constexpr float kPeakDecay = 0.992f;
    static constexpr float kChangeThreshold = 0.002f;
    
    static constexpr int kPeakHoldTime = 45;
    
    static constexpr float kOuterPadding = 2.0f;
    static constexpr float kPlotPaddingH = 15.0f;
    static constexpr float kPlotPaddingV = 18.0f;
    static constexpr float kCornerRadius = 8.0f;
    static constexpr float kInnerCornerRadius = 7.0f;
    
    static constexpr float kFftScale = 2.0f;
    static constexpr float kWindowCorrection = 1.5f;
    static constexpr float kMinMagnitude = 1.0e-9f;
    
    static constexpr float kMainLineWidth = 2.0f;
    static constexpr float kEqLineWidth = 2.5f;
    static constexpr float kGlowWidth = 5.0f;
    static constexpr int kGlowLayers = 2;

    void drawNextFrameOfSpectrum();
    void drawFrame(juce::Graphics &g);
    void drawEQCurve(juce::Graphics &g);
    void drawGrid(juce::Graphics &g, const juce::Rectangle<float>& plot);
    void drawFrequencyLabels(juce::Graphics &g, const juce::Rectangle<float>& plot);
    
    float frequencyToX(float freq, const juce::Rectangle<float>& plot) const;
    float gainToY(float gainDb, const juce::Rectangle<float>& plot) const;

    juce::dsp::FFT forwardFFT{kFftOrder};
    juce::dsp::WindowingFunction<float> window{kFftSize, juce::dsp::WindowingFunction<float>::hann};

    std::array<float, kFftSize> fifo{};
    std::array<float, 2 * kFftSize> fftData{};
    std::array<float, kScopeSize> scopeData{};
    std::array<float, kScopeSize> smoothedScopeData{};
    std::array<float, kScopeSize> targetScopeData{};
    std::array<float, kScopeSize> currentScopeData{};
    std::array<float, kScopeSize> peakHoldData{};
    std::array<int, kScopeSize> peakHoldTimer{};
    
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady{false};
    
    juce::Array<juce::Point<float>> cachedPoints;
    double sampleRate = 0.0;

    ParametricEQ *eqReference = nullptr;
    bool eqBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
