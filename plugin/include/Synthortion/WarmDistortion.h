#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "Synthortion/DspModule.h"

namespace synthortion::dsp {

class WarmDistortion
{
public:
    WarmDistortion();
    ~WarmDistortion() = default;

    void prepare(const juce::dsp::ProcessSpec &spec);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& buffer, const WarmDistortionParams& params);
    int getLatencySamples() const noexcept { return oversampler ? static_cast<int>(oversampler->getLatencyInSamples()) : 0; }

    static constexpr int kNumChannels = 2;
    static constexpr int kOversamplingFactor = 2; // 2^2 = 4x Polyphase IIR
    static constexpr int kPinkNoiseStages = 7;
    
    static constexpr float kMinDrive = 0.0f;
    static constexpr float kMaxDrive = 1.0f;
    static constexpr float kMinDriveThreshold = 0.001f;
    static constexpr float kExciterGateThreshold = 0.4f;
    static constexpr float kExciterFrequency = 3500.0f;
    static constexpr float kDampingBaseFreq = 18000.0f;
    static constexpr float kDampingMinFreq = 10000.0f;
    static constexpr float kMaxFilterAlpha = 0.999f;
    
    static constexpr float kDenormThreshold = 1.0e-20f;
    static constexpr float kDenormNoiseLevel = 1.0e-18f;
    static constexpr float kDenormNoiseOffset = 0.5f;
    
    static constexpr float kTapeModulationNoise = 2.0e-6f;
    static constexpr float kThermalNoiseBase = 1.0e-6f;
    static constexpr float kNoiseFloorThreshold = 0.001f;
    static constexpr float kNoiseGlobalScale = 0.2f;
    static constexpr float kTapeNoiseDriveScale = 1.2f;
    static constexpr float kBiasNoiseScale = 0.5f;
    
    static constexpr float kExciterHarmonicDrive = 1.5f;
    static constexpr float kExciterPositiveSaturation = 0.85f;
    static constexpr float kExciterNegativeSaturation = 1.15f;
    static constexpr float kExciterNegativeAsymmetry = 0.90f;
    static constexpr float kExciterMixAmount = 0.08f;
    
    static constexpr float kWowFrequency = 1.5f;
    static constexpr float kFlutterFrequency = 10.0f;
    static constexpr float kWowDepthMax = 0.25f;
    static constexpr float kFlutterDepthMax = 0.06f;
    static constexpr int kWowFlutterBufferSize = 32;
    static constexpr float kWowFlutterBaseDelayMs = 0.2f;
    static constexpr float kWowFlutterMixAmount = 0.4f;
    static constexpr float kWowFlutterMinDelay = 1.0f;
    static constexpr int kWowFlutterSafetyMargin = 2;
    
    static constexpr float kCompensationSmoothingTime = 0.05f;
    
    static constexpr float kPinkB0 = 0.99886f;
    static constexpr float kPinkA0 = 0.0555179f;
    static constexpr float kPinkB1 = 0.99332f;
    static constexpr float kPinkA1 = 0.0750759f;
    static constexpr float kPinkB2 = 0.96900f;
    static constexpr float kPinkA2 = 0.1538520f;
    static constexpr float kPinkB3 = 0.86650f;
    static constexpr float kPinkA3 = 0.3104856f;
    static constexpr float kPinkB4 = 0.55000f;
    static constexpr float kPinkA4 = 0.5329522f;
    static constexpr float kPinkB5 = -0.7616f;
    static constexpr float kPinkA5 = -0.0168980f;
    static constexpr float kPinkA6 = 0.115926f;
    static constexpr float kPinkWhiteScale = 0.5362f;

    // Mathematical specification methods
    static float calculateInputGain(float drive) noexcept
    {
        const float d = juce::jlimit(kMinDrive, kMaxDrive, drive);
        return std::pow(10.0f, 1.2f * std::pow(d, 2.2f));
    }

    static float calculateDynamicBias(float drive) noexcept
    {
        const float d = juce::jlimit(kMinDrive, kMaxDrive, drive);
        return 0.25f * d * (1.0f - 0.4f * d);
    }

    static float asymmetricTanh(float x, float bias) noexcept
    {
        const float tanhB = std::tanh(bias);
        const float denom = 1.0f - (tanhB * tanhB);
        return (std::tanh(x + bias) - tanhB) / denom;
    }

    static float calculateVolumeCompensation(float drive) noexcept
    {
        const float gIn = calculateInputGain(drive);
        const float gInSq = gIn * gIn;
        const float denom = 1.0f + 0.28f * gInSq;
        const float term = 1.05f * (gInSq - 1.0f) / denom;
        return 1.0f / std::sqrt(1.0f + term);
    }

    static float calculateDampingFrequency(float drive) noexcept
    {
        const float d = juce::jlimit(kMinDrive, kMaxDrive, drive);
        return kDampingBaseFreq - (kDampingBaseFreq - kDampingMinFreq) * d;
    }

    float applySaturation(float input, float drive, int channel);
    void addDenormalizationNoise(float &sample, int channel);
    void addAnalogNoise(float &sample, float drive, int channel);
    void applyDriveDependentFiltering(float &sample, float drive, int channel);
    void applyHighFrequencyExciter(float &sample, float drive, int channel);
    void applyWowAndFlutter(float& sample, float drive, int channel);
    
    float getOversampledSampleRate() const;
    size_t getSafeChannel(size_t channel) const;

    float driveAmount = 0.0f;
    bool volumeCompensationEnabled = false;

    std::array<float, kNumChannels> dampingState{};
    std::array<float, kNumChannels> exciterHighpass{};
    std::array<std::array<float, kPinkNoiseStages>, kNumChannels> pinkNoiseState{};
    
    std::array<float, kNumChannels> wowPhase{};
    std::array<float, kNumChannels> flutterPhase{};
    std::array<std::array<float, kWowFlutterBufferSize>, kNumChannels> wowFlutterBuffer{};
    std::array<size_t, kNumChannels> wowFlutterWritePos{};

    std::array<juce::dsp::FirstOrderTPTFilter<float>, kNumChannels> dcBlockers;

    juce::LinearSmoothedValue<float> compensationGain{1.0f};
    juce::LinearSmoothedValue<float> smoothedDrive{0.0f};

    double sampleRate = 0.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    std::array<juce::Random, kNumChannels> noiseGenerator;
    std::vector<float> blockDriveValues;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};

static_assert(DspModule<WarmDistortion, WarmDistortionParams>);

} // namespace synthortion::dsp

namespace synthortion {
using WarmDistortion = dsp::WarmDistortion;
}