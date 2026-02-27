#include "Synthortion/WarmDistortion.h"

WarmDistortion::WarmDistortion()
{
    reset();
}

void WarmDistortion::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    if (oversampler)
        oversampler->reset();
}

void WarmDistortion::reset()
{
    for (auto& gen : noiseGenerator)
        gen.setSeedRandomly();

    preEmphState.fill(0.0f);
    postFilterState.fill(0.0f);
    exciterHighpass.fill(0.0f);
    exciterDelay.fill(0.0f);
    hysteresisState.fill(0.0f);
    wowPhase.fill(0.0f);
    flutterPhase.fill(0.0f);
    wowFlutterWritePos.fill(0);

    for (auto& channelState : pinkNoiseState)
        channelState.fill(0.0f);

    for (auto& buffer : wowFlutterBuffer)
        buffer.fill(0.0f);

    compensationGain.setCurrentAndTargetValue(kCompensationMax);

    if (oversampler)
        oversampler->reset();
}

void WarmDistortion::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        kOversamplingFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    oversampler->initProcessing(spec.maximumBlockSize);

    compensationGain.reset(sampleRate, kCompensationSmoothingTime);
    compensationGain.setCurrentAndTargetValue(kCompensationMax);
}

void WarmDistortion::setDrive(float newDrive)
{
    driveAmount = juce::jlimit(kMinDrive, kMaxDrive, newDrive);

    if (volumeCompensationEnabled)
    {
        const float compensation = calculateVolumeCompensation(driveAmount);
        compensationGain.setTargetValue(compensation);
    }
}

float WarmDistortion::getOversampledSampleRate() const
{
    return static_cast<float>(sampleRate) * (1 << kOversamplingFactor);
}

int WarmDistortion::getSafeChannel(int channel) const
{
    return juce::jlimit(0, kNumChannels - 1, channel);
}

void WarmDistortion::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    juce::dsp::AudioBlock<float> block = context.getOutputBlock();

    if (driveAmount < kMinDriveThreshold)
        return;

    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        float* channelData = oversampledBlock.getChannelPointer(channel);
        const int numSamples = static_cast<int>(oversampledBlock.getNumSamples());

        for (int i = 0; i < numSamples; ++i)
        {
            float s = channelData[i];

            addDenormalizationNoise(s, channel);
            applyWowAndFlutter(s, driveAmount, channel);
            applyDriveDependentFiltering(s, driveAmount, channel);
            applyHighFrequencyExciter(s, driveAmount, channel);
            s = applySaturation(s, driveAmount, channel);
            addAnalogNoise(s, driveAmount, channel);

            channelData[i] = s;
        }
    }

    oversampler->processSamplesDown(block);

    if (driveAmount > kLowDriveThreshold)
    {
        const float finalCompensation = juce::jmap(driveAmount, kLowDriveThreshold, kMaxDrive, 
                                                   kCompensationMax, kFinalCompensationMin);

        for (int channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float* channelData = block.getChannelPointer(channel);
            const int numSamples = static_cast<int>(block.getNumSamples());

            for (int i = 0; i < numSamples; ++i)
                channelData[i] = std::tanh(channelData[i] * kFinalClipperGain) * kFinalClipperOutput * finalCompensation;
        }
    }

    if (volumeCompensationEnabled)
    {
        if (compensationGain.isSmoothing())
        {
            for (int channel = 0; channel < block.getNumChannels(); ++channel)
            {
                float* channelData = block.getChannelPointer(channel);
                const int numSamples = static_cast<int>(block.getNumSamples());

                if (channel > 0)
                    compensationGain.setCurrentAndTargetValue(compensationGain.getTargetValue());

                for (int i = 0; i < numSamples; ++i)
                    channelData[i] *= compensationGain.getNextValue();
            }
        }
        else
        {
            const float gain = compensationGain.getTargetValue();
            for (int channel = 0; channel < block.getNumChannels(); ++channel)
            {
                juce::FloatVectorOperations::multiply(
                    block.getChannelPointer(channel),
                    gain,
                    static_cast<int>(block.getNumSamples()));
            }
        }
    }
    else
    {
        compensationGain.skip(static_cast<int>(block.getNumSamples()));
    }
}

float WarmDistortion::applySaturation(float input, float drive, int channel)
{
    const int safeChannel = getSafeChannel(channel);

    const float hysteresisEffect = hysteresisState[safeChannel] * kHysteresisAmount * drive;
    const float inputWithHysteresis = input + hysteresisEffect;

    const float output = tapeSaturation(inputWithHysteresis, drive);

    hysteresisState[safeChannel] = output * kHysteresisInputScale + 
                                   hysteresisState[safeChannel] * kHysteresisFeedback;

    return output;
}

