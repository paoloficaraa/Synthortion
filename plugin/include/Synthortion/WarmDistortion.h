#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
 * @brief High-quality warm distortion processor with multiple saturation algorithms
 *
 * Features 8x oversampling, multiple saturation types, and bit crushing effects.
 * Designed for musical distortion with minimal aliasing artifacts.
 */
class WarmDistortion
{
public:
    enum class SaturationType
    {
        SMOOTH, ///< Smooth tanh-based saturation
        TUBE,   ///< Asymmetric tube-style saturation
        TAPE    ///< Soft-knee tape-style compression
    };

    WarmDistortion();
    ~WarmDistortion() = default;

    void setSampleRate(double sampleRate);
    void reset();
    void prepare(const juce::dsp::ProcessSpec &spec);

    /**
     * @brief Set drive amount for saturation intensity
     * @param drive Drive amount [0.0 - 1.0] where 0 = clean, 1 = maximum saturation
     */
    void setDrive(float drive);

    /**
     * @brief Set saturation algorithm type
     * @param type Saturation algorithm (SMOOTH/TUBE/TAPE)
     */
    void setSaturationType(SaturationType type);

    /**
     * @brief Enable or disable automatic volume compensation
     * @param enabled True to enable auto-compensation, false to disable
     */
    void setVolumeCompensation(bool enabled) { volumeCompensationEnabled = enabled; }

    /**
     * @brief Process audio buffer with warm distortion
     * @param context DSP processing context
     */
    void process(const juce::dsp::ProcessContextReplacing<float> &context);

    /**
     * @brief Get processing latency in samples due to oversampling filters
     * @return Estimated latency (samples) for aligning dry/wet paths
     */
    int getLatencySamples() const { return oversampler ? static_cast<int>(oversampler->getLatencyInSamples()) : 0; }

private:
    // Audio processing methods
    float applySaturation(float input, float drive, int channel);

    /**
     * @brief Smooth tanh-based saturation
     * @param input Input sample [-1.0, 1.0]
     * @param drive Drive amount [0.0, 1.0]
     * @return Processed sample with smooth saturation characteristics
     */
    float smoothSaturation(float input, float drive);

    /**
     * @brief Tube-style asymmetric saturation
     * @param input Input sample [-1.0, 1.0]
     * @param drive Drive amount [0.0, 1.0]
     * @return Processed sample with tube characteristics
     */
    float tubeSaturation(float input, float drive, int channel);

    /**
     * @brief Tape-style soft-knee saturation
     * @param input Input sample [-1.0, 1.0]
     * @param drive Drive amount [0.0, 1.0]
     * @return Processed sample with tape characteristics
     */
    float tapeSaturation(float input, float drive);

    /**
     * @brief Apply subtle bit crushing effect
     * @param input Input sample
     * @return Bit-crushed sample with 14-bit quantization at 15% mix
     */
    float applyBitCrush(float input, int channel);

    /**
     * @brief Add denormalization noise to prevent CPU spikes
     * @param sample Reference to sample to modify
     * @param channel Channel index for independent noise generation
     */
    void addDenormalizationNoise(float &sample, int channel);

    /**
     * @brief Add realistic analog noise based on saturation type and drive
     * @param sample Reference to sample to modify
     * @param drive Current drive amount for noise scaling
     */
    void addAnalogNoise(float &sample, float drive, int channel);

    /**
     * @brief Apply drive-dependent filtering that simulates analog component saturation
     * @param sample Reference to sample to modify
     * @param drive Current drive amount [0.0-1.0]
     */
    void applyDriveDependentFiltering(float &sample, float drive, int channel);

    /**
     * @brief Apply high-frequency exciter for more "crispness" and presence
     * @param sample Reference to sample to modify
     * @param drive Current drive amount [0.0-1.0]
     * @param channel Channel index for state management
     */
    void applyHighFrequencyExciter(float &sample, float drive, int channel);

    /**
     * @brief Calculate volume compensation factor based on drive and saturation type
     * @param drive Current drive amount [0.0-1.0]
     * @param type Current saturation type
     * @return Compensation factor to apply to output
     */
    float calculateVolumeCompensation(float drive, SaturationType type) const;

    /**
     * @brief Updates the analog modeling state once per processing block.
     * @param numSamples The number of samples in the current block.
     */
    void updateAnalogModelState(int numSamples);

