#include "Synthortion/SynthortionChorus.h"

SynthortionChorus::SynthortionChorus()
{
    chorus.setCentreDelay(kDefaultCentreDelayMs);
    chorus.setDepth(kDefaultDepth);
    chorus.setRate(kDefaultRateHz);
    chorus.setFeedback(kDefaultFeedback);
    chorus.setMix(kChorusInternalMix);
}

void SynthortionChorus::prepare(const juce::dsp::ProcessSpec &spec)
{
    chorus.prepare(spec);
    dryWetMixer.prepare(spec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
}

void SynthortionChorus::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
    dryWetMixer.setWetMixProportion(chorusMix);
    dryWetMixer.pushDrySamples(context.getInputBlock());
    chorus.process(context);
    dryWetMixer.mixWetSamples(context.getOutputBlock());
}

void SynthortionChorus::reset()
{
    chorus.reset();
    dryWetMixer.reset();
}

void SynthortionChorus::setChorusMix(float mix)
{
    chorusMix = juce::jlimit(kMinMix, kMaxMix, mix);
}

void SynthortionChorus::setCentreDelay(float delayMs)
{
    chorus.setCentreDelay(juce::jlimit(kMinDelay, kMaxDelay, delayMs));
}

void SynthortionChorus::setDepth(float depth)
{
    chorus.setDepth(juce::jlimit(kMinDepth, kMaxDepth, depth));
}

void SynthortionChorus::setRate(float rateHz)
{
    chorus.setRate(juce::jlimit(kMinRate, kMaxRate, rateHz));
}

void SynthortionChorus::setFeedback(float feedback)
{
    chorus.setFeedback(juce::jlimit(kMinFeedback, kMaxFeedback, feedback));
}