#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace synthortion
{
    /**
     * FFT spectrum analyzer engine.
     * Computes order-11 (2048-point) Hann-windowed FFTs, pools raw bins into 80 log-spaced
     * frequency bands (20 Hz to 20 kHz) with sub-bin linear interpolation for low frequencies
     * (20 Hz - 200 Hz) and peak bin aggregation for high frequencies (2 kHz - 20 kHz),
     * converts magnitudes to decibels clamped to [-60 dB, 0 dB] normalized to [0.0, 1.0],
     * and applies asymmetric peak attack and exponential decay ballistics (~150-200 ms).
     */
    class SpectrumAnalyzer
    {
    public:
        static constexpr int kFftOrder = 11;
        static constexpr int kFftSize = 1 << kFftOrder; // 2048
        static constexpr int kNumBands = 80;
        static constexpr float kMinFreqHz = 20.0f;
        static constexpr float kMaxFreqHz = 20000.0f;
        static constexpr float kMinDb = -60.0f;
        static constexpr float kMaxDb = 0.0f;
        static constexpr float kDefaultDecayTimeSeconds = 0.18f; // ~180 ms decay
        static constexpr float kDefaultFrameRateHz = 60.0f;

        SpectrumAnalyzer();
        ~SpectrumAnalyzer() = default;

        /** Prepares the analyzer for the given sample rate and display timer frame rate. */
        void prepare (double newSampleRate, float frameRate = kDefaultFrameRateHz) noexcept;

        /** Clears FFT buffers and resets smoothed band ballistics to zero. */
        void reset() noexcept;

        /**
         * Processes input time-domain samples and returns the current 80 normalized band magnitudes in [0.0, 1.0].
         */
        const std::array<float, kNumBands>& process (const float* timeDomainSamples, int numSamples) noexcept;

        /** Returns the center frequency in Hz of the band at index [0..79]. */
        float getBandFrequency (int bandIndex) const noexcept;

        /** Returns the current smoothed band magnitudes. */
        const std::array<float, kNumBands>& getSmoothedBands() const noexcept { return smoothedBands; }

        /** Returns the computed per-frame decay factor. */
        float getDecayFactor() const noexcept { return decayFactor; }

        /** Normalizes a decibel value [-60 dB, 0 dB] into [0.0, 1.0]. */
        static float normalizeDb (float db) noexcept
        {
            if (db <= kMinDb)
                return 0.0f;
            if (db >= kMaxDb)
                return 1.0f;
            return (db - kMinDb) / (kMaxDb - kMinDb);
        }

        /** Converts a linear magnitude into decibels. */
        static float magnitudeToDb (float magnitude) noexcept
        {
            return 20.0f * std::log10 (juce::jmax (magnitude, 1.0e-6f));
        }

    private:
        void computeBandFrequencies() noexcept;

        juce::dsp::FFT fft;
        juce::dsp::WindowingFunction<float> window;

        std::array<float, kFftSize * 2> fftData {};
        std::array<float, kNumBands> bandFrequencies {};
        std::array<float, kNumBands> smoothedBands {};

        double sampleRate = 48000.0;
        float decayFactor = 0.91f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzer)
    };
}