    // Constants for saturation algorithms
    static constexpr float SMOOTH_DRIVE_MIN = 1.0f;
    static constexpr float SMOOTH_DRIVE_MAX = 20.0f;

    // Advanced Tube modeling constants (12AX7 based)
    static constexpr float TUBE_DRIVE_MIN = 1.0f;
    static constexpr float TUBE_DRIVE_MAX = 15.0f;
    static constexpr float TUBE_BIAS_DRIFT = 0.02f;         // Simula drift del bias
    static constexpr float TUBE_GRID_CURRENT = 0.85f;       // Soglia grid current
    static constexpr float TUBE_PLATE_KNEE = 0.7f;          // Knee point della placca
    static constexpr float TUBE_CATHODE_COMPRESSION = 1.8f; // Compressione del catodo
    static constexpr float TUBE_ASYMMETRY_FACTOR = 0.3f;    // Asimmetria variabile
    static constexpr float TUBE_HARMONIC_CONTENT = 0.15f;   // Contenuto armonico

    static constexpr float TAPE_DRIVE_MIN = 1.0f;
    static constexpr float TAPE_DRIVE_MAX = 8.0f;
    static constexpr float TAPE_KNEE_THRESHOLD = 0.5f;
    static constexpr float TAPE_COMPRESSION_FACTOR = 2.0f;

    // Bit crush constants
    static constexpr float BITCRUSH_BITS = 14.0f;
    static constexpr float BITCRUSH_MIX = 0.05f;

    // Denormalization constants
    static constexpr float DENORM_THRESHOLD = 1.0e-20f;
    static constexpr float DENORM_NOISE_LEVEL = 1.0e-35f;

    // Analog noise modeling constants
    static constexpr float TUBE_HISS_BASE = 2.0e-6f;        // Tube hiss base level
    static constexpr float TUBE_FLICKER_NOISE = 1.5e-6f;    // 1/f noise per i tubi
    static constexpr float TAPE_MODULATION_NOISE = 3.0e-6f; // Tape modulation noise
    static constexpr float THERMAL_NOISE_BASE = 1.0e-6f;    // Thermal noise base

    // Drive-dependent filtering constants
    static constexpr float PREEMPH_BASE_FREQ = 1000.0f;    // Base frequency per pre-emphasis
    static constexpr float PREEMPH_DRIVE_FACTOR = 0.4f;    // Quanto aumenta con il drive
    static constexpr float POSTFILTER_BASE_FREQ = 5000.0f; // Base frequency per post-filter
    static constexpr float POSTFILTER_DRIVE_FACTOR = 0.7f; // Quanto diminuisce con il drive

    // High-frequency exciter constants
    static constexpr float EXCITER_HIGHPASS_FREQ = 3000.0f; // Frequenza di taglio per estrarre alte
    static constexpr float EXCITER_HARMONIC_DRIVE = 2.5f;   // Drive per generare armoniche
    static constexpr float EXCITER_MIX_AMOUNT = 0.15f;      // Quantità di exciter da mixare

    // Main parameters
    float driveAmount = 0.5f;
    // Internal mix removed; always process 100% wet, mix later globally
    SaturationType saturationType = SaturationType::SMOOTH;
    bool volumeCompensationEnabled = true;

    // Advanced analog modeling state
    float tubeBiasDrift[2] = {0.0f, 0.0f};           // Simula drift termico del bias
    float tubeWarmupFactor[2] = {0.0f, 0.0f};        // Simula riscaldamento del tubo
    int samplesSinceReset = 0;                       // Contatore per effetti termici
    float flickerNoiseAccumulator[2] = {0.0f, 0.0f}; // Per 1/f noise

    // Drive-dependent filtering state (simple one-pole filters)
    float preEmphState[2] = {0.0f, 0.0f};    // Pre-emphasis filter state
    float postFilterState[2] = {0.0f, 0.0f}; // Post-filter state

    // High-frequency exciter state
    float exciterHighpass[2] = {0.0f, 0.0f}; // High-pass filter per estrarre alte frequenze
    float exciterDelay[2] = {0.0f, 0.0f};    // Delay line per generare armoniche

    // Volume compensation smoothing
    juce::LinearSmoothedValue<float> compensationGain{1.0f};

    // Audio processing setup
    double sampleRate = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    static constexpr int OVERSAMPLING_FACTOR = 3; // 2^3 = 8x oversampling

    // Noise generator for denormalization
    juce::Random noiseGenerator[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};