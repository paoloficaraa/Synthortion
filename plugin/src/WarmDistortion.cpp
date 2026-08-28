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

    dampingState.fill(0.0f);
    exciterHighpass.fill(0.0f);
    wowPhase.fill(0.0f);
    flutterPhase.fill(0.0f);
    wowFlutterWritePos.fill(0);

    for (auto& channelState : pinkNoiseState)
        channelState.fill(0.0f);

    for (auto& buffer : wowFlutterBuffer)
        buffer.fill(0.0f);

    for (auto& blocker : dcBlockers)
        blocker.reset();

    compensationGain.setCurrentAndTargetValue(1.0f);
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
    const size_t maxBlock = juce::jmax(static_cast<size_t>(spec.maximumBlockSize), size_t(8192));

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        kOversamplingFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    oversampler->initProcessing(maxBlock);

    smoothedDrive.reset(sampleRate, 0.05);
    smoothedDrive.setCurrentAndTargetValue(0.0f);

    compensationGain.reset(sampleRate, kCompensationSmoothingTime);
    compensationGain.setCurrentAndTargetValue(1.0f);

    blockDriveValues.resize(maxBlock);

    const double oversampledRate = spec.sampleRate * (1 << kOversamplingFactor);
    const auto oversampledBlockSize = static_cast<juce::uint32>(maxBlock * (1 << kOversamplingFactor));
    juce::dsp::ProcessSpec dcSpec{ oversampledRate, oversampledBlockSize, spec.numChannels };
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

    if (!oversampler)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    const int oversamplingRatio = 1 << kOversamplingFactor;

    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        float* channelData = oversampledBlock.getChannelPointer(channel);
        const int numSamples = static_cast<int>(oversampledBlock.getNumSamples());
        const int safeCh = static_cast<int>(getSafeChannel(channel));

        for (int i = 0; i < numSamples; ++i)
        {
            float s = channelData[i];

            float currentDrive = driveAmount;
            if (isSmoothingDrive)
            {
                size_t originalSampleIndex = static_cast<size_t>(i / oversamplingRatio);
                currentDrive = blockDriveValues[originalSampleIndex];
            }

            addDenormalizationNoise(s, safeCh);
            applyWowAndFlutter(s, currentDrive, safeCh);
            applyHighFrequencyExciter(s, currentDrive, safeCh);
            s = applySaturation(s, currentDrive, safeCh);
            applyDriveDependentFiltering(s, currentDrive, safeCh);
            addAnalogNoise(s, currentDrive, safeCh);
            s = dcBlockers[static_cast<size_t>(safeCh)].processSample(0, s);

            channelData[i] = s;
        }
    }

    oversampler->processSamplesDown(block);

    const float targetCompensation = volumeCompensationEnabled ? calculateVolumeCompensation(driveAmount) : 1.0f;
    compensationGain.setTargetValue(targetCompensation);

    if (volumeCompensationEnabled || compensationGain.isSmoothing())
    {
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
        compensationGain.setCurrentAndTargetValue(1.0f);
    }

    if (numOriginalSamples > safeSamples)
    {
        const int extra = numOriginalSamples - safeSamples;
        smoothedDrive.skip(extra);
        compensationGain.skip(extra);
    }
}

float WarmDistortion::applySaturation(float input, float drive, int /*channel*/)
{
    const float inputGain = calculateInputGain(drive);
    const float dynamicBias = calculateDynamicBias(drive);
    const float x = input * inputGain;
    return asymmetricTanh(x, dynamicBias);
}

void WarmDistortion::applyDriveDependentFiltering(float& sample, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
    const float oversampledSampleRate = getOversampledSampleRate();
    const float cutoffFreq = calculateDampingFrequency(drive);

    const float alpha = std::min(kMaxFilterAlpha,
                                 1.0f - std::exp(-juce::MathConstants<float>::twoPi * cutoffFreq / oversampledSampleRate));

    dampingState[safeChannel] += alpha * (sample - dampingState[safeChannel]);
    sample = dampingState[safeChannel];
}

