#include "Synthortion/SynthortionChorus.h"

SynthortionChorus::SynthortionChorus()
{
    // Fixed parameters based on your specifications
    // Note: setDepth expects a normalized value (0-1), not milliseconds
    chorus.setCentreDelay(3.33f); // 3.33 ms centre delay
    chorus.setDepth(0.9f);        // Depth as normalized value (0-1) - high modulation
    chorus.setRate(1.06f);        // 1.06 Hz rate
    chorus.setFeedback(0.33f);    // 33% feedback
    chorus.setMix(1.0f);          // 100% wet (we handle dry/wet externally)
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