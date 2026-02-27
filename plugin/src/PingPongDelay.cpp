#include "Synthortion/PingPongDelay.h"

PingPongDelay::PingPongDelay()
{
    smoothedDelayTime.setCurrentAndTargetValue(kDefaultDelayTimeMs);
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedFeedback.setCurrentAndTargetValue(kDefaultFeedback);
}

void PingPongDelay::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    const int maxDelaySamples = static_cast<int>(sampleRate * kMaxDelaySeconds);
    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(spec);

    dampingFilterLeft.prepare(spec);
    dampingFilterRight.prepare(spec);
    updateDampingFilters();

    dryWetMixer.prepare(spec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);

    smoothedDelayTime.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedMix.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedFeedback.reset(sampleRate, kSmoothingTimeSeconds);

    reset();
}

void PingPongDelay::process(juce::AudioBuffer<float> &buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 2 || numSamples == 0)
        return;

    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float>(buffer));

    const float maxDelaySamples = static_cast<float>(delayLine.getMaximumDelayInSamples()) - kDelaySamplesSafetyMargin;

    for (int i = 0; i < numSamples; ++i)
    {
        const float currentDelayMs = smoothedDelayTime.getNextValue();
        const float currentFeedback = smoothedFeedback.getNextValue();
        smoothedMix.getNextValue();

        const float delaySamples = juce::jlimit(
            kMinDelaySamples,
            maxDelaySamples,
            currentDelayMs * static_cast<float>(sampleRate) * kMsToSeconds
        );

        delayLine.setDelay(delaySamples);

        const float delayedLeft = delayLine.popSample(0);
        const float delayedRight = delayLine.popSample(1);

        const float feedbackLeft = dampingFilterLeft.processSample(delayedLeft);
        const float feedbackRight = dampingFilterRight.processSample(delayedRight);

        const float leftInput = leftChannel[i] + (feedbackRight * currentFeedback);
        const float rightInput = rightChannel[i] + (feedbackLeft * currentFeedback);

        delayLine.pushSample(0, leftInput);
        delayLine.pushSample(1, rightInput);

        leftChannel[i] = delayedLeft;
        rightChannel[i] = delayedRight;
    }

    dryWetMixer.setWetMixProportion(smoothedMix.getCurrentValue());
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
}

void PingPongDelay::reset()
{
    delayLine.reset();
    dampingFilterLeft.reset();
    dampingFilterRight.reset();
    dryWetMixer.reset();
}

void PingPongDelay::setDelayTime(float timeMs)
{
    delayTimeMs = juce::jlimit(kMinDelayTimeMs, kMaxDelayTimeMs, timeMs);
    smoothedDelayTime.setTargetValue(delayTimeMs);
}

void PingPongDelay::setDelayMix(float mix)
{
    delayMix = juce::jlimit(kMinMix, kMaxMix, mix);
    smoothedMix.setTargetValue(delayMix);
}

void PingPongDelay::setFeedback(float fb)
{
    feedback = juce::jlimit(kMinFeedback, kMaxFeedback, fb);
    smoothedFeedback.setTargetValue(feedback);
}

void PingPongDelay::setDampingFrequency(float frequency)
{
    dampingFrequency = juce::jlimit(1000.0f, 20000.0f, frequency);
    updateDampingFilters();
}

void PingPongDelay::updateDampingFilters()
{
    if (sampleRate <= 0.0)
        return;

    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate,
        dampingFrequency
    );

    dampingFilterLeft.coefficients = coefficients;
    dampingFilterRight.coefficients = coefficients;
}