#include "Synthortion/PingPongDelay.h"

PingPongDelay::PingPongDelay()
{
    smoothedDelayTime.setCurrentAndTargetValue(250.0f);
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedFeedback.setCurrentAndTargetValue(0.4f);
}

void PingPongDelay::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    delayLine.setMaximumDelayInSamples(maxDelaySamples);

    delayLine.prepare(spec);

    dampingFilterLeft.prepare(spec);
    dampingFilterRight.prepare(spec);
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 12000.0f);
    dampingFilterLeft.coefficients = coefficients;
    dampingFilterRight.coefficients = coefficients;

    dryWetMixer.prepare(spec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);

    smoothedDelayTime.reset(sampleRate, 0.05);
    smoothedMix.reset(sampleRate, 0.05);
    smoothedFeedback.reset(sampleRate, 0.05);

    reset();
}

void PingPongDelay::process(juce::AudioBuffer<float> &buffer)
{
    const int numSamples = buffer.getNumSamples();
    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float>(buffer));

    for (int i = 0; i < numSamples; ++i)
    {
        // Get smoothed parameters per-sample for smooth modulation
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float currentFeedback = smoothedFeedback.getNextValue();

        // Convert delay time to samples
        float delaySamples = (currentDelayMs * static_cast<float>(sampleRate)) / 1000.0f;
        delaySamples = juce::jlimit(1.0f, static_cast<float>(delayLine.getMaximumDelayInSamples() - 2), delaySamples);

        // Set delay for both lines
        delayLine.setDelay(delaySamples);

        // Read delayed signals
        float delayedLeft = delayLine.popSample(0);
        float delayedRight = delayLine.popSample(1);

        // Apply damping
        float feedbackLeft = dampingFilterLeft.processSample(delayedLeft);
        float feedbackRight = dampingFilterRight.processSample(delayedRight);

        // Ping-pong: left feedback goes to right, right goes to left
        float leftInput = leftChannel[i] + (feedbackRight * currentFeedback);
        float rightInput = rightChannel[i] + (feedbackLeft * currentFeedback);

        // Push to delay lines
        delayLine.pushSample(0, leftInput);
        delayLine.pushSample(1, rightInput);

        // Write delayed output
        leftChannel[i] = delayedLeft;
        rightChannel[i] = delayedRight;
    }

    // Mix dry and wet using the mixer
    smoothedMix.skip(numSamples);
    dryWetMixer.setWetMixProportion(smoothedMix.getTargetValue());
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
}

void PingPongDelay::reset()
{
    dampingFilterLeft.reset();
    dampingFilterRight.reset();
    dryWetMixer.reset();
}

void PingPongDelay::setDelayTime(float timeMs)
{
    delayTimeMs = juce::jlimit(1.0f, 2000.0f, timeMs);
    smoothedDelayTime.setTargetValue(delayTimeMs);
}

void PingPongDelay::setDelayMix(float mix)
{
    delayMix = juce::jlimit(0.0f, 1.0f, mix);
    smoothedMix.setTargetValue(delayMix);
}

void PingPongDelay::setFeedback(float fb)
{
    // Allow feedback up to 95% for longer tails
    feedback = juce::jlimit(0.0f, 0.95f, fb);
    smoothedFeedback.setTargetValue(feedback);
}