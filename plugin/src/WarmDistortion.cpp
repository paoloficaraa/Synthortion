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

    // Reset analog modeling state
    tubeBiasDrift = 0.0f;
    tubeWarmupFactor = 0.0f;
    samplesSinceReset = 0;
    flickerNoiseAccumulator = 0.0f;

    // Reset filter states
    preEmphState = 0.0f;
    postFilterState = 0.0f;
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
                // Pre-filtering (pre-emphasis) prima della saturazione
                applyDriveDependentFiltering(samples[i], driveAmount);

                samples[i] = applySaturation(samples[i], driveAmount);
                samples[i] = applyBitCrush(samples[i]);

                // Add realistic analog noise
                addAnalogNoise(samples[i], driveAmount);
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

            // Apply drive-dependent filtering prima della saturazione
            applyDriveDependentFiltering(inputSample, driveAmount);

            // Apply saturation
            float saturatedSample = applySaturation(inputSample, driveAmount);

            // Apply bit crush effect
            saturatedSample = applyBitCrush(saturatedSample);

            // Add realistic analog noise
            addAnalogNoise(saturatedSample, driveAmount);

            channelData[sampleIndex] = saturatedSample;

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
    // Advanced 12AX7 tube modeling con thermal effects
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, TUBE_DRIVE_MIN, TUBE_DRIVE_MAX);

    // Simula riscaldamento graduale del tubo (primi ~5 secondi)
    samplesSinceReset++;
    if (samplesSinceReset < sampleRate * 5.0) // 5 secondi di warmup
    {
        tubeWarmupFactor = (float)samplesSinceReset / (float)(sampleRate * 5.0);
        tubeWarmupFactor = std::pow(tubeWarmupFactor, 0.7f); // Curva non lineare
    }
    else
    {
        tubeWarmupFactor = 1.0f;
    }

    // Bias drift termico - molto lento e sottile
    if (samplesSinceReset % 1024 == 0) // Aggiorna ogni 1024 campioni
    {
        tubeBiasDrift += (noiseGenerator.nextFloat() - 0.5f) * TUBE_BIAS_DRIFT * 0.001f;
        tubeBiasDrift = juce::jlimit(-TUBE_BIAS_DRIFT, TUBE_BIAS_DRIFT, tubeBiasDrift);
    }

    // Applica il bias drift e warmup
    float driven = input * driveMapped * tubeWarmupFactor;
    driven += tubeBiasDrift; // Bias offset sottile

    // Modello realistico 12AX7: curve di saturazione asimmetriche
    float output;

    if (driven >= 0.0f)
    {
        // Saturazione positiva: più dolce, simula grid current limiting
        if (driven < TUBE_GRID_CURRENT)
        {
            // Regione lineare/dolce
            output = driven * (1.0f + driven * 0.1f);
        }
        else
        {
            // Grid current limiting - saturazione morbida
            float excess = driven - TUBE_GRID_CURRENT;
            output = TUBE_GRID_CURRENT * (1.0f + TUBE_GRID_CURRENT * 0.1f);
            output += excess / (1.0f + excess * TUBE_CATHODE_COMPRESSION);
        }
    }
    else
    {
        // Saturazione negativa: più dura, simula plate current cutoff
        float abs_driven = -driven;

        if (abs_driven < TUBE_PLATE_KNEE)
        {
            // Regione semi-lineare
            output = driven * (1.0f + abs_driven * 0.05f);
        }
        else
        {
            // Hard limiting della placca
            float excess = abs_driven - TUBE_PLATE_KNEE;
            output = -TUBE_PLATE_KNEE * (1.0f + TUBE_PLATE_KNEE * 0.05f);
            output -= excess / (1.0f + excess * 3.0f); // Limiting più duro
        }
    }

    // Aggiunge contenuto armonico sottile (simula distorsione della griglia)
    float harmonic = std::sin(driven * 3.14159f) * TUBE_HARMONIC_CONTENT * drive;
    output += harmonic * input * input; // Solo per segnali più forti

    // Asimmetria variabile basata sul drive
    float asymmetry = TUBE_ASYMMETRY_FACTOR * drive;
    if (output > 0.0f)
        output *= (1.0f - asymmetry);
    else
        output *= (1.0f + asymmetry);

    // Clamp finale per evitare valori eccessivi
    return juce::jlimit(-1.2f, 1.0f, output);
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

