#include "Synthortion/WarmDistortion.h"

WarmDistortion::WarmDistortion()
{
    reset();
}

void WarmDistortion::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    if (oversampler)
    {
        oversampler->reset();
    }
}

void WarmDistortion::reset()
{
    noiseGenerator.setSeedRandomly();
}

void WarmDistortion::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        OVERSAMPLING_FACTOR,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    oversampler->initProcessing(spec.maximumBlockSize);

    // Initialize volume compensation smoothing
    compensationGain.reset(sampleRate, 0.05); // 50ms smoothing
    compensationGain.setCurrentAndTargetValue(1.0f);
}

void WarmDistortion::setDrive(float newDrive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, newDrive);

    // Update volume compensation if enabled
    if (volumeCompensationEnabled)
    {
        float compensation = calculateVolumeCompensation(driveAmount, saturationType);
        compensationGain.setTargetValue(compensation);
    }
}

void WarmDistortion::setSaturationType(SaturationType type)
{
    saturationType = type;

    // Update volume compensation if enabled
    if (volumeCompensationEnabled)
    {
        float compensation = calculateVolumeCompensation(driveAmount, saturationType);
        compensationGain.setTargetValue(compensation);
    }
}

void WarmDistortion::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
    juce::dsp::AudioBlock<float> block = context.getOutputBlock();

    // Process always 100% wet here; global dry/wet mix is applied in the processor
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    // Process each channel using SIMD-optimized operations where possible
    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        float *channelData = oversampledBlock.getChannelPointer(channel);
        const int numSamples = static_cast<int>(oversampledBlock.getNumSamples());

        // Process in SIMD blocks where possible (4 samples at a time)
        const int simdBlocks = numSamples / 4;
        const int remainingSamples = numSamples % 4;

        int sampleIndex = 0;

        // SIMD processing for aligned blocks
        for (int simdBlock = 0; simdBlock < simdBlocks; ++simdBlock)
        {
            // Load 4 samples
            float samples[4];
            for (int i = 0; i < 4; ++i)
            {
                samples[i] = channelData[sampleIndex + i];
                addDenormalizationNoise(samples[i]);
            }

            // Apply saturation to each sample
            for (int i = 0; i < 4; ++i)
            {
                samples[i] = applySaturation(samples[i], driveAmount);
                samples[i] = applyBitCrush(samples[i]);
            }

            // Store results back
            for (int i = 0; i < 4; ++i)
            {
                channelData[sampleIndex + i] = samples[i];
            }

            sampleIndex += 4;
        }

        // Process remaining samples individually
        for (int i = 0; i < remainingSamples; ++i)
        {
            float inputSample = channelData[sampleIndex];
            addDenormalizationNoise(inputSample);

            // Apply saturation
            float saturatedSample = applySaturation(inputSample, driveAmount);

            // Apply bit crush effect
            channelData[sampleIndex] = applyBitCrush(saturatedSample);

            ++sampleIndex;
        }
    }

    oversampler->processSamplesDown(block);

    // Apply volume compensation to the downsampled signal
    if (volumeCompensationEnabled)
    {
        for (int channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float *channelData = block.getChannelPointer(channel);

            // Use JUCE's optimized vector operations for gain application
            if (compensationGain.isSmoothing())
            {
                // Apply smoothed gain sample by sample
                for (int sample = 0; sample < block.getNumSamples(); ++sample)
                {
                    channelData[sample] *= compensationGain.getNextValue();
                }
            }
            else
            {
                // Apply constant gain using optimized vector operation
                const float gain = compensationGain.getTargetValue();
                juce::FloatVectorOperations::multiply(channelData, gain, static_cast<int>(block.getNumSamples()));
            }
        }
    }
    else
    {
        // Skip compensation smoothing if disabled
        compensationGain.skip(static_cast<int>(block.getNumSamples()));
    }
}

