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
    for (int ch = 0; ch < 2; ch++)
    {
        noiseGenerator[ch].setSeedRandomly();
    }

    // Reset filter states
    std::fill(std::begin(preEmphState), std::end(preEmphState), 0.0f);
    std::fill(std::begin(postFilterState), std::end(postFilterState), 0.0f);
    std::fill(std::begin(exciterHighpass), std::end(exciterHighpass), 0.0f);
    std::fill(std::begin(exciterDelay), std::end(exciterDelay), 0.0f);

    // Reset gain compensation
    compensationGain.setCurrentAndTargetValue(1.0f);

    // Reset oversampler
    if (oversampler)
    {
        oversampler->reset();
    }
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
    compensationGain.reset(sampleRate, 0.1); // 50ms smoothing
    compensationGain.setCurrentAndTargetValue(1.0f);
}

void WarmDistortion::setDrive(float newDrive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, newDrive);

    // Update volume compensation if enabled
    if (volumeCompensationEnabled)
    {
        float compensation = calculateVolumeCompensation(driveAmount);
        compensationGain.setTargetValue(compensation);
    }
}

void WarmDistortion::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
    juce::dsp::AudioBlock<float> block = context.getOutputBlock();

    // Bypass for drive too low
    if (driveAmount < 0.01f)
    {
        return;
    }

    // Process always 100% wet here; global dry/wet mix is applied in the processor
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    // =====================================================

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
                addDenormalizationNoise(samples[i], channel);
            }

            // Apply saturation to each sample
            for (int i = 0; i < 4; ++i)
            {
                // Pre-filtering (pre-emphasis) prima della saturazione
                applyDriveDependentFiltering(samples[i], driveAmount, channel);

                // High-frequency exciter per croccantezza
                applyHighFrequencyExciter(samples[i], driveAmount, channel);

                samples[i] = applySaturation(samples[i], driveAmount, channel);

                // Add realistic analog noise
                addAnalogNoise(samples[i], driveAmount, channel);
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
            addDenormalizationNoise(inputSample, channel);

            // Apply drive-dependent filtering prima della saturazione
            applyDriveDependentFiltering(inputSample, driveAmount, channel);

            // High-frequency exciter per croccantezza
            applyHighFrequencyExciter(inputSample, driveAmount, channel);

            // Apply saturation
            float saturatedSample = applySaturation(inputSample, driveAmount, channel);

            // Add realistic analog noise
            addAnalogNoise(saturatedSample, driveAmount, channel);

            channelData[sampleIndex] = saturatedSample;

            ++sampleIndex;
        }
    }

    oversampler->processSamplesDown(block);

    if (driveAmount > 0.05f)
    {
        for (int channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float *channelData = block.getChannelPointer(channel);
            for (int sample = 0; sample < block.getNumSamples(); ++sample)
            {
                // Soft clipper finale più delicato per preservare il volume
                float x = channelData[sample];

                // Clipper meno aggressivo che preserva meglio l'ampiezza
                float clipped = std::tanh(x * 0.85f) * 1.15f;

                // Compensazione finale molto più conservativa per mantenere unity gain
                float finalCompensation = juce::jmap(driveAmount, 0.05f, 1.0f, 1.0f, 0.92f);
                channelData[sample] = clipped * finalCompensation;
            }
        }
    }

    // Apply volume compensation to the downsampled signal
    if (volumeCompensationEnabled)
    {
        // Pre-calculate gain values per sample to avoid double-smoothing in stereo
        std::vector<float> gainValues(block.getNumSamples());

        if (compensationGain.isSmoothing())
        {
            // Calculate gain values once for all channels
            for (int sample = 0; sample < block.getNumSamples(); ++sample)
            {
                gainValues[sample] = compensationGain.getNextValue();
            }

            // Apply to all channels using the pre-calculated values
            for (int channel = 0; channel < block.getNumChannels(); ++channel)
            {
                float *channelData = block.getChannelPointer(channel);
                for (int sample = 0; sample < block.getNumSamples(); ++sample)
                {
                    channelData[sample] *= gainValues[sample];
                }
            }
        }
        else
        {
            // Apply constant gain using optimized vector operation
            const float gain = compensationGain.getTargetValue();
            for (int channel = 0; channel < block.getNumChannels(); ++channel)
            {
                float *channelData = block.getChannelPointer(channel);
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

float WarmDistortion::applySaturation(float input, float drive, int channel)
{
    juce::ignoreUnused(channel);
    return tapeSaturation(input, drive);
}

float WarmDistortion::tapeSaturation(float input, float drive)
{
    // Tape-style saturation with soft knee compression
    float driveMapped = juce::jmap(drive, 0.0f, 1.0f, TAPE_DRIVE_MIN, TAPE_DRIVE_MAX);
    float driven = input * driveMapped;

    // Soft knee saturation characteristic of tape
    float abs_driven = std::abs(driven);
    float output;

    if (abs_driven < TAPE_KNEE_THRESHOLD)
    {
        output = driven;
    }
    else
    {
        float sign = (driven > 0.0f) ? 1.0f : -1.0f;
        float compressed = TAPE_KNEE_THRESHOLD + (abs_driven - TAPE_KNEE_THRESHOLD) /
                                                     (1.0f + (abs_driven - TAPE_KNEE_THRESHOLD) * TAPE_COMPRESSION_FACTOR);
        output = sign * compressed;
    }

    // Compensazione volume più conservativa per il tape
    float tapeVolumeCompensation = juce::jmap(drive, 0.0f, 1.0f, 0.95f, 0.80f);
    return output * tapeVolumeCompensation;
}

void WarmDistortion::addDenormalizationNoise(float &sample, int channel)
{
    if (std::abs(sample) < DENORM_THRESHOLD)
    {
        // Assicurati che l'indice del canale sia valido
        int safeChannel = juce::jlimit(0, 1, channel);
        sample += (noiseGenerator[safeChannel].nextFloat() - 0.5f) * DENORM_NOISE_LEVEL;
    }
}

void WarmDistortion::addAnalogNoise(float &sample, float drive, int channel)
{
    // Assicurati che l'indice del canale sia valido
    int safeChannel = juce::jlimit(0, 1, channel);

    // Tape modulation noise
    float tapeNoise = TAPE_MODULATION_NOISE * (1.0f + drive * 1.5f);

    // High frequency bias noise
    float biasNoise = THERMAL_NOISE_BASE * 0.5f;

    float noiseLevel = tapeNoise + biasNoise;

    // Applica il rumore solo se il segnale è sopra una certa soglia
    if (std::abs(sample) > 0.001f || drive > 0.3f)
    {
        float noise = (noiseGenerator[safeChannel].nextFloat() - 0.5f) * noiseLevel;
        sample += noise;
    }
}

float WarmDistortion::calculateVolumeCompensation(float drive) const
{
    if (drive < 0.01f)
        return 1.0f; // Nessuna compensazione per drive molto bassi

    // Tape saturation ha compressione soft, compensazione minima
    float compensationFactor = juce::jmap(drive, 0.0f, 1.0f, 1.0f, 0.85f);

    // Applica una curva più naturale che mantiene meglio il volume percepito
    float smoothedCompensation = std::pow(compensationFactor, 0.9f);

    // Range di compensazione più conservativo per evitare perdite eccessive
    return juce::jlimit(0.65f, 1.0f, smoothedCompensation);
}

void WarmDistortion::applyDriveDependentFiltering(float &sample, float drive, int channel)
{
    // Assicurati che l'indice del canale sia valido
    int safeChannel = juce::jlimit(0, 1, channel);

    // Solo se c'è drive significativo
    if (drive < 0.01f)
        return;

    // Post-filtering solo se necessario
    if (drive > 0.1f)
    {
        // Pre-emphasis POTENZIATO: aumenta gli alti con il drive per più croccantezza
        // Frequency aumenta da 1.5kHz a ~6kHz con drive max (range più ampio)
        float preEmphFreq = PREEMPH_BASE_FREQ * 1.5f * (1.0f + drive * PREEMPH_DRIVE_FACTOR * 1.2f);
        float preEmphAlpha = std::min(0.95f, 1.0f - std::exp(-2.0f * 3.14159f * preEmphFreq / (float)sampleRate));

        // High-pass filter semplice per pre-emphasis
        float preEmphOutput = sample - preEmphState[safeChannel];
        preEmphState[safeChannel] += preEmphAlpha * preEmphOutput;

        // Mix con l'originale - MIX PIÙ AGGRESSIVO per più croccantezza
        float preEmphAmount = drive * 0.5f; // Aumentato da 0.3f a 0.5f
        sample = sample * (1.0f - preEmphAmount) + preEmphOutput * preEmphAmount;

        // Post-filtering: taglia gli alti con drive alto (simula trasformatori saturi)
        float postFilterFreq = std::max(1200.0f, POSTFILTER_BASE_FREQ * (1.0f - drive * 0.5f));
        float postFilterAlpha = std::min(0.95f, 1.0f - std::exp(-2.0f * 3.14159f * postFilterFreq / (float)sampleRate));

        float postFilterInput = sample;
        postFilterState[safeChannel] += postFilterAlpha * (postFilterInput - postFilterState[safeChannel]);

        // Mix con l'originale - più drive = più filtering
        float postFilterAmount = drive * 0.6f;
        sample = sample * (1.0f - postFilterAmount) + postFilterState[safeChannel] * postFilterAmount;
    }
}

void WarmDistortion::applyHighFrequencyExciter(float &sample, float drive, int channel)
{
    // Assicurati che l'indice del canale sia valido
    int safeChannel = juce::jlimit(0, 1, channel);

    // Applica exciter solo se c'è drive sufficiente
    if (drive < 0.15f)
        return;

    // 1. Estrai le alte frequenze con un high-pass filter
    float cutoffFreq = EXCITER_HIGHPASS_FREQ * (0.8f + drive * 0.4f); // Varia da 2.4kHz a 4.2kHz
    float alpha = std::min(0.95f, 1.0f - std::exp(-2.0f * 3.14159f * cutoffFreq / (float)sampleRate));

    // High-pass filter
    float highFreqSignal = sample - exciterHighpass[safeChannel];
    exciterHighpass[safeChannel] += alpha * highFreqSignal;

    // 2. Genera armoniche delle alte frequenze con saturazione asimmetrica
    float harmonicDrive = EXCITER_HARMONIC_DRIVE * drive;
    float drivenHigh = highFreqSignal * harmonicDrive;

    // Saturazione asimmetrica che enfatizza le armoniche pari (più croccanti)
    float excitedSignal;
    if (drivenHigh >= 0.0f)
    {
        // Saturazione più dolce sui positivi
        excitedSignal = std::tanh(drivenHigh * 0.8f);
    }
    else
    {
        // Saturazione più aggressiva sui negativi per asimmetria
        excitedSignal = std::tanh(drivenHigh * 1.2f) * 0.85f;
    }

    // 3. Aggiungi seconda armonica per più "air" e presenza
    float secondHarmonic = std::sin(highFreqSignal * 6.28318f) * 0.1f * drive;
    excitedSignal += secondHarmonic;

    // 4. Mix l'exciter con il segnale originale
    float exciterAmount = EXCITER_MIX_AMOUNT * drive * drive; // Curva quadratica
    sample += excitedSignal * exciterAmount;
}