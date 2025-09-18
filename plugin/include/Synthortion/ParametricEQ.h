#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
 * @brief 4-band parametric equalizer with high-quality IIR filters and optional linear phase
 *
 * Features:
 * - High-pass and low-pass filters with adjustable Q
 * - Two parametric mid-band filters with gain and Q control
 * - Linear phase mode using FIR convolution
 * - Frequency response visualization support
 * - Real-time parameter updates
 */
class ParametricEQ
{
public:
    ParametricEQ();
    ~ParametricEQ();

    /**
     * @brief Prepare the EQ for processing
     * @param spec ProcessSpec containing sample rate, block size, and channel count
     */
    void prepare(const juce::dsp::ProcessSpec &spec);

    /**
     * @brief Process audio buffer through the EQ
     * @param context DSP processing context
     */
    void process(const juce::dsp::ProcessContextReplacing<float> &context);

    // EQ Band setters
    /**
     * @brief Set low-cut (high-pass) filter parameters
     * @param frequency Cutoff frequency in Hz [20-1000]
     * @param q Quality factor [0.1-10.0]
     */
    void setLowCut(float frequency, float q, bool enabled = true);

    /**
     * @brief Set low-mid parametric band parameters
     * @param frequency Center frequency in Hz [100-2000]
     * @param gain Gain in dB [-15 to +15]
     * @param q Quality factor [0.1-10.0]
     */
    void setLowMid(float frequency, float gain, float q);

    /**
     * @brief Set high-mid parametric band parameters
     * @param frequency Center frequency in Hz [1000-8000]
     * @param gain Gain in dB [-15 to +15]
     * @param q Quality factor [0.1-10.0]
     */
    void setHighMid(float frequency, float gain, float q);

    /**
     * @brief Set high-cut (low-pass) filter parameters
     * @param frequency Cutoff frequency in Hz [5000-20000]
     * @param q Quality factor [0.1-10.0]
     */
    void setHighCut(float frequency, float q, bool enabled = true);

    /**
     * @brief Enable or disable linear phase mode
     * @param enabled True for linear phase (FIR), false for minimum phase (IIR)
     */
    void setLinearPhase(bool enabled);

    /**
     * @brief Check if linear phase mode is enabled
     * @return True if linear phase mode is active
     */
    bool isLinearPhaseEnabled() const { return linearPhaseEnabled; }

    /**
     * @brief Get processing latency in samples
     * @return Latency in samples (0 for IIR mode, FIR length/2 for linear phase)
     */
    int getLatencySamples() const;

    /**
     * @brief Get frequency response for visualization
     * @param frequencies Vector of frequencies to evaluate
     * @return Vector of magnitude responses in dB
     */
    std::vector<float> getFrequencyResponse(const std::vector<float> &frequencies);

private:
    // 4-band EQ filters - using ProcessorDuplicator for stereo support
    using FilterType = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    FilterType lowCutFilter;  ///< High-pass filter for low frequencies
    FilterType lowMidFilter;  ///< Parametric filter for low-mid frequencies
    FilterType highMidFilter; ///< Parametric filter for high-mid frequencies
    FilterType highCutFilter; ///< Low-pass filter for high frequencies

    // Linear phase FIR convolution
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> firBuffer;
    static constexpr int firLength = 2048; ///< FIR filter length for linear phase
    bool linearPhaseEnabled = false;
    bool firNeedsUpdate = true;

    // Processing state
    double sampleRate = 44100.0;
    bool isPrepared = false;

    /**
     * @brief Update all filter coefficients based on current parameters
     * Called automatically when parameters change
     */
    void updateFilters();

    /**
     * @brief Generate FIR impulse response for linear phase mode
     * Creates an impulse response combining all EQ bands
     */
    void generateFIRResponse();

    // Filter parameters with sensible defaults
    float lowCutFreq = 20.0f, lowCutQ = 0.7f;                         ///< Low-cut: 20Hz, Q=0.7
    float lowMidFreq = 350.0f, lowMidGain = 0.0f, lowMidQ = 1.0f;     ///< Low-mid: 350Hz, 0dB, Q=1.0
    float highMidFreq = 3800.0f, highMidGain = 0.0f, highMidQ = 1.0f; ///< High-mid: 3.8kHz, 0dB, Q=1.0
    float highCutFreq = 20000.0f, highCutQ = 0.7f;                    ///< High-cut: 20kHz, Q=0.7

    bool lowCutEnabled = false;
    bool highCutEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};