float WarmDistortion::applySaturation(float input, float drive)
{
    switch (saturationType)
    {
    case SaturationType::SMOOTH:
        return smoothSaturation(input, drive);
    case SaturationType::TUBE:
        return tubeSaturation(input, drive);
    case SaturationType::TAPE:
        return tapeSaturation(input, drive);
    default:
        return smoothSaturation(input, drive);
    }
}

float WarmDistortion::smoothSaturation(float input, float drive)
{
    // Smooth tanh saturation (original behavior)
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, SMOOTH_DRIVE_MIN, SMOOTH_DRIVE_MAX);
    float driven = input * driveMapped;
    return std::tanh(driven);
}

float WarmDistortion::tubeSaturation(float input, float drive)
{
    // Tube-style saturation with asymmetric clipping
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, TUBE_DRIVE_MIN, TUBE_DRIVE_MAX);
    float driven = input * driveMapped;

    // Asymmetric saturation mimicking tube behavior
    if (driven > 0.0f)
    {
        // Softer saturation for positive signals
        return std::tanh(driven * TUBE_POSITIVE_FACTOR) * TUBE_POSITIVE_GAIN;
    }
    else
    {
        // Harder clipping for negative signals
        return juce::jmax(TUBE_NEGATIVE_CLAMP, driven * TUBE_NEGATIVE_FACTOR);
    }
}

float WarmDistortion::tapeSaturation(float input, float drive)
{
    // Tape-style saturation with soft knee compression
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, TAPE_DRIVE_MIN, TAPE_DRIVE_MAX);
    float driven = input * driveMapped;

    // Soft knee saturation characteristic of tape
    float abs_driven = std::abs(driven);
    if (abs_driven < TAPE_KNEE_THRESHOLD)
    {
        return driven;
    }
    else
    {
        float sign = (driven > 0.0f) ? 1.0f : -1.0f;
        float compressed = TAPE_KNEE_THRESHOLD + (abs_driven - TAPE_KNEE_THRESHOLD) /
                                                     (1.0f + (abs_driven - TAPE_KNEE_THRESHOLD) * TAPE_COMPRESSION_FACTOR);
        return sign * compressed;
    }
}

float WarmDistortion::applyBitCrush(float input)
{
    // Apply subtle bit-crush effect with fixed settings
    // Calculate quantization levels
    float levels = std::pow(2.0f, BITCRUSH_BITS) - 1.0f;

    // Quantize the signal
    float crushed = std::round(input * levels) / levels;

    // Mix with original signal
    return input * (1.0f - BITCRUSH_MIX) + crushed * BITCRUSH_MIX;
}

void WarmDistortion::addDenormalizationNoise(float &sample)
{
    if (std::abs(sample) < DENORM_THRESHOLD)
    {
        sample += noiseGenerator.nextFloat() * DENORM_NOISE_LEVEL;
    }
}

float WarmDistortion::calculateVolumeCompensation(float drive, SaturationType type) const
{
    // Calculate compensation based on drive amount and saturation type
    // Higher drive typically reduces overall volume, so we compensate accordingly

    float baseDriveCompensation = 1.0f;

    switch (type)
    {
    case SaturationType::SMOOTH:
    {
        // tanh saturation reduces volume as drive increases
        // Empirical compensation curve for smooth saturation
        float driveEffect = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 0.3f);
        baseDriveCompensation = 1.0f / juce::jmax(0.1f, driveEffect);
        break;
    }
    case SaturationType::TUBE:
    {
        // Tube saturation has asymmetric behavior, less volume reduction
        float driveEffect = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 0.5f);
        baseDriveCompensation = 1.0f / juce::jmax(0.2f, driveEffect);
        break;
    }
    case SaturationType::TAPE:
    {
        // Tape saturation has soft knee, moderate volume reduction
        float driveEffect = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 0.4f);
        baseDriveCompensation = 1.0f / juce::jmax(0.15f, driveEffect);
        break;
    }
    }

    // Limit compensation to reasonable range to avoid excessive volumes
    return juce::jlimit(0.5f, 3.0f, baseDriveCompensation);
}