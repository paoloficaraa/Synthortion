#include "Synthortion/PingPongDelay.h"

PingPongDelay::PingPongDelay()
{
    dampingFilterLeft.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 8000.0f);
    dampingFilterRight.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 8000.0f);
}

void PingPongDelay::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    // Set maximum delay BEFORE prepare
    int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    // Now prepare the delay lines
    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);

    dampingFilterLeft.prepare(spec);
    dampingFilterRight.prepare(spec);
    dampingFilterLeft.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0f);
    dampingFilterRight.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0f);

    reset();
    updateDelayTime(); // Call after prepare and reset
}

void PingPongDelay::process(juce::AudioBuffer<float> &buffer)
{
    const int numSamples = buffer.getNumSamples();
    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        // Read delayed signals
        float delayedLeft = delayLineLeft.popSample(0);
        float delayedRight = delayLineRight.popSample(1);

        // Apply damping
        delayedLeft = dampingFilterLeft.processSample(delayedLeft);
        delayedRight = dampingFilterRight.processSample(delayedRight);

        // Ping-pong: left feedback goes to right, right goes to left
        float leftInput = leftChannel[i] + (delayedRight * feedback);
        float rightInput = rightChannel[i] + (delayedLeft * feedback);

        // Push to delay lines
        delayLineLeft.pushSample(0, leftInput);
        delayLineRight.pushSample(1, rightInput);

        // Mix dry and wet
        leftChannel[i] = leftChannel[i] * (1.0f - delayMix) + delayedLeft * delayMix;
        rightChannel[i] = rightChannel[i] * (1.0f - delayMix) + delayedRight * delayMix;
    }
}

void PingPongDelay::reset()
{
    delayLineLeft.reset();
    delayLineRight.reset();
    dampingFilterLeft.reset();
    dampingFilterRight.reset();
}

void PingPongDelay::setDelayTime(float timeMs)
{
    delayTimeMs = juce::jlimit(1.0f, 2000.0f, timeMs);
    updateDelayTime();
}

void PingPongDelay::setDelayMix(float mix)
{
    delayMix = juce::jlimit(0.0f, 1.0f, mix);
}

void PingPongDelay::setFeedback(float fb)
{
    feedback = juce::jlimit(0.0f, 0.95f, fb);
}

void PingPongDelay::updateDelayTime()
{
    if (sampleRate <= 0.0)
        return; // Not yet prepared

    // Calculate delay samples conservatively
    float delaySamplesFloat = delayTimeMs * static_cast<float>(sampleRate) / 1000.0f;

    // Get the actual maximum from the delay line (safer than recalculating)
    int maxDelaySamples = delayLineLeft.getMaximumDelayInSamples();
    float maxDelaySamplesFloat = static_cast<float>(maxDelaySamples);

    // Clamp to valid range with margin
    delaySamplesFloat = juce::jlimit(0.0f, maxDelaySamplesFloat - 2.0f, delaySamplesFloat);

    delayLineLeft.setDelay(delaySamplesFloat);
    delayLineRight.setDelay(delaySamplesFloat);
}