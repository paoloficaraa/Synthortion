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
    float applySaturation(float input, float drive);

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
    float tubeSaturation(float input, float drive);

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
    float applyBitCrush(float input);

    /**
     * @brief Add denormalization noise to prevent CPU spikes
     * @param sample Reference to sample to modify
     */
    void addDenormalizationNoise(float &sample);

    /**
     * @brief Calculate volume compensation factor based on drive and saturation type
     * @param drive Current drive amount [0.0-1.0]
     * @param type Current saturation type
     * @return Compensation factor to apply to output
     */
    float calculateVolumeCompensation(float drive, SaturationType type) const;

    // Constants for saturation algorithms
    static constexpr float SMOOTH_DRIVE_MIN = 1.0f;
    static constexpr float SMOOTH_DRIVE_MAX = 20.0f;
    static constexpr float TUBE_POSITIVE_FACTOR = 0.7f;
    static constexpr float TUBE_POSITIVE_GAIN = 1.2f;
    static constexpr float TUBE_NEGATIVE_FACTOR = 0.9f;
    static constexpr float TUBE_NEGATIVE_CLAMP = -0.9f;
    static constexpr float TUBE_DRIVE_MIN = 1.0f;
    static constexpr float TUBE_DRIVE_MAX = 15.0f;
    static constexpr float TAPE_DRIVE_MIN = 1.0f;
    static constexpr float TAPE_DRIVE_MAX = 8.0f;
    static constexpr float TAPE_KNEE_THRESHOLD = 0.5f;
    static constexpr float TAPE_COMPRESSION_FACTOR = 2.0f;

    // Bit crush constants
    static constexpr float BITCRUSH_BITS = 14.0f;
    static constexpr float BITCRUSH_MIX = 0.15f;

    // Denormalization constants
    static constexpr float DENORM_THRESHOLD = 1.0e-15f;
    static constexpr float DENORM_NOISE_LEVEL = 1.0e-30f;

    // Main parameters
    float driveAmount = 0.5f;
    // Internal mix removed; always process 100% wet, mix later globally
    SaturationType saturationType = SaturationType::SMOOTH;
    bool volumeCompensationEnabled = true;

    // Volume compensation smoothing
    juce::LinearSmoothedValue<float> compensationGain{1.0f};

    // Audio processing setup
    double sampleRate = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    static constexpr int OVERSAMPLING_FACTOR = 3; // 2^3 = 8x oversampling

    // Noise generator for denormalization
    juce::Random noiseGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};