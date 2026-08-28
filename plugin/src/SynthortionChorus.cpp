#include "Synthortion/SynthortionChorus.h"

namespace synthortion::dsp {

void SynthortionChorus::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (spec.sampleRate > 0.0) ? spec.sampleRate : 44100.0;

    baseDelaySamples = calculateBaseDelaySamples(sampleRate);
    depthSamples = calculateDepthSamples(sampleRate);
    normalizedGain = calculateNormalizedGain();

    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.1));
    delayLine.prepare(spec);

    // Prepare 4th-order Linkwitz-Riley crossover filter
    crossoverFilter.prepare(spec);
    crossoverFilter.setCutoffFrequency(kCrossoverFreqHz);

    smoothedMix.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedMix.setCurrentAndTargetValue(0.0f);

    smoothedWidth.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedWidth.setCurrentAndTargetValue(0.5f);

    reset();
}

void SynthortionChorus::reset() noexcept
{
    delayLine.reset();
    crossoverFilter.reset();
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedWidth.setCurrentAndTargetValue(0.5f);
    lfoPhases.fill(0.0f);
}

void SynthortionChorus::setChorusMix(float mix)
{
    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
}

void SynthortionChorus::process(juce::AudioBuffer<float>& buffer, const ChorusParams& params)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0) return;

    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, params.mix));
    smoothedWidth.setTargetValue(juce::jlimit(0.0f, 1.0f, params.width));

    if (smoothedMix.getCurrentValue() <= 0.0f && !smoothedMix.isSmoothing() && params.mix <= 0.0f)
        return;

    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    const float phaseInc0 = juce::MathConstants<float>::twoPi * kLfo1FreqHz / static_cast<float>(sampleRate);
    const float phaseInc1 = juce::MathConstants<float>::twoPi * kLfo2FreqHz / static_cast<float>(sampleRate);
    const float phaseInc2 = juce::MathConstants<float>::twoPi * kLfo3FreqHz / static_cast<float>(sampleRate);

    const float offset0 = getVoicePhaseOffsetRad(0);
    const float offset1 = getVoicePhaseOffsetRad(1);
    const float offset2 = getVoicePhaseOffsetRad(2);

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = smoothedMix.getNextValue();
        const float width = smoothedWidth.getNextValue();
        const float stereoPhaseOffsetRad = calculateStereoPhaseOffsetRad(width);

        // Advance decoupled LFO phases
        lfoPhases[0] += phaseInc0;
        if (lfoPhases[0] >= juce::MathConstants<float>::twoPi) lfoPhases[0] -= juce::MathConstants<float>::twoPi;

        lfoPhases[1] += phaseInc1;
        if (lfoPhases[1] >= juce::MathConstants<float>::twoPi) lfoPhases[1] -= juce::MathConstants<float>::twoPi;

        lfoPhases[2] += phaseInc2;
        if (lfoPhases[2] >= juce::MathConstants<float>::twoPi) lfoPhases[2] -= juce::MathConstants<float>::twoPi;

        const float inputL = leftData[i];
        const float inputR = (rightData != nullptr) ? rightData[i] : 0.0f;

        // 4th-order Linkwitz-Riley crossover splitting at 320 Hz
        float lowL = 0.0f, highL = 0.0f;
        crossoverFilter.processSample(0, inputL, lowL, highL);

        float lowR = 0.0f, highR = 0.0f;
        if (rightData != nullptr)
            crossoverFilter.processSample(1, inputR, lowR, highR);
        // Mono low-end summing for low-frequency preservation
        const float lowMono = (rightData != nullptr) ? (0.5f * (lowL + lowR)) : lowL;

        // Push high-frequency audio to delay lines
        delayLine.pushSample(0, highL);
        if (rightData != nullptr)
            delayLine.pushSample(1, highR);

        // Voice phases (Left channel)
        const float phiL0 = lfoPhases[0] + offset0;
        const float phiL1 = lfoPhases[1] + offset1;
        const float phiL2 = lfoPhases[2] + offset2;

        const float delayL0 = baseDelaySamples + (std::sin(phiL0) * depthSamples);
        const float delayL1 = baseDelaySamples + (std::sin(phiL1) * depthSamples);
        const float delayL2 = baseDelaySamples + (std::sin(phiL2) * depthSamples);

        // Pop 3 Left channel taps
        float voiceSumL = 0.0f;
        voiceSumL += delayLine.popSample(0, delayL0, false);
        voiceSumL += delayLine.popSample(0, delayL1, false);
        voiceSumL += delayLine.popSample(0, delayL2, true);

        const float wetHighL = voiceSumL * normalizedGain;
        const float wetL = lowMono + wetHighL;
        leftData[i] = (1.0f - mix) * inputL + (mix * wetL);

        if (rightData != nullptr)
        {
            // Voice phases (Right channel with stereo phase offset)
            const float phiR0 = phiL0 + stereoPhaseOffsetRad;
            const float phiR1 = phiL1 + stereoPhaseOffsetRad;
            const float phiR2 = phiL2 + stereoPhaseOffsetRad;

            const float delayR0 = baseDelaySamples + (std::sin(phiR0) * depthSamples);
            const float delayR1 = baseDelaySamples + (std::sin(phiR1) * depthSamples);
            const float delayR2 = baseDelaySamples + (std::sin(phiR2) * depthSamples);

            // Pop 3 Right channel taps
            float voiceSumR = 0.0f;
            voiceSumR += delayLine.popSample(1, delayR0, false);
            voiceSumR += delayLine.popSample(1, delayR1, false);
            voiceSumR += delayLine.popSample(1, delayR2, true);

            const float wetHighR = voiceSumR * normalizedGain;
            const float wetR = lowMono + wetHighR;
            rightData[i] = (1.0f - mix) * inputR + (mix * wetR);
        }
    }
}

} // namespace synthortion::dsp
