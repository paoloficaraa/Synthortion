#include "Synthortion/SpectrumAnalyzer.h"
#include <cmath>
#include <algorithm>

namespace synthortion
{
    SpectrumAnalyzer::SpectrumAnalyzer()
        : fft (kFftOrder),
          window (kFftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        computeBandFrequencies();
        reset();
    }

    void SpectrumAnalyzer::computeBandFrequencies() noexcept
    {
        for (int b = 0; b < kNumBands; ++b)
        {
            const float ratio = static_cast<float> (b) / static_cast<float> (kNumBands - 1);
            bandFrequencies[static_cast<size_t> (b)] = kMinFreqHz * std::pow (kMaxFreqHz / kMinFreqHz, ratio);
        }
    }

    float SpectrumAnalyzer::getBandFrequency (int bandIndex) const noexcept
    {
        if (bandIndex < 0 || bandIndex >= kNumBands)
            return 0.0f;

        return bandFrequencies[static_cast<size_t> (bandIndex)];
    }

    void SpectrumAnalyzer::prepare (double newSampleRate, float frameRate) noexcept
    {
        sampleRate = juce::jmax (1000.0, newSampleRate);
        computeBandFrequencies();

        const float safeFrameRate = juce::jmax (1.0f, frameRate);
        decayFactor = std::exp (-1.0f / (safeFrameRate * kDefaultDecayTimeSeconds));
    }

    void SpectrumAnalyzer::reset() noexcept
    {
        fftData.fill (0.0f);
        smoothedBands.fill (0.0f);
    }

    const std::array<float, SpectrumAnalyzer::kNumBands>& SpectrumAnalyzer::process (const float* timeDomainSamples, int numSamples) noexcept
    {
        if (timeDomainSamples == nullptr || numSamples <= 0)
        {
            for (auto& band : smoothedBands)
            {
                band *= decayFactor;
                if (band < 0.0001f)
                    band = 0.0f;
            }
            return smoothedBands;
        }

        fftData.fill (0.0f);
        const int samplesToCopy = juce::jmin (numSamples, kFftSize);
        std::copy_n (timeDomainSamples, samplesToCopy, fftData.data());

        // Apply Hann window
        window.multiplyWithWindowingTable (fftData.data(), kFftSize);

        // Compute forward FFT magnitudes in-place
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        // Coherent gain of Hann window is 0.5, so peak amplitude 1.0 sine produces FFT magnitude ~ kFftSize * 0.5
        const float scaleFactor = 2.0f / static_cast<float> (kFftSize);
        const float binWidthHz = static_cast<float> (sampleRate) / static_cast<float> (kFftSize);
        const float invBinWidth = 1.0f / juce::jmax (1.0e-3f, binWidthHz);
        const int maxBin = kFftSize / 2 - 1;

        for (int b = 0; b < kNumBands; ++b)
        {
            const float fc = bandFrequencies[static_cast<size_t> (b)];
            const float fLow = kMinFreqHz * std::pow (kMaxFreqHz / kMinFreqHz, (static_cast<float> (b) - 0.5f) / static_cast<float> (kNumBands - 1));
            const float fHigh = kMinFreqHz * std::pow (kMaxFreqHz / kMinFreqHz, (static_cast<float> (b) + 0.5f) / static_cast<float> (kNumBands - 1));

            const float kCenter = fc * invBinWidth;
            const float kLow = fLow * invBinWidth;
            const float kHigh = fHigh * invBinWidth;

            const int binStart = juce::jlimit (0, maxBin, static_cast<int> (std::floor (kLow)));
            const int binEnd   = juce::jlimit (0, maxBin, static_cast<int> (std::ceil (kHigh)));

            // Sub-bin linear interpolation
            const int idx = juce::jlimit (0, maxBin - 1, static_cast<int> (std::floor (kCenter)));
            const float frac = juce::jlimit (0.0f, 1.0f, kCenter - static_cast<float> (idx));
            const float mag0 = fftData[static_cast<size_t> (idx)] * scaleFactor;
            const float mag1 = fftData[static_cast<size_t> (idx + 1)] * scaleFactor;
            const float interpMag = mag0 + frac * (mag1 - mag0);

            // Peak bin aggregation
            float peakMag = 0.0f;
            for (int j = binStart; j <= binEnd; ++j)
            {
                peakMag = juce::jmax (peakMag, fftData[static_cast<size_t> (j)] * scaleFactor);
            }

            float rawMag = interpMag;
            if (fc <= 200.0f || binEnd == binStart)
            {
                rawMag = interpMag;
            }
            else if (fc >= 2000.0f)
            {
                rawMag = peakMag;
            }
            else
            {
                rawMag = juce::jmax (interpMag, peakMag);
            }

            const float db = magnitudeToDb (rawMag);
            const float normalized = normalizeDb (db);

            // Ballistics: instant peak attack, exponential decay
            auto& smoothed = smoothedBands[static_cast<size_t> (b)];
            if (normalized >= smoothed)
            {
                smoothed = normalized;
            }
            else
            {
                smoothed *= decayFactor;
                if (smoothed < 0.0001f)
                    smoothed = 0.0f;
            }
        }

        return smoothedBands;
    }
}
