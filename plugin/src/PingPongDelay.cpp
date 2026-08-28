#include "Synthortion/PingPongDelay.h"

namespace synthortion::dsp {

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

void PingPongDelay::process(juce::AudioBuffer<float> &buffer, const PingPongDelayParams& params)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 2 || numSamples == 0)
        return;

    smoothedDelayTime.setTargetValue(juce::jlimit(kMinDelayTimeMs, kMaxDelayTimeMs, params.delayTimeMs));
    smoothedMix.setTargetValue(juce::jlimit(kMinMix, kMaxMix, params.mix));
    smoothedFeedback.setTargetValue(juce::jlimit(kMinFeedback, kMaxFeedback, params.feedback));

    const float targetDamping = juce::jlimit(1000.0f, 20000.0f, params.dampingFrequency);
    if (std::abs(dampingFrequency - targetDamping) > 1.0f)
    {
        dampingFrequency = targetDamping;
        updateDampingFilters();
    }

    // Check if delay is completely bypassed
    if (smoothedMix.getCurrentValue() <= 0.0001f && smoothedMix.getTargetValue() <= 0.0001f) {
        // Clear delay line to prevent feedback build-up while muted
        if (!isDelayLineClear) {
            delayLine.reset();
            isDelayLineClear = true;
        }
        return; // Buffer is untouched, meaning 100% dry signal passes through
    }

    isDelayLineClear = false;

    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float>(buffer));

    const float maxDelaySamples = static_cast<float>(delayLine.getMaximumDelayInSamples()) - kDelaySamplesSafetyMargin;

    for (int i = 0; i < numSamples; ++i)
    {
        const float currentDelayMs = smoothedDelayTime.getNextValue();
        const float currentFeedback = juce::jlimit(0.0f, 0.95f, smoothedFeedback.getNextValue());
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

        // Ping-pong delay topology:
        // Mono input fed into Left channel;
        // Left feedback feeds Right channel, Right feedback feeds Left channel
        const float monoInput = 0.5f * (leftChannel[i] + rightChannel[i]);
        const float leftInput = monoInput + (feedbackRight * currentFeedback);
        const float rightInput = (feedbackLeft * currentFeedback);

        delayLine.pushSample(0, leftInput);
        delayLine.pushSample(1, rightInput);

        leftChannel[i] = delayedLeft;
        rightChannel[i] = delayedRight;
    }

    dryWetMixer.setWetMixProportion(smoothedMix.getCurrentValue());
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
}

void PingPongDelay::reset() noexcept
{
    delayLine.reset();
    dampingFilterLeft.reset();
    dampingFilterRight.reset();
    dryWetMixer.reset();
    smoothedDelayTime.setCurrentAndTargetValue(kDefaultDelayTimeMs);
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedFeedback.setCurrentAndTargetValue(kDefaultFeedback);
    isDelayLineClear = true;
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
float PingPongDelay::getSubdivisionBeats(int subdivisionIndex) noexcept
{
    static constexpr std::array<float, kNumSubdivisions> kSubdivisionBeats = {
        0.125f,               // 1/32  (1/8 beat)
        1.0f / 6.0f,          // 1/16T (1/6 beat)
        0.25f,                // 1/16  (1/4 beat)
        0.375f,               // 1/16D (3/8 beat)
        1.0f / 3.0f,          // 1/8T  (1/3 beat)
        0.5f,                 // 1/8   (1/2 beat)
        0.75f,                // 1/8D  (3/4 beat)
        2.0f / 3.0f,          // 1/4T  (2/3 beat)
        1.0f,                 // 1/4   (1 beat)
        1.5f,                 // 1/4D  (1.5 beats)
        4.0f / 3.0f,          // 1/2T  (4/3 beats)
        2.0f,                 // 1/2   (2 beats)
        3.0f,                 // 1/2D  (3 beats)
        4.0f                  // 1/1   (4 beats)
    };
    const int idx = juce::jlimit(0, kNumSubdivisions - 1, subdivisionIndex);
    return kSubdivisionBeats[static_cast<size_t>(idx)];
}

const char* PingPongDelay::getSubdivisionName(int subdivisionIndex) noexcept
{
    static constexpr std::array<const char*, kNumSubdivisions> kSubdivisionNames = {
        "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D",
        "1/4T", "1/4", "1/4D", "1/2T", "1/2", "1/2D", "1/1"
    };
    const int idx = juce::jlimit(0, kNumSubdivisions - 1, subdivisionIndex);
    return kSubdivisionNames[static_cast<size_t>(idx)];
}

float PingPongDelay::calculateSyncDelayTimeMs(int subdivisionIndex, double bpm) noexcept
{
    const double effectiveBpm = (bpm > 0.0 && std::isfinite(bpm)) ? bpm : static_cast<double>(kDefaultBpm);
    const float beats = getSubdivisionBeats(subdivisionIndex);
    const float beatTimeMs = static_cast<float>(60000.0 / effectiveBpm);
    return beatTimeMs * beats;
}

float PingPongDelay::calculateDelayTimeMs(float delayTimeFree, int delayTimeSync, bool isSync, double bpm) noexcept
{
    if (isSync)
    {
        return calculateSyncDelayTimeMs(delayTimeSync, bpm);
    }
    return juce::jlimit(kMinDelayTimeMs, kMaxFreeDelayTimeMs, delayTimeFree);
}
} // namespace synthortion::dsp