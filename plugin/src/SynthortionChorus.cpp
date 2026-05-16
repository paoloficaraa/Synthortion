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
        
        // Advance LFO phase
        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        float voiceOutL = 0.0f;
        float voiceOutR = 0.0f;

        // Multiple voices with phase offsets
        for (int v = 0; v < numVoices; ++v)
        {
            // Spread voices evenly across the LFO phase
            float phaseOffset = v * (juce::MathConstants<float>::twoPi / numVoices);
            
            // Left and right channels get different phase for stereo width
            float lfoValL = std::sin(lfoPhase + phaseOffset);
            float lfoValR = std::sin(lfoPhase + phaseOffset + juce::MathConstants<float>::pi * 0.5f); // 90 deg offset for right

            float delayTimeSamplesL = baseSamples + (lfoValL * interpolatedDepth * maxModSamples);
            float delayTimeSamplesR = baseSamples + (lfoValR * interpolatedDepth * maxModSamples);

            // Pop samples for each voice without advancing the read pointer
            voiceOutL += delayLine.popSample(0, delayTimeSamplesL, false);
            if (rightData != nullptr)
            {
                voiceOutR += delayLine.popSample(1, delayTimeSamplesR, false);
            }
        }
        
        // Average the voices to match volume
        voiceOutL /= numVoices;
        voiceOutR /= numVoices;

        // Main input samples
        const float inL = leftData[i];
        const float inR = rightData != nullptr ? rightData[i] : 0.0f;

        // Feedback path
        const float saturatedL = std::tanh(voiceOutL * 1.5f);
        const float filteredL = feedbackFilter[0].processSample(saturatedL);
        delayLine.pushSample(0, inL + (filteredL * 0.3f)); // Advance write/read pointers for Left

        // Constant power panning for wet/dry
        const float dryGain = std::cos(mix * juce::MathConstants<float>::halfPi);
        const float wetGain = std::sin(mix * juce::MathConstants<float>::halfPi);

        leftData[i] = (inL * dryGain) + (filteredL * wetGain);

        if (rightData != nullptr)
        {
            const float saturatedR = std::tanh(voiceOutR * 1.5f);
            const float filteredR = feedbackFilter[1].processSample(saturatedR);
            delayLine.pushSample(1, inR + (filteredR * 0.3f)); // Advance write/read pointers for Right
            rightData[i] = (inR * dryGain) + (filteredR * wetGain);
        }
    }
}