float WarmDistortion::tapeSaturation(float input, float drive) const
{
    const float driveMapped = juce::jmap(drive, kMinDrive, kMaxDrive, kTapeDriveMin, kTapeDriveMax);
    const float driven = input * driveMapped;

    const float stage1 = std::tanh(driven * kTapeStage1Gain);

    const float asymmetry = kTapeAsymmetryBase * drive;
    const float stage2 = stage1 + asymmetry * stage1 * stage1;

    const float abs_val = std::abs(stage2);
    float output;

    if (abs_val < kTapeHardClipKnee)
    {
        output = stage2;
    }
    else
    {
        const float sign = (stage2 > 0.0f) ? 1.0f : -1.0f;
        const float excess = abs_val - kTapeHardClipKnee;
        output = sign * (kTapeHardClipKnee + excess / (1.0f + excess * kTapeHardClipRatio));
    }

    const float compensation = 1.0f / (1.0f + drive * kTapeCompensationFactor);
    return output * compensation;
}

void WarmDistortion::addDenormalizationNoise(float &sample, int channel)
{
    if (std::abs(sample) < kDenormThreshold)
    {
        const int safeChannel = getSafeChannel(channel);
        sample += (noiseGenerator[safeChannel].nextFloat() - kDenormNoiseOffset) * kDenormNoiseLevel;
    }
}

void WarmDistortion::addAnalogNoise(float &sample, float drive, int channel)
{
    const int safeChannel = getSafeChannel(channel);

    const float white = noiseGenerator[safeChannel].nextFloat() - kDenormNoiseOffset;

    auto& state = pinkNoiseState[safeChannel];
    state[0] = kPinkB0 * state[0] + white * kPinkA0;
    state[1] = kPinkB1 * state[1] + white * kPinkA1;
    state[2] = kPinkB2 * state[2] + white * kPinkA2;
    state[3] = kPinkB3 * state[3] + white * kPinkA3;
    state[4] = kPinkB4 * state[4] + white * kPinkA4;
    state[5] = kPinkB5 * state[5] - white * kPinkA5;

    const float pink = state[0] + state[1] + state[2] + state[3] + state[4] + state[5] + state[6] + white * kPinkWhiteScale;

    state[6] = white * kPinkA6;

    const float tapeNoise = kTapeModulationNoise * (1.0f + drive * kTapeNoiseDriveScale);
    const float biasNoise = kThermalNoiseBase * kBiasNoiseScale;
    const float noiseLevel = (tapeNoise + biasNoise) * kNoiseGlobalScale;

    if (std::abs(sample) > kNoiseFloorThreshold || drive > kHighDriveThreshold)
        sample += pink * noiseLevel;
}

float WarmDistortion::calculateVolumeCompensation(float drive) const
{
    if (drive < kMinDriveThreshold)
        return kCompensationMax;

    const float compensationFactor = juce::jmap(drive, kMinDrive, kMaxDrive, kCompensationMax, kCompensationScale);
    const float smoothedCompensation = std::pow(compensationFactor, kCompensationPower);

    return juce::jlimit(kCompensationMin, kCompensationMax, smoothedCompensation);
}

void WarmDistortion::applyDriveDependentFiltering(float& sample, float drive, int channel)
{
    const int safeChannel = getSafeChannel(channel);

    if (drive < kMinDriveThreshold)
        return;

    const float oversampledSampleRate = getOversampledSampleRate();

    const float preEmphFreq = kPreEmphBaseFreq * kPreEmphFreqScale * (1.0f + drive * kPreEmphDriveFactor * kPreEmphDriveScale);
    const float preEmphAlpha = std::min(kMaxFilterAlpha, 
                                       1.0f - std::exp(-juce::MathConstants<float>::twoPi * preEmphFreq / oversampledSampleRate));

    const float preEmphOutput = sample - preEmphState[safeChannel];
    preEmphState[safeChannel] += preEmphAlpha * preEmphOutput;

    const float preEmphAmount = drive * kPreEmphMixAmount;
    sample = sample * (1.0f - preEmphAmount) + preEmphOutput * preEmphAmount;

    const float postFilterFreq = std::max(kPostFilterMinFreq, kPostFilterBaseFreq * (1.0f - drive * kPostFilterDriveFactor));
    const float postFilterAlpha = std::min(kMaxFilterAlpha, 
                                          1.0f - std::exp(-juce::MathConstants<float>::twoPi * postFilterFreq / oversampledSampleRate));

    const float postFilterInput = sample;
    postFilterState[safeChannel] += postFilterAlpha * (postFilterInput - postFilterState[safeChannel]);

    const float postFilterAmount = drive * kPostFilterMixAmount;
    sample = sample * (1.0f - postFilterAmount) + postFilterState[safeChannel] * postFilterAmount;
}