void WarmDistortion::addAnalogNoise(float &sample, float drive)
{
    float noiseLevel = 0.0f;

    switch (saturationType)
    {
    case SaturationType::TUBE:
    {
        // Tube hiss che aumenta con il drive
        float tubeHiss = TUBE_HISS_BASE * (1.0f + drive * 2.0f);

        // 1/f flicker noise tipico dei tubi
        flickerNoiseAccumulator = flickerNoiseAccumulator * 0.99f +
                                  (noiseGenerator.nextFloat() - 0.5f) * TUBE_FLICKER_NOISE;

        // Thermal noise del catodo (aumenta con warmup)
        float thermalNoise = THERMAL_NOISE_BASE * tubeWarmupFactor;

        noiseLevel = tubeHiss + std::abs(flickerNoiseAccumulator) + thermalNoise;
        break;
    }
    case SaturationType::TAPE:
    {
        // Tape modulation noise
        float tapeNoise = TAPE_MODULATION_NOISE * (1.0f + drive * 1.5f);

        // High frequency bias noise
        float biasNoise = THERMAL_NOISE_BASE * 0.5f;

        noiseLevel = tapeNoise + biasNoise;
        break;
    }
    case SaturationType::SMOOTH:
    {
        // Minimal digital noise per la modalità smooth
        noiseLevel = THERMAL_NOISE_BASE * 0.3f;
        break;
    }
    }

    // Applica il rumore solo se il segnale è sopra una certa soglia
    if (std::abs(sample) > 0.001f || drive > 0.3f)
    {
        float noise = (noiseGenerator.nextFloat() - 0.5f) * noiseLevel;
        sample += noise;
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

void WarmDistortion::applyDriveDependentFiltering(float &sample, float drive)
{
    // Pre-emphasis: aumenta gli alti con il drive (simula condensatori che saturano)
    // Frequency aumenta da 1kHz a ~4kHz con drive max
    float preEmphFreq = PREEMPH_BASE_FREQ * (1.0f + drive * PREEMPH_DRIVE_FACTOR);
    float preEmphAlpha = 1.0f - std::exp(-2.0f * 3.14159f * preEmphFreq / (float)sampleRate);

    // High-pass filter semplice per pre-emphasis
    float preEmphOutput = sample - preEmphState;
    preEmphState += preEmphAlpha * preEmphOutput;

    // Mix con l'originale - più drive = più pre-emphasis
    float preEmphAmount = drive * 0.3f; // Massimo 30% di pre-emphasis
    sample = sample * (1.0f - preEmphAmount) + preEmphOutput * preEmphAmount;

    // Post-filtering: taglia gli alti con drive alto (simula trasformatori saturi)
    // Frequency diminuisce da 8kHz a ~2kHz con drive max
    float postFilterFreq = POSTFILTER_BASE_FREQ * (1.0f - drive * POSTFILTER_DRIVE_FACTOR);
    float postFilterAlpha = 1.0f - std::exp(-2.0f * 3.14159f * postFilterFreq / (float)sampleRate);

    // Low-pass filter semplice per post-filtering
    postFilterState += postFilterAlpha * (sample - postFilterState);

    // Mix con l'originale - più drive = più filtering
    float postFilterAmount = drive * 0.4f; // Massimo 40% di post-filtering
    sample = sample * (1.0f - postFilterAmount) + postFilterState * postFilterAmount;
}