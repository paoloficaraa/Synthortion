#include "Synthortion/SynthortionChorus.h"

void SynthortionChorus::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    baseDelaySamples = static_cast<float>(sampleRate) * (kDelayMs / 1000.0f);
    depthSamples = static_cast<float>(sampleRate) * (kDepthMs / 1000.0f);
    stereoPhaseOffsetRad = kStereoPhaseOffsetDeg * (juce::MathConstants<float>::pi / 180.0f);

    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.1));
    delayLine.prepare(spec);

    // Prepare Crossover: Two filters per channel to guarantee flat phase sum
    for (int i = 0; i < 2; ++i)
    {
        crossoverLP[i].prepare(spec);
        crossoverLP[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        crossoverLP[i].setCutoffFrequency(kCrossoverFreq);

        crossoverHP[i].prepare(spec);
        crossoverHP[i].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        crossoverHP[i].setCutoffFrequency(kCrossoverFreq);
    }

    smoothedMix.reset(sampleRate, 0.05);
    reset();
}

void SynthortionChorus::reset()
{
    delayLine.reset();
    for (int i = 0; i < 2; ++i)
    {
        crossoverLP[i].reset();
        crossoverHP[i].reset();
    }
    lfo1Phase = 0.0f;
    lfo2Phase = 0.0f;
    lfo3Phase = 0.0f;
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

    const float phaseInc1 = juce::MathConstants<float>::twoPi * kLfo1FreqHz / static_cast<float>(sampleRate);
    const float phaseInc2 = juce::MathConstants<float>::twoPi * kLfo2FreqHz / static_cast<float>(sampleRate);
    const float phaseInc3 = juce::MathConstants<float>::twoPi * kLfo3FreqHz / static_cast<float>(sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = smoothedMix.getNextValue();

        // Advance LFO phases
        lfo1Phase += phaseInc1;
        if (lfo1Phase >= juce::MathConstants<float>::twoPi) lfo1Phase -= juce::MathConstants<float>::twoPi;

        lfo2Phase += phaseInc2;
        if (lfo2Phase >= juce::MathConstants<float>::twoPi) lfo2Phase -= juce::MathConstants<float>::twoPi;

        lfo3Phase += phaseInc3;
        if (lfo3Phase >= juce::MathConstants<float>::twoPi) lfo3Phase -= juce::MathConstants<float>::twoPi;

        // Save dry signal intact
        const float inputL = leftData[i];
        const float inputR = rightData != nullptr ? rightData[i] : 0.0f;

        // Highpass ONLY for the delay input (Process HF)
        const float highL = crossoverHP[0].processSample(0, inputL);
        const float highR = rightData != nullptr ? crossoverHP[1].processSample(1, inputR) : 0.0f;

        // Keep LP filters in sync (optional, not used for output)
        crossoverLP[0].processSample(0, inputL);
        if (rightData != nullptr) crossoverLP[1].processSample(1, inputR);

        // Push sample into delay line BEFORE reading
        delayLine.pushSample(0, highL);
        if (rightData != nullptr) delayLine.pushSample(1, highR);

        // Pop: Read the 3 voices
        float voiceOutL = 0.0f, voiceOutR = 0.0f;

        // Voice 1 (Read without advancing delay: false)
        float delayL1 = baseDelaySamples + (std::sin(lfo1Phase) * depthSamples);
        float delayR1 = baseDelaySamples + (std::sin(lfo1Phase + stereoPhaseOffsetRad) * depthSamples);
        voiceOutL += delayLine.popSample(0, delayL1, false);
        if (rightData != nullptr) voiceOutR += delayLine.popSample(1, delayR1, false);

        // Voice 2 (Read without advancing delay: false)
        float delayL2 = baseDelaySamples + (std::sin(lfo2Phase) * depthSamples);
        float delayR2 = baseDelaySamples + (std::sin(lfo2Phase + stereoPhaseOffsetRad) * depthSamples);
        voiceOutL += delayLine.popSample(0, delayL2, false);
        if (rightData != nullptr) voiceOutR += delayLine.popSample(1, delayR2, false);

        // Voice 3 (Read WITH delay advance: TRUE) - Eliminates clicks
        float delayL3 = baseDelaySamples + (std::sin(lfo3Phase) * depthSamples);
        float delayR3 = baseDelaySamples + (std::sin(lfo3Phase + stereoPhaseOffsetRad) * depthSamples);
        voiceOutL += delayLine.popSample(0, delayL3, true);
        if (rightData != nullptr) voiceOutR += delayLine.popSample(1, delayR3, true);

        // Average to maintain volume
        voiceOutL /= 3.0f;
        voiceOutR /= 3.0f;

        // Final Mix: Full DRY signal + (Filtered WET signal * Mix)
        // Restores full high-frequency brilliance and preserves solid lows.
        leftData[i] = inputL + (voiceOutL * mix);
        if (rightData != nullptr)
        {
            rightData[i] = inputR + (voiceOutR * mix);
        }
    }
}
