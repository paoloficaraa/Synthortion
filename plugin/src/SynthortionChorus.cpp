#include "Synthortion/SynthortionChorus.h"

void SynthortionChorus::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    // Compute base samples using targetDelayMs
    baseSamples = sampleRate * (targetDelayMs / 1000.0f);

    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.1)); // 100ms max

    // Prepare crossover filters (highpass)
    crossoverFilter[0].prepare(spec);
    crossoverFilter[0].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    crossoverFilter[0].setCutoffFrequency(crossoverFreq);
    crossoverFilter[1].prepare(spec);
    crossoverFilter[1].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    crossoverFilter[1].setCutoffFrequency(crossoverFreq);

    feedbackFilter[0].prepare(spec);
    feedbackFilter[0].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4000.0f); // 6dB LPF for warmth
    feedbackFilter[1].prepare(spec);
    feedbackFilter[1].coefficients = feedbackFilter[0].coefficients;

    smoothedMix.reset(sampleRate, 0.05);
    reset();
}

void SynthortionChorus::reset()
{
    delayLine.reset();
    crossoverFilter[0].reset();
    crossoverFilter[1].reset();
    lfoPhase = 0.0f;
    interpolatedDepth = 0.0f;
    interpolatedPhaseOffsetRad = 0.0f;
}

void SynthortionChorus::setChorusMix(float mix)
{
    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
}

void SynthortionChorus::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0) return;

    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    const float maxModSamples = static_cast<float>(sampleRate) * 0.01f; // 10ms max modulation
    const float phaseIncrement = juce::MathConstants<float>::twoPi * targetRateHz / static_cast<float>(sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = smoothedMix.getNextValue();

        // Interpolate depth and phase from mix
        interpolatedDepth = mix * 0.25f; // Depth target 0.25
        interpolatedPhaseOffsetRad = mix * juce::MathConstants<float>::pi * (targetPhaseOffsetDeg / 180.0f); // 45° → π/4

        // Advance LFO phase
        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        // Split input via crossover per channel
        float lowL = 0.0f, highL = 0.0f;
        float lowR = 0.0f, highR = 0.0f;

        crossoverFilter[0].processSample(0, leftData[i], lowL, highL);
        if (rightData != nullptr)
            crossoverFilter[1].processSample(1, rightData[i], lowR, highR);

        // Process high band through chorus (fixed delay + multi sine LFO)
        float voiceOutL = 0.0f;
        float voiceOutR = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            float phaseOffset = v * (juce::MathConstants<float>::twoPi / numVoices);

            // Multi-sine LFO: A*(sin(ωt) + 0.5*sin(3ωt))
            float lfoL = std::sin(lfoPhase + phaseOffset) + 0.5f * std::sin(3.0f * (lfoPhase + phaseOffset));
            float lfoR = std::sin(lfoPhase + phaseOffset + interpolatedPhaseOffsetRad) + 0.5f * std::sin(3.0f * (lfoPhase + phaseOffset + interpolatedPhaseOffsetRad));

            float delayTimeSamplesL = baseSamples + (lfoL * interpolatedDepth * maxModSamples);
            float delayTimeSamplesR = baseSamples + (lfoR * interpolatedDepth * maxModSamples);

            voiceOutL += delayLine.popSample(0, delayTimeSamplesL, false);
            if (rightData != nullptr)
                voiceOutR += delayLine.popSample(1, delayTimeSamplesR, false);
        }

        voiceOutL /= numVoices;
        voiceOutR /= numVoices;

        // Feedback path with tanh saturation
        const float saturatedL = std::tanh(voiceOutL * 1.5f);
        const float filteredL = feedbackFilter[0].processSample(saturatedL);

        float filteredR = 0.0f;
        if (rightData != nullptr)
        {
            const float saturatedR = std::tanh(voiceOutR * 1.5f);
            filteredR = feedbackFilter[1].processSample(saturatedR);
            delayLine.pushSample(1, highR + (filteredR * 0.3f));
        }

        delayLine.pushSample(0, highL + (filteredL * 0.3f));

        // Dry/wet mixing: low stays dry; high band crossfades
        const float dryGain = std::cos(mix * juce::MathConstants<float>::halfPi);
        const float wetGain = std::sin(mix * juce::MathConstants<float>::halfPi);

        leftData[i] = lowL + (highL * dryGain) + (filteredL * wetGain);

        if (rightData != nullptr)
        {
            rightData[i] = lowR + (highR * dryGain) + (filteredR * wetGain);
        }
    }
}