void WarmDistortion::applyWowAndFlutter(float& sample, float drive, int channel)
{
    const int safeChannel = getSafeChannel(channel);

    if (drive < kMediumDriveThreshold)
        return;

    const float oversampledSampleRate = getOversampledSampleRate();

    wowFlutterBuffer[safeChannel][wowFlutterWritePos[safeChannel]] = sample;

    const float wowFreqRadians = juce::MathConstants<float>::twoPi * kWowFrequency / oversampledSampleRate;
    wowPhase[safeChannel] += wowFreqRadians;

    if (wowPhase[safeChannel] > juce::MathConstants<float>::twoPi)
        wowPhase[safeChannel] -= juce::MathConstants<float>::twoPi;

    const float wowModulation = std::sin(wowPhase[safeChannel]) * kWowDepthMax * drive;

    const float flutterFreqRadians = juce::MathConstants<float>::twoPi * kFlutterFrequency / oversampledSampleRate;
    flutterPhase[safeChannel] += flutterFreqRadians;

    if (flutterPhase[safeChannel] > juce::MathConstants<float>::twoPi)
        flutterPhase[safeChannel] -= juce::MathConstants<float>::twoPi;

    const float flutterModulation = std::sin(flutterPhase[safeChannel]) * kFlutterDepthMax * drive;

    const float totalModulation = wowModulation + flutterModulation;

    const float baseDelay = (kWowFlutterBaseDelayMs / 1000.0f) * oversampledSampleRate;
    const float modulatedDelay = juce::jlimit(kWowFlutterMinDelay, 
                                              static_cast<float>(kWowFlutterBufferSize - kWowFlutterSafetyMargin), 
                                              baseDelay + totalModulation);

    float readPosFloat = static_cast<float>(wowFlutterWritePos[safeChannel]) - modulatedDelay;

    while (readPosFloat < 0.0f)
        readPosFloat += static_cast<float>(kWowFlutterBufferSize);

    const int readPos1 = static_cast<int>(readPosFloat) % kWowFlutterBufferSize;
    const int readPos2 = (readPos1 + 1) % kWowFlutterBufferSize;

    const float frac = readPosFloat - std::floor(readPosFloat);

    const float sample1 = wowFlutterBuffer[safeChannel][readPos1];
    const float sample2 = wowFlutterBuffer[safeChannel][readPos2];
    const float interpolatedSample = sample1 + frac * (sample2 - sample1);

    const float mixAmount = drive * kWowFlutterMixAmount;
    sample = sample * (1.0f - mixAmount) + interpolatedSample * mixAmount;

    wowFlutterWritePos[safeChannel] = (wowFlutterWritePos[safeChannel] + 1) % kWowFlutterBufferSize;
}

void WarmDistortion::applyHighFrequencyExciter(float &sample, float drive, int channel)
{
    const int safeChannel = getSafeChannel(channel);

    if (drive < kMediumDriveThreshold)
        return;

    const float oversampledSampleRate = getOversampledSampleRate();

    const float cutoffFreq = kExciterHighpassFreq * (kExciterFreqDriveOffset + drive * kExciterFreqDriveRange);
    const float alpha = std::min(kMaxFilterAlpha, 
                                1.0f - std::exp(-juce::MathConstants<float>::twoPi * cutoffFreq / oversampledSampleRate));

    const float highFreqSignal = sample - exciterHighpass[safeChannel];
    exciterHighpass[safeChannel] += alpha * highFreqSignal;

    const float harmonicDrive = kExciterHarmonicDrive * drive;
    const float drivenHigh = highFreqSignal * harmonicDrive;

    float excitedSignal;
    if (drivenHigh >= 0.0f)
    {
        excitedSignal = std::tanh(drivenHigh * kExciterPositiveSaturation);
    }
    else
    {
        excitedSignal = std::tanh(drivenHigh * kExciterNegativeSaturation) * kExciterNegativeAsymmetry;
    }

    const float secondHarmonic = std::sin(highFreqSignal * juce::MathConstants<float>::twoPi) * kExciterSecondHarmonicAmp * drive;
    excitedSignal += secondHarmonic;

    const float exciterAmount = kExciterMixAmount * drive * drive;
    sample += excitedSignal * exciterAmount;
}