void WarmDistortion::applyHighFrequencyExciter(float &sample, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
    const float oversampledSampleRate = getOversampledSampleRate();
    const float alpha = std::min(kMaxFilterAlpha,
                                 1.0f - std::exp(-juce::MathConstants<float>::twoPi * kExciterFrequency / oversampledSampleRate));

    const float highFreqSignal = sample - exciterHighpass[safeChannel];
    exciterHighpass[safeChannel] += alpha * highFreqSignal;

    const float gateDrive = calculateGateDrive(drive);
    if (gateDrive <= 0.0f)
        return;

    const float drivenHigh = highFreqSignal * (1.0f + kExciterHarmonicDrive * gateDrive);

    float excitedSignal;
    if (drivenHigh >= 0.0f)
    {
        excitedSignal = std::tanh(drivenHigh * kExciterPositiveSaturation);
    }
    else
    {
        excitedSignal = std::tanh(drivenHigh * kExciterNegativeSaturation) * kExciterNegativeAsymmetry;
    }

    const float secondHarmonic = std::sin(highFreqSignal * juce::MathConstants<float>::twoPi) * 0.1f * gateDrive;
    excitedSignal += secondHarmonic;

    const float exciterAmount = kExciterMixAmount * gateDrive * gateDrive;
    sample += excitedSignal * exciterAmount;
}

void WarmDistortion::applyWowAndFlutter(float& sample, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
    const float gateDrive = calculateGateDrive(drive);

    if (gateDrive <= 0.0f)
    {
        wowFlutterBuffer[safeChannel][wowFlutterWritePos[safeChannel]] = sample;
        wowFlutterWritePos[safeChannel] = (wowFlutterWritePos[safeChannel] + 1) % kWowFlutterBufferSize;
        return;
    }

    const float oversampledSampleRate = getOversampledSampleRate();

    wowFlutterBuffer[safeChannel][wowFlutterWritePos[safeChannel]] = sample;

    const float wowFreqRadians = juce::MathConstants<float>::twoPi * kWowFrequency / oversampledSampleRate;
    wowPhase[safeChannel] += wowFreqRadians;

    if (wowPhase[safeChannel] > juce::MathConstants<float>::twoPi)
        wowPhase[safeChannel] -= juce::MathConstants<float>::twoPi;

    const float wowModulation = std::sin(wowPhase[safeChannel]) * kWowDepthMax * gateDrive;

    const float flutterFreqRadians = juce::MathConstants<float>::twoPi * kFlutterFrequency / oversampledSampleRate;
    flutterPhase[safeChannel] += flutterFreqRadians;

    if (flutterPhase[safeChannel] > juce::MathConstants<float>::twoPi)
        flutterPhase[safeChannel] -= juce::MathConstants<float>::twoPi;

    const float flutterModulation = std::sin(flutterPhase[safeChannel]) * kFlutterDepthMax * gateDrive;

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

    const float mixAmount = gateDrive * kWowFlutterMixAmount;
    sample = sample * (1.0f - mixAmount) + interpolatedSample * mixAmount;

    wowFlutterWritePos[safeChannel] = (wowFlutterWritePos[safeChannel] + 1) % kWowFlutterBufferSize;
}

void WarmDistortion::addAnalogNoise(float &sample, float drive, int channel)
{
    const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
    const float gateDrive = calculateGateDrive(drive);

    if (gateDrive <= 0.0f || std::abs(sample) <= 1.0e-6f)
        return;

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

    const float tapeNoise = kTapeModulationNoise * (1.0f + gateDrive * kTapeNoiseDriveScale);
    const float biasNoise = kThermalNoiseBase * kBiasNoiseScale;
    const float noiseLevel = (tapeNoise + biasNoise) * kNoiseGlobalScale * gateDrive;

    sample += pink * noiseLevel;
}

void WarmDistortion::addDenormalizationNoise(float &sample, int channel)
{
    if (std::abs(sample) < kDenormThreshold)
    {
        const size_t safeChannel = getSafeChannel(static_cast<size_t>(channel));
        sample += (noiseGenerator[safeChannel].nextFloat() - kDenormNoiseOffset) * kDenormNoiseLevel;
    }
}

} // namespace synthortion::dsp
