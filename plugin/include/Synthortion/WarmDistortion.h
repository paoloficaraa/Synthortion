#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class WarmDistortion
{
public:
    WarmDistortion();
    ~WarmDistortion() = default;

    void setSampleRate(double sampleRate);
    void reset();
    void prepare(const juce::dsp::ProcessSpec &spec);

    void setDrive(float drive);
    void setVolumeCompensation(bool enabled) { volumeCompensationEnabled = enabled; }
    void process(const juce::dsp::ProcessContextReplacing<float> &context);
    int getLatencySamples() const { return oversampler ? static_cast<int>(oversampler->getLatencyInSamples()) : 0; }

private:
    static constexpr int kNumChannels = 2;
    static constexpr int kOversamplingFactor = 3;
    static constexpr int kPinkNoiseStages = 7;
    
    static constexpr float kMinDrive = 0.0f;
    static constexpr float kMaxDrive = 1.0f;
    static constexpr float kMinDriveThreshold = 0.01f;
    static constexpr float kLowDriveThreshold = 0.05f;
    static constexpr float kMediumDriveThreshold = 0.15f;
    static constexpr float kHighDriveThreshold = 0.3f;
    
    static constexpr float kTapeDriveMin = 1.0f;
    static constexpr float kTapeDriveMax = 8.0f;
    static constexpr float kTapeStage1Gain = 1.2f;
    static constexpr float kTapeAsymmetryBase = 0.05f;
    static constexpr float kTapeHardClipKnee = 0.7f;
    static constexpr float kTapeHardClipRatio = 2.5f;
    static constexpr float kTapeCompensationFactor = 0.35f;
    
    static constexpr float kDenormThreshold = 1.0e-20f;
    static constexpr float kDenormNoiseLevel = 1.0e-35f;
    static constexpr float kDenormNoiseOffset = 0.5f;
    
    static constexpr float kTapeModulationNoise = 3.0e-6f;
    static constexpr float kThermalNoiseBase = 1.0e-6f;
    static constexpr float kNoiseFloorThreshold = 0.001f;
    static constexpr float kNoiseGlobalScale = 0.3f;
    static constexpr float kTapeNoiseDriveScale = 1.5f;
    static constexpr float kBiasNoiseScale = 0.5f;
    
    static constexpr float kPreEmphBaseFreq = 1000.0f;
    static constexpr float kPreEmphFreqScale = 1.5f;
    static constexpr float kPreEmphDriveFactor = 0.4f;
    static constexpr float kPreEmphDriveScale = 1.2f;
    static constexpr float kPreEmphMixAmount = 0.5f;
    static constexpr float kPostFilterBaseFreq = 5000.0f;
    static constexpr float kPostFilterMinFreq = 1200.0f;
    static constexpr float kPostFilterDriveFactor = 0.5f;
    static constexpr float kPostFilterMixAmount = 0.6f;
    static constexpr float kMaxFilterAlpha = 0.95f;
    
    static constexpr float kExciterHighpassFreq = 3000.0f;
    static constexpr float kExciterFreqDriveRange = 0.4f;
    static constexpr float kExciterFreqDriveOffset = 0.8f;
    static constexpr float kExciterHarmonicDrive = 2.5f;
    static constexpr float kExciterPositiveSaturation = 0.8f;
    static constexpr float kExciterNegativeSaturation = 1.2f;
    static constexpr float kExciterNegativeAsymmetry = 0.85f;
    static constexpr float kExciterSecondHarmonicAmp = 0.1f;
    static constexpr float kExciterMixAmount = 0.15f;
    
    static constexpr float kWowFrequency = 1.5f;
    static constexpr float kFlutterFrequency = 10.0f;
    static constexpr float kWowDepthMax = 0.35f;
    static constexpr float kFlutterDepthMax = 0.08f;
    static constexpr int kWowFlutterBufferSize = 32;
    static constexpr float kWowFlutterBaseDelayMs = 0.2f;
    static constexpr float kWowFlutterMixAmount = 0.6f;
    static constexpr float kWowFlutterMinDelay = 1.0f;
    static constexpr int kWowFlutterSafetyMargin = 2;
    
    static constexpr float kHysteresisAmount = 0.15f;
    static constexpr float kHysteresisFeedback = 0.7f;
    static constexpr float kHysteresisInputScale = 0.3f;
    
    static constexpr float kFinalClipperGain = 0.85f;
    static constexpr float kFinalClipperOutput = 1.15f;
    static constexpr float kFinalCompensationMin = 0.92f;
    
    static constexpr float kCompensationSmoothingTime = 0.1f;
    static constexpr float kCompensationPower = 0.9f;
    static constexpr float kCompensationMin = 0.65f;
    static constexpr float kCompensationMax = 1.0f;
    static constexpr float kCompensationScale = 0.85f;
    
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

    float applySaturation(float input, float drive, int channel);
    float tapeSaturation(float input, float drive) const;
    void addDenormalizationNoise(float &sample, int channel);
    void addAnalogNoise(float &sample, float drive, int channel);
    void applyDriveDependentFiltering(float &sample, float drive, int channel);
    void applyHighFrequencyExciter(float &sample, float drive, int channel);
    void applyWowAndFlutter(float& sample, float drive, int channel);
    float calculateVolumeCompensation(float drive) const;
    
    float getOversampledSampleRate() const;
    int getSafeChannel(int channel) const;

    float driveAmount = 0.5f;
    bool volumeCompensationEnabled = true;

    std::array<float, kNumChannels> preEmphState{};
    std::array<float, kNumChannels> postFilterState{};
    std::array<float, kNumChannels> exciterHighpass{};
    std::array<float, kNumChannels> exciterDelay{};
    std::array<float, kNumChannels> hysteresisState{};
    std::array<std::array<float, kPinkNoiseStages>, kNumChannels> pinkNoiseState{};
    
    std::array<float, kNumChannels> wowPhase{};
    std::array<float, kNumChannels> flutterPhase{};
    std::array<std::array<float, kWowFlutterBufferSize>, kNumChannels> wowFlutterBuffer{};
    std::array<int, kNumChannels> wowFlutterWritePos{};

    juce::LinearSmoothedValue<float> compensationGain{1.0f};

    double sampleRate = 0.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    std::array<juce::Random, kNumChannels> noiseGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WarmDistortion)
};