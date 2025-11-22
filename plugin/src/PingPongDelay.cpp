#include "Synthortion/PingPongDelay.h"

PingPongDelay::PingPongDelay()
{
    dampingFilterLeft.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 8000.0f);
    dampingFilterRight.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 8000.0f);

    smoothedDelayTime.setCurrentAndTargetValue(250.0f);
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedFeedback.setCurrentAndTargetValue(0.4f);
}

void PingPongDelay::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);

    dampingFilterLeft.prepare(spec);
    dampingFilterRight.prepare(spec);
    dampingFilterLeft.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0f);
    dampingFilterRight.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0f);

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
        delaySamples = juce::jlimit(1.0f, static_cast<float>(delayLineLeft.getMaximumDelayInSamples() - 2), delaySamples);

        // Set delay for both lines
        delayLineLeft.setDelay(delaySamples);
        delayLineRight.setDelay(delaySamples);

        // Read delayed signals
        float delayedLeft = delayLineLeft.popSample(0);
        float delayedRight = delayLineRight.popSample(1);

        // Apply damping
        delayedLeft = dampingFilterLeft.processSample(delayedLeft);
        delayedRight = dampingFilterRight.processSample(delayedRight);

        // Ping-pong: left feedback goes to right, right goes to left
        float leftInput = leftChannel[i] + (delayedRight * currentFeedback);
        float rightInput = rightChannel[i] + (delayedLeft * currentFeedback);

        // Push to delay lines
        delayLineLeft.pushSample(0, leftInput);
        delayLineRight.pushSample(1, rightInput);

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
    // delayLineLeft.reset();
    // delayLineRight.reset();
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