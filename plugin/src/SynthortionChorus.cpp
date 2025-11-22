#include "Synthortion/SynthortionChorus.h"

SynthortionChorus::SynthortionChorus()
{
    chorus.setCentreDelay(3.33f);
    chorus.setDepth(0.47f);
    chorus.setRate(1.06f);
    chorus.setFeedback(0.33f);
    chorus.setMix(1.0f);
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
    chorusMix = juce::jlimit(0.0f, 1.0f, mix);
}