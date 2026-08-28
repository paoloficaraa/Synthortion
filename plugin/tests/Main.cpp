#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Synthortion/WarmDistortion.h"
#include "Synthortion/Bitcrusher.h"
#include "Synthortion/PingPongDelay.h"
#include "Synthortion/SynthortionChorus.h"
#include "Synthortion/SpectrumAnalyzer.h"
#include <iostream>

class DspSanityTests final : public juce::UnitTest
{
public:
    DspSanityTests() : juce::UnitTest("DSP Modules Sanity", "Synthortion") {}

    void runTest() override
    {
        beginTest("WarmDistortion prepare & process");
        {
            synthortion::dsp::WarmDistortion dist;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            dist.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 256; ++i)
                    buffer.setSample(ch, i, std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * (float)i / 48000.0f));

            synthortion::dsp::WarmDistortionParams params;
            params.drive = 0.5f;
            params.volumeCompensation = false;

            dist.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("BitCrusher prepare & process");
        {
            synthortion::dsp::BitCrusher bc;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            bc.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 256; ++i)
                    buffer.setSample(ch, i, 0.5f);

            synthortion::dsp::BitCrusherParams params;
            params.crush = 0.5f;

            bc.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("PingPongDelay prepare & process");
        {
            synthortion::dsp::PingPongDelay delay;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            delay.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);

            synthortion::dsp::PingPongDelayParams params;
            params.mix = 0.5f;
            params.feedback = 0.4f;
            params.delayTimeMs = 250.0f;
            params.dampingFrequency = 12000.0f;

            delay.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("SynthortionChorus prepare & process");
        {
            synthortion::dsp::SynthortionChorus chorus;
            juce::dsp::ProcessSpec spec{ 48000.0, 256, 2 };
            chorus.prepare(spec);

            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);

            synthortion::dsp::ChorusParams params;
            params.mix = 0.5f;
            params.width = 0.5f;

            chorus.process(buffer, params);
            expect(buffer.getMagnitude(0, 256) > 0.0f);
        }

        beginTest("SpectrumAnalyzer prepare & process");
        {
            synthortion::SpectrumAnalyzer analyzer;
            analyzer.prepare(48000.0, 60.0f);

            std::array<float, synthortion::SpectrumAnalyzer::kFftSize> input;
            input.fill(0.1f);
            const auto& mags = analyzer.process(input.data(), synthortion::SpectrumAnalyzer::kFftSize);
            expect(mags.size() == synthortion::SpectrumAnalyzer::kNumBands);
        }
    }
};

static DspSanityTests dspSanityTests;

int main(int /*argc*/, char** /*argv*/)
{
    juce::initialiseJuce_GUI();

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runTestsInCategory("Synthortion");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* res = runner.getResult(i);
        std::cout << "Test: " << res->unitTestName.toStdString() << " - Passed: " << res->passes << " Failed: " << res->failures << "\n";
        for (const auto& msg : res->messages)
        {
            std::cout << "  " << msg.toStdString() << "\n";
        }
        failures += static_cast<int>(res->failures);
    }
    std::cout << "TOTAL FAILURES: " << failures << "\n";

    juce::shutdownJuce_GUI();
    return failures == 0 ? 0 : 1;
}
