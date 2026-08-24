#include "Synthortion/WarmDistortion.h"

namespace synthortion::dsp {

WarmDistortion::WarmDistortion()
{
    reset();
}


void WarmDistortion::reset() noexcept
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

    for (auto& blocker : dcBlockers)
        blocker.reset();

    compensationGain.setCurrentAndTargetValue(kCompensationMax);
    smoothedDrive.setCurrentAndTargetValue(0.0f);

    if (oversampler)
        oversampler->reset();
}

void WarmDistortion::prepare(const juce::dsp::ProcessSpec &spec)
{
    jassert(spec.sampleRate > 0.0);
    jassert(spec.maximumBlockSize > 0);
    jassert(spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        kOversamplingFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    oversampler->initProcessing(spec.maximumBlockSize);

    smoothedDrive.reset(sampleRate, 0.05);
    smoothedDrive.setCurrentAndTargetValue(0.0f);

    compensationGain.reset(sampleRate, kCompensationSmoothingTime);
    compensationGain.setCurrentAndTargetValue(kCompensationMax);

    blockDriveValues.resize(juce::jmax(static_cast<size_t>(spec.maximumBlockSize), size_t(2048)));
    // Prepare DC blockers
    juce::dsp::ProcessSpec dcSpec{spec.sampleRate, spec.maximumBlockSize, spec.numChannels};
    for (auto& blocker : dcBlockers)
    {
        blocker.prepare(dcSpec);
        blocker.setType(juce::dsp::FirstOrderTPTFilterType::highpass);
        blocker.setCutoffFrequency(15.0f);
    }

    reset();
}


float WarmDistortion::getOversampledSampleRate() const
{
    return static_cast<float>(sampleRate) * (1 << kOversamplingFactor);
}

size_t WarmDistortion::getSafeChannel(size_t channel) const
{
    return channel < 2 ? channel : 0;
}

void WarmDistortion::process(juce::AudioBuffer<float>& buffer, const WarmDistortionParams& params)
{
    const int numOriginalSamples = buffer.getNumSamples();
    if (numOriginalSamples == 0 || buffer.getNumChannels() == 0)
        return;
    
    const int safeSamples = juce::jmin(numOriginalSamples, static_cast<int>(blockDriveValues.size()));
    if (safeSamples == 0)
        return;

    volumeCompensationEnabled = params.volumeCompensation;
    smoothedDrive.setTargetValue(juce::jlimit(kMinDrive, kMaxDrive, params.drive));
    const bool isSmoothingDrive = smoothedDrive.isSmoothing();
    for (int i = 0; i < safeSamples; ++i)
    {
        blockDriveValues[static_cast<size_t>(i)] = juce::jlimit(kMinDrive, kMaxDrive, smoothedDrive.getNextValue());
    }
    driveAmount = blockDriveValues[static_cast<size_t>(safeSamples - 1)];

    if (driveAmount < kMinDriveThreshold && !isSmoothingDrive)
        return;

    if (!oversampler)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    const int oversamplingRatio = 1 << kOversamplingFactor;

    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        float* channelData = oversampledBlock.getChannelPointer(channel);
        const int numSamples = static_cast<int>(oversampledBlock.getNumSamples());

        for (int i = 0; i < numSamples; ++i)
        {
            float s = channelData[i];
            
            float currentDrive = driveAmount;
            if (isSmoothingDrive)
            {
                size_t originalSampleIndex = static_cast<size_t>(i / oversamplingRatio);
                currentDrive = blockDriveValues[originalSampleIndex];
            }

            addDenormalizationNoise(s, static_cast<int>(channel));
            applyWowAndFlutter(s, currentDrive, static_cast<int>(channel));
            applyDriveDependentFiltering(s, currentDrive, static_cast<int>(channel));
            applyHighFrequencyExciter(s, currentDrive, static_cast<int>(channel));
            s = applySaturation(s, currentDrive, static_cast<int>(channel));
            addAnalogNoise(s, currentDrive, static_cast<int>(channel));
            s = dcBlockers[getSafeChannel(channel)].processSample(0, s);

            channelData[i] = s;
        }
    }

    oversampler->processSamplesDown(block);

    // After downsampling, apply final clipping/compensation
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        float* channelData = block.getChannelPointer(channel);

        for (size_t i = 0; i < static_cast<size_t>(safeSamples); ++i)
        {
            float currentDrive = isSmoothingDrive ? blockDriveValues[i] : driveAmount;
            currentDrive = juce::jlimit(kMinDrive, kMaxDrive, currentDrive);
            
            if (currentDrive > kLowDriveThreshold)
            {
                const float finalCompensation = juce::jmap(currentDrive, kLowDriveThreshold, kMaxDrive, 
                                                           kCompensationMax, kFinalCompensationMin);
                channelData[i] = std::tanh(channelData[i] * kFinalClipperGain) * kFinalClipperOutput * finalCompensation;
            }
        }
    }

    if (volumeCompensationEnabled)
    {
        const float newCompensation = calculateVolumeCompensation(driveAmount);
        compensationGain.setTargetValue(newCompensation);

        if (compensationGain.isSmoothing())
        {
            for (int i = 0; i < safeSamples; ++i)
            {
                const float gain = compensationGain.getNextValue();
                for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                {
                    block.getChannelPointer(channel)[i] *= gain;
                }
            }
        }
        else
        {
            const float gain = compensationGain.getTargetValue();
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
            {
                juce::FloatVectorOperations::multiply(
                    block.getChannelPointer(channel),
                    gain,
                    safeSamples);
            }
        }
    }
    else
    {
        compensationGain.skip(safeSamples);
    }
    
    // Discard any remaining samples if buffer was larger
    if (numOriginalSamples > safeSamples)
    {
        const int extra = numOriginalSamples - safeSamples;
        smoothedDrive.skip(extra);
        if (!volumeCompensationEnabled) compensationGain.skip(extra);
    }
}

float WarmDistortion::applySaturation(float input, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));

    const float hysteresisEffect = hysteresisState[safeChannel] * kHysteresisAmount * drive;
    const float inputWithHysteresis = input + hysteresisEffect;

    const float output = tapeSaturation(inputWithHysteresis, drive);

    hysteresisState[safeChannel] = output * kHysteresisFeedback + input * kHysteresisInputScale;

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
        const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
        sample += (noiseGenerator[safeChannel].nextFloat() - kDenormNoiseOffset) * kDenormNoiseLevel;
    }
}

void WarmDistortion::addAnalogNoise(float &sample, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));

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

    if (std::abs(sample) > 1.0e-6f)
    {
        if (std::abs(sample) > kNoiseFloorThreshold || drive > kHighDriveThreshold)
            sample += pink * noiseLevel;
    }
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
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));

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
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));

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

    const size_t readPos1 = static_cast<size_t>(readPosFloat) % kWowFlutterBufferSize;
    const size_t readPos2 = (readPos1 + 1) % kWowFlutterBufferSize;

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
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));

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
} // namespace synthortion::dsp