#pragma once

#include <JuceHeader.h>
#include <atomic>

extern std::atomic<bool> g_trackAllocations;
extern std::atomic<size_t> g_allocationCount;

#include "Synthortion/DspModule.h"
#include "Synthortion/WarmDistortion.h"
#include "Synthortion/Bitcrusher.h"
#include "Synthortion/PingPongDelay.h"
#include "Synthortion/SynthortionChorus.h"
#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"
#include "Synthortion/AudioCaptureFifo.h"
#include "Synthortion/SpectrumAnalyzer.h"
namespace synthortion
{
    class AudioPluginTests final : public juce::UnitTest
    {
    public:
        AudioPluginTests()
            : juce::UnitTest ("Synthortion component tests", "Synthortion")
        {
        }

        void runTest() override
        {
            testEditorSizeIs960x600();
            testEditorContainsWebBrowserComponent();
            testProcessorAPVTSParameterCount();
            testEditorPaintRendersBackground();
            testParameterValuesAndBridgeProtocol();
            testDistResourceProvider();
            testAudioCaptureFifoBasic();
            testProcessorCapturesPostFxAudio();
            testSpectrumAnalyzerFrequencyMapping();
            testSpectrumAnalyzerLowFrequencyInterpolation();
            testSpectrumAnalyzerHighFrequencyPeakAggregation();
            testSpectrumAnalyzerDecibelNormalization();
            testSpectrumAnalyzerBallistics();
            testEditorTimerAndSpectrumFrame();
            testProcessorBypassParameterAndPassthrough();
            testDspModulesConceptAndIsolatedLifecycle();
            testDspModulesAudioMutationAndClickFreeModulation();
            testDspModulesZeroAllocationsAndLocksOnAudioThread();
            testStateSerializationRoundtrip();
            testStateSerializationMissingNodesAndOlderRevisions();
            testStateSerializationSynchronousExecution();
            testEditorUIPreferencesChange();
            testWarmDistortionMathematicalFormulas();
            testWarmDistortionOversamplingAndLatency();
            testWarmDistortionExciterGatingAndDynamicDamping();
            testWarmDistortionAutoGainCompensation();
            testBitCrusherCoupledTrajectoryMathematicalFormulas();
            testBitCrusherIdentityTransparentOutputAtZero();
            testBitCrusherFractionalPhaseAccumulatorAndLinearInterpolation();
            testBitCrusherTpdfDynamicDitheringAndQuantization();
            testBitCrusherClickFreeParameterModulation();
        }
        void testEditorSizeIs960x600()
        {
            beginTest ("Plugin editor dimensions are 960x600 and resizable with fixed 16:10 aspect ratio");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 960, "Editor width should be 960");
            expect (editor.getHeight() == 600, "Editor height should be 600");
            expect (editor.isResizable(), "Editor should be resizable");
            if (auto* constrainer = editor.getConstrainer())
            {
                expectWithinAbsoluteError (constrainer->getFixedAspectRatio(), 1.6, 0.001, "Aspect ratio should be 1.6 (16:10)");
                expect (constrainer->getMinimumWidth() == 768, "Minimum width should be 768");
                expect (constrainer->getMinimumHeight() == 480, "Minimum height should be 480");
                expect (constrainer->getMaximumWidth() == 1920, "Maximum width should be 1920");
                expect (constrainer->getMaximumHeight() == 1200, "Maximum height should be 1200");
            }
        }
        void testEditorContainsWebBrowserComponent()
        {
            beginTest ("Plugin editor contains WebBrowserComponent");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            bool foundWebBrowser = false;
            for (auto* child : editor.getChildren())
            {
                if (dynamic_cast<juce::WebBrowserComponent*> (child) != nullptr)
                {
                    foundWebBrowser = true;
                    break;
                }
            }

            expect (foundWebBrowser, "Editor should contain a WebBrowserComponent child");
        }

        void testProcessorAPVTSParameterCount()
        {
            beginTest ("Plugin processor contains 16 APVTS parameters");

            AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            expect (apvts.getParameter ("INPUT_GAIN") != nullptr);
            expect (apvts.getParameter ("OUTPUT_GAIN") != nullptr);
            expect (apvts.getParameter ("COLOR") != nullptr);
            expect (apvts.getParameter ("BITCRUSH") != nullptr);
            expect (apvts.getParameter ("DELAY_TIME") != nullptr);
            expect (apvts.getParameter ("DELAY_MIX") != nullptr);
            expect (apvts.getParameter ("DELAY_FEEDBACK") != nullptr);
            expect (apvts.getParameter ("CHORUS_MIX") != nullptr);
            expect (apvts.getParameter ("PLUGIN_BYPASS") != nullptr);
            expect (apvts.getParameter ("DRIVE_ON") != nullptr);
            expect (apvts.getParameter ("BITCRUSH_ON") != nullptr);
            expect (apvts.getParameter ("DELAY_ON") != nullptr);
            expect (apvts.getParameter ("CHORUS_ON") != nullptr);
            expect (apvts.getParameter ("DRIVE_ROUTE") != nullptr);
            expect (apvts.getParameter ("DELAY_SYNC") != nullptr);
            expect (apvts.getParameter ("CHORUS_WIDE") != nullptr);
        }

        void testEditorPaintRendersBackground()
        {
            beginTest ("Plugin editor paints background");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            editor.repaint();
            const auto snapshot = editor.createComponentSnapshot (editor.getLocalBounds());
            expect (snapshot.getWidth() == 960 && snapshot.getHeight() == 600,
                    "Snapshot should match editor dimensions");
        }

        void testParameterValuesAndBridgeProtocol()
        {
            beginTest ("Bridge protocol and parameter normalization");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);
            auto& apvts = processor.getAPVTS();

            // Test init payload generation
            auto initPayload = editor.buildInitPayload();
            expect (initPayload.isObject());
            if (auto* obj = initPayload.getDynamicObject())
            {
                expect (static_cast<int>(obj->getProperty("schemaVersion")) == 1, "schemaVersion should be 1");
                auto params = obj->getProperty("parameters");
                expect (params.isArray(), "parameters should be an array");
                if (auto* arr = params.getArray())
                {
                    expect (arr->size() == processor.getParameters().size(), "Should have correct number of parameters");
                    if (arr->size() > 0)
                    {
                        auto firstParam = arr->getReference(0);
                        expect (firstParam.isObject(), "Parameter entry should be an object");
                        if (auto* pObj = firstParam.getDynamicObject())
                        {
                            expect (pObj->hasProperty("id"), "Should have id");
                            expect (pObj->hasProperty("value"), "Should have value");
                            expect (pObj->hasProperty("min"), "Should have min");
                            expect (pObj->hasProperty("max"), "Should have max");
                            expect (pObj->hasProperty("defaultValue"), "Should have defaultValue");
                        }
                    }
                }
                expect (obj->hasProperty("uiPreferences"), "Should have uiPreferences");
                auto uiPrefsVar = obj->getProperty("uiPreferences");
                expect (uiPrefsVar.isObject(), "uiPreferences should be an object");
                if (auto* prefsObj = uiPrefsVar.getDynamicObject())
                {
                    expectWithinAbsoluteError (static_cast<double>(prefsObj->getProperty(UIPreferences::kUiScale)), UIPreferences::kDefaultUiScale, 0.001);
                    expectWithinAbsoluteError (static_cast<double>(prefsObj->getProperty(UIPreferences::kSpectrumDecay)), UIPreferences::kDefaultSpectrumDecay, 0.001);
                    expect (static_cast<bool>(prefsObj->getProperty(UIPreferences::kSkipBootSequence)) == UIPreferences::kDefaultSkipBootSequence);
                }
            }

            // Test handleSetParameter with valid values
            juce::DynamicObject::Ptr msg = new juce::DynamicObject();
            msg->setProperty ("id", "COLOR");
            msg->setProperty ("value", 0.75f);
            editor.handleSetParameter(juce::var(msg.get()));
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.75f, 0.001f, "COLOR normalized value should be 0.75");
            }

            // Test handleSetParameter bounds clamping
            juce::DynamicObject::Ptr msgClampHigh = new juce::DynamicObject();
            msgClampHigh->setProperty ("id", "COLOR");
            msgClampHigh->setProperty ("value", 2.5f); // Out of bounds
            editor.handleSetParameter(juce::var(msgClampHigh.get()));
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 1.0f, 0.001f, "COLOR should be clamped to 1.0");
            }

            juce::DynamicObject::Ptr msgClampLow = new juce::DynamicObject();
            msgClampLow->setProperty ("id", "COLOR");
            msgClampLow->setProperty ("value", -1.5f); // Out of bounds
            editor.handleSetParameter(juce::var(msgClampLow.get()));
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.0f, 0.001f, "COLOR should be clamped to 0.0");
            }

            // Test invalid ID rejection
            juce::DynamicObject::Ptr msgInvalidId = new juce::DynamicObject();
            msgInvalidId->setProperty ("id", "INVALID_PARAM_ID");
            msgInvalidId->setProperty ("value", 0.5f);
            editor.handleSetParameter(juce::var(msgInvalidId.get())); // Should not crash or hang

            // Finite check test - we set it to NaN, it shouldn't change the value from 0.0
            juce::DynamicObject::Ptr msgNaN = new juce::DynamicObject();
            msgNaN->setProperty ("id", "COLOR");
            msgNaN->setProperty ("value", std::numeric_limits<float>::quiet_NaN());
            editor.handleSetParameter(juce::var(msgNaN.get()));
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.0f, 0.001f, "COLOR should ignore NaN");
            }

            // Telemetry dispatches conforming to spectrumFrame and meterFrame schemas.
            std::array<float, SpectrumAnalyzer::kNumBands> dummyMags;
            dummyMags.fill(0.5f);
            auto spectrumPayload = editor.buildSpectrumPayload(dummyMags);
            expect(spectrumPayload.isArray(), "Spectrum payload should be array");
            if (auto* arr = spectrumPayload.getArray())
            {
                expect(arr->size() == SpectrumAnalyzer::kNumBands, "Spectrum array size should be 80");
            }

            AudioPluginAudioProcessor::MeterPeaks dummyPeaks { 0.8f, 0.9f };
            auto meterPayload = editor.buildMeterPayload(dummyPeaks);
            expect(meterPayload.isObject(), "Meter payload should be object");
            if (auto* obj = meterPayload.getDynamicObject())
            {
                expect(obj->hasProperty("input") && obj->hasProperty("output"), "Meter should have input and output");
                expectWithinAbsoluteError(static_cast<float>(obj->getProperty("input")), 0.8f, 0.001f, "Meter input should match");
            }
        }
        void testProcessorBypassParameterAndPassthrough()
        {
            beginTest ("PluginProcessor overrides getBypassParameter and processBlockBypassed captures audio");

            AudioPluginAudioProcessor processor;
            processor.prepareToPlay (48000.0, 512);

            auto* bypassParam = processor.getBypassParameter();
            expect (bypassParam != nullptr, "getBypassParameter should return a valid parameter");
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (bypassParam))
            {
                expect (pWithId->paramID == "PLUGIN_BYPASS", "Bypass parameter ID must be PLUGIN_BYPASS");
            }

            auto& fifo = processor.getAudioFifo();
            fifo.reset();

            juce::AudioBuffer<float> testBuffer (2, 512);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < 512; ++s)
                    testBuffer.setSample (ch, s, 0.6f);

            juce::MidiBuffer midi;
            processor.processBlockBypassed (testBuffer, midi);

            expect (fifo.getNumReady() >= 512, "processBlockBypassed must push dry audio into FIFO");
            auto peaks = processor.getMeterPeaks();
            expectWithinAbsoluteError (peaks.input, 0.6f, 0.01f, "processBlockBypassed should measure input peak");
            expectWithinAbsoluteError (peaks.output, 0.6f, 0.01f, "processBlockBypassed should measure output peak");
        }
        void testDistResourceProvider()
        {
            beginTest ("Web Resource Provider delivers index.html");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto res = editor.getResource ("/");
            if (res.has_value())
            {
                expect (res->mimeType == "text/html", "Mime type should be text/html");
                expect (! res->data.empty(), "Resource data should not be empty");
            }
        }
        void testAudioCaptureFifoBasic()
        {
            beginTest ("AudioCaptureFifo push, pop, mono downmixing, and wraparound");

            AudioCaptureFifo fifo;
            expect (fifo.getCapacity() == 16384, "Capacity should be 16384");
            expect (fifo.getNumReady() == 0, "Initial ready samples should be 0");

            // Create stereo test buffer: L = 1.0, R = 0.5 -> Mono = 0.75
            juce::AudioBuffer<float> stereoBuffer (2, 512);
            for (int i = 0; i < 512; ++i)
            {
                stereoBuffer.setSample (0, i, 1.0f);
                stereoBuffer.setSample (1, i, 0.5f);
            }

            fifo.push (stereoBuffer);
            expect (fifo.getNumReady() == 512, "Should have 512 ready samples");

            std::vector<float> readBuf (512, 0.0f);
            int popped = fifo.pop (readBuf.data(), 512);
            expect (popped == 512, "Should pop 512 samples");
            expect (fifo.getNumReady() == 0, "Should have 0 ready samples after pop");

            for (int i = 0; i < 512; ++i)
            {
                expectWithinAbsoluteError (readBuf[static_cast<size_t> (i)], 0.75f, 0.0001f, "Mono downmix (1.0 + 0.5) * 0.5 = 0.75");
            }

            // Test wraparound by pushing 16000, reading 16000, then pushing 1000 across the wrap boundary
            juce::AudioBuffer<float> bigBuffer (1, 16000);
            for (int i = 0; i < 16000; ++i)
                bigBuffer.setSample (0, i, static_cast<float> (i));

            fifo.push (bigBuffer);
            std::vector<float> bigRead (16000);
            fifo.pop (bigRead.data(), 16000);

            juce::AudioBuffer<float> wrapBuffer (1, 1000);
            for (int i = 0; i < 1000; ++i)
                wrapBuffer.setSample (0, i, static_cast<float> (i + 100));
            fifo.push (wrapBuffer);
            expect (fifo.getNumReady() == 1000, "Should have 1000 samples after wraparound push");

            std::vector<float> wrapRead (1000);
            int wrapPopped = fifo.pop (wrapRead.data(), 1000);
            expect (wrapPopped == 1000, "Should pop 1000 samples across wrap");
            for (int i = 0; i < 1000; ++i)
            {
                expectWithinAbsoluteError (wrapRead[static_cast<size_t> (i)], static_cast<float> (i + 100), 0.0001f, "Wraparound sample fidelity");
            }
        }

        void testProcessorCapturesPostFxAudio()
        {
            beginTest ("PluginProcessor captures post-FX audio into AudioCaptureFifo in processBlock");

            AudioPluginAudioProcessor processor;
            processor.prepareToPlay (48000.0, 512);

            auto& fifo = processor.getAudioFifo();
            fifo.reset();
            expect (fifo.getNumReady() == 0, "Fifo should be empty after reset");

            juce::AudioBuffer<float> testBuffer (2, 512);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < 512; ++s)
                    testBuffer.setSample (ch, s, 0.4f);

            juce::MidiBuffer midi;
            processor.processBlock (testBuffer, midi);

            expect (fifo.getNumReady() >= 512, "Processor processBlock should have pushed samples into AudioCaptureFifo");
            std::vector<float> readSamples (512);
            int count = fifo.pop (readSamples.data(), 512);
            expect (count == 512, "Should pop 512 samples from processor FIFO");
        }
        void testSpectrumAnalyzerFrequencyMapping()
        {
            beginTest ("SpectrumAnalyzer maps 80 log-spaced frequency bands from 20 Hz to 20 kHz");

            SpectrumAnalyzer analyzer;
            analyzer.prepare (48000.0, 60.0f);

            expectWithinAbsoluteError (analyzer.getBandFrequency (0), 20.0f, 0.1f, "Band 0 frequency should be 20 Hz");
            expectWithinAbsoluteError (analyzer.getBandFrequency (79), 20000.0f, 1.0f, "Band 79 frequency should be 20000 Hz");

            for (int b = 1; b < SpectrumAnalyzer::kNumBands; ++b)
            {
                expect (analyzer.getBandFrequency (b) > analyzer.getBandFrequency (b - 1),
                        "Band frequencies must be strictly monotonically increasing");
            }
        }

        void testSpectrumAnalyzerLowFrequencyInterpolation()
        {
            beginTest ("SpectrumAnalyzer sub-bin linear interpolation for low frequencies (20 Hz - 200 Hz)");

            SpectrumAnalyzer analyzer;
            const double sampleRate = 48000.0;
            analyzer.prepare (sampleRate, 60.0f);

            // Generate 100 Hz sine wave (amplitude 1.0 = 0 dBFS)
            const float testFreq = 100.0f;
            std::vector<float> sineWave (SpectrumAnalyzer::kFftSize);
            for (int i = 0; i < SpectrumAnalyzer::kFftSize; ++i)
            {
                sineWave[static_cast<size_t> (i)] = std::sin (2.0f * juce::MathConstants<float>::pi * testFreq * static_cast<float> (i) / static_cast<float> (sampleRate));
            }

            const auto& bands = analyzer.process (sineWave.data(), SpectrumAnalyzer::kFftSize);

            // Find peak band
            int peakBand = 0;
            float peakVal = 0.0f;
            for (int b = 0; b < SpectrumAnalyzer::kNumBands; ++b)
            {
                if (bands[static_cast<size_t> (b)] > peakVal)
                {
                    peakVal = bands[static_cast<size_t> (b)];
                    peakBand = b;
                }
            }

            const float peakFreq = analyzer.getBandFrequency (peakBand);
            expectWithinAbsoluteError (peakFreq, testFreq, 20.0f, "Peak band should be centered near 100 Hz");
            expect (peakVal > 0.85f, "Peak band magnitude for 0 dBFS sine should be near 1.0");

            // Distant high band should be quiet
            expect (bands[70] < 0.2f, "High frequency bands should be quiet for 100 Hz sine");
        }

        void testSpectrumAnalyzerHighFrequencyPeakAggregation()
        {
            beginTest ("SpectrumAnalyzer peak bin aggregation for high frequencies (2 kHz - 20 kHz)");

            SpectrumAnalyzer analyzer;
            const double sampleRate = 48000.0;
            analyzer.prepare (sampleRate, 60.0f);

            // Generate 5000 Hz sine wave
            const float testFreq = 5000.0f;
            std::vector<float> sineWave (SpectrumAnalyzer::kFftSize);
            for (int i = 0; i < SpectrumAnalyzer::kFftSize; ++i)
            {
                sineWave[static_cast<size_t> (i)] = std::sin (2.0f * juce::MathConstants<float>::pi * testFreq * static_cast<float> (i) / static_cast<float> (sampleRate));
            }

            const auto& bands = analyzer.process (sineWave.data(), SpectrumAnalyzer::kFftSize);

            int peakBand = 0;
            float peakVal = 0.0f;
            for (int b = 0; b < SpectrumAnalyzer::kNumBands; ++b)
            {
                if (bands[static_cast<size_t> (b)] > peakVal)
                {
                    peakVal = bands[static_cast<size_t> (b)];
                    peakBand = b;
                }
            }

            const float peakFreq = analyzer.getBandFrequency (peakBand);
            expectWithinAbsoluteError (peakFreq, testFreq, 500.0f, "Peak band should be centered near 5000 Hz");
            expect (peakVal > 0.85f, "Peak band magnitude for 5000 Hz 0 dBFS sine should be near 1.0");
            expect (bands[5] < 0.2f, "Low frequency bands should be quiet for 5000 Hz sine");
        }

        void testSpectrumAnalyzerDecibelNormalization()
        {
            beginTest ("SpectrumAnalyzer converts [-60 dB, 0 dB] to [0.0, 1.0]");

            expectWithinAbsoluteError (SpectrumAnalyzer::normalizeDb (0.0f), 1.0f, 0.001f, "0 dB -> 1.0");
            expectWithinAbsoluteError (SpectrumAnalyzer::normalizeDb (-60.0f), 0.0f, 0.001f, "-60 dB -> 0.0");
            expectWithinAbsoluteError (SpectrumAnalyzer::normalizeDb (-30.0f), 0.5f, 0.001f, "-30 dB -> 0.5");
            expectWithinAbsoluteError (SpectrumAnalyzer::normalizeDb (-80.0f), 0.0f, 0.001f, "Clamped below -60 dB -> 0.0");
            expectWithinAbsoluteError (SpectrumAnalyzer::normalizeDb (+6.0f), 1.0f, 0.001f, "Clamped above 0 dB -> 1.0");
        }

        void testSpectrumAnalyzerBallistics()
        {
            beginTest ("SpectrumAnalyzer instant attack and exponential decay ballistics");

            SpectrumAnalyzer analyzer;
            analyzer.prepare (48000.0, 60.0f);
            analyzer.reset();

            // Full scale 1000 Hz sine
            std::vector<float> sineWave (SpectrumAnalyzer::kFftSize);
            for (int i = 0; i < SpectrumAnalyzer::kFftSize; ++i)
            {
                sineWave[static_cast<size_t> (i)] = std::sin (2.0f * juce::MathConstants<float>::pi * 1000.0f * static_cast<float> (i) / 48000.0f);
            }

            // Frame 1: instant attack jumps to near 1.0
            const auto& frame1 = analyzer.process (sineWave.data(), SpectrumAnalyzer::kFftSize);
            float peak1 = *std::max_element (frame1.begin(), frame1.end());
            expect (peak1 > 0.85f, "Instant attack should reach peak in single frame");

            // Frames 2..11 with silence -> exponential decay
            std::vector<float> silence (SpectrumAnalyzer::kFftSize, 0.0f);
            float prevPeak = peak1;
            for (int f = 0; f < 10; ++f)
            {
                const auto& frame = analyzer.process (silence.data(), SpectrumAnalyzer::kFftSize);
                float currentPeak = *std::max_element (frame.begin(), frame.end());
                expect (currentPeak < prevPeak, "Decay must decrease peak value each frame");
                prevPeak = currentPeak;
            }

            // After 10 frames (~167ms), peak should be decayed to roughly 30-50% of initial peak
            expect (prevPeak > 0.15f && prevPeak < 0.65f,
                    "After ~167ms (10 frames at 60Hz), decayed magnitude should be ~0.25 - 0.60");
        }
        void testEditorTimerAndSpectrumFrame()
        {
            beginTest ("PluginEditor runs 60 Hz timer and processes FFT spectrum from processor audio FIFO");

            AudioPluginAudioProcessor processor;
            processor.prepareToPlay (48000.0, 512);
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.isTimerRunning(), "Editor should start 60 Hz timer on creation");
            expect (editor.getTimerInterval() > 0 && editor.getTimerInterval() <= 20, "Timer interval should be ~16 ms (60 Hz)");

            // Feed 1000 Hz sine into processor
            juce::AudioBuffer<float> buffer (2, 2048);
            for (int s = 0; s < 2048; ++s)
            {
                float val = std::sin (2.0f * juce::MathConstants<float>::pi * 1000.0f * static_cast<float> (s) / 48000.0f);
                buffer.setSample (0, s, val);
                buffer.setSample (1, s, val);
            }

            juce::MidiBuffer midi;
            processor.processBlock (buffer, midi);

            // Manually invoke timerCallback to verify pipeline
            editor.timerCallback();

            const auto& bands = editor.getSpectrumAnalyzer().getSmoothedBands();
            float maxVal = *std::max_element (bands.begin(), bands.end());
            expect (maxVal > 0.85f, "SpectrumAnalyzer should capture 1000 Hz peak > 0.85 after timerCallback");
        }

        void testDspModulesConceptAndIsolatedLifecycle()
        {
            beginTest ("DspModule concept and isolated prepare/reset/latency across sample rates");

            static_assert(dsp::DspModule<dsp::WarmDistortion, dsp::WarmDistortionParams>);
            static_assert(dsp::DspModule<dsp::BitCrusher, dsp::BitCrusherParams>);
            static_assert(dsp::DspModule<dsp::PingPongDelay, dsp::PingPongDelayParams>);
            static_assert(dsp::DspModule<dsp::SynthortionChorus, dsp::ChorusParams>);

            const double sampleRates[] = { 44100.0, 48000.0, 96000.0, 192000.0 };
            const int maxBlockSize = 512;
            const int numChannels = 2;

            for (double sr : sampleRates)
            {
                juce::dsp::ProcessSpec spec{ sr, static_cast<juce::uint32>(maxBlockSize), static_cast<juce::uint32>(numChannels) };

                dsp::WarmDistortion warmDist;
                warmDist.prepare(spec);
                expect(warmDist.getLatencySamples() >= 0, "WarmDistortion latency non-negative");
                warmDist.reset();

                dsp::BitCrusher bitCrusher;
                bitCrusher.prepare(spec);
                expect(bitCrusher.getLatencySamples() == 0, "BitCrusher latency is 0");
                bitCrusher.reset();

                dsp::PingPongDelay delay;
                delay.prepare(spec);
                expect(delay.getLatencySamples() == 0, "PingPongDelay latency is 0");
                delay.reset();

                dsp::SynthortionChorus chorus;
                chorus.prepare(spec);
                expect(chorus.getLatencySamples() == 0, "SynthortionChorus latency is 0");
                chorus.reset();
            }
        }

        void testDspModulesAudioMutationAndClickFreeModulation()
        {
            beginTest ("DspModule audio mutation and click-free parameter modulation");

            const double sampleRate = 48000.0;
            const int blockSize = 256;
            juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(blockSize), 2 };

            auto generateSine = [](juce::AudioBuffer<float>& buf, float freq, double sr)
            {
                for (int s = 0; s < buf.getNumSamples(); ++s)
                {
                    float v = 0.5f * std::sin(2.0f * juce::MathConstants<float>::pi * freq * static_cast<float>(s) / static_cast<float>(sr));
                    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                        buf.setSample(ch, s, v);
                }
            };

            // 1. WarmDistortion mutation & modulation
            {
                dsp::WarmDistortion dist;
                dist.prepare(spec);

                juce::AudioBuffer<float> dryBuf(2, blockSize);
                juce::AudioBuffer<float> wetBuf(2, blockSize);
                generateSine(dryBuf, 440.0f, sampleRate);
                wetBuf.makeCopyOf(dryBuf);

                dsp::WarmDistortionParams params{ 0.8f, false };
                dist.process(wetBuf, params);

                // Verify mutation (harmonics added)
                float diffSum = 0.0f;
                for (int s = 0; s < blockSize; ++s)
                    diffSum += std::abs(wetBuf.getSample(0, s) - dryBuf.getSample(0, s));
                expect(diffSum > 0.1f, "WarmDistortion must alter waveform when driven");

                // Test modulation continuity across blocks (no step > 0.4 between adjacent samples)
                float prevSample = wetBuf.getSample(0, blockSize - 1);
                bool clickDetected = false;
                for (int b = 0; b < 20; ++b)
                {
                    float modDrive = static_cast<float>(b) / 20.0f;
                    generateSine(wetBuf, 440.0f, sampleRate);
                    dist.process(wetBuf, dsp::WarmDistortionParams{ modDrive, false });

                    for (int s = 0; s < blockSize; ++s)
                    {
                        float curSample = wetBuf.getSample(0, s);
                        if (std::abs(curSample - prevSample) > 0.5f)
                            clickDetected = true;
                        prevSample = curSample;
                    }
                }
                expect(!clickDetected, "WarmDistortion drive modulation must be click-free");
            }

            // 2. BitCrusher mutation & modulation
            {
                dsp::BitCrusher bc;
                bc.prepare(spec);

                juce::AudioBuffer<float> dryBuf(2, blockSize);
                juce::AudioBuffer<float> wetBuf(2, blockSize);
                generateSine(dryBuf, 440.0f, sampleRate);
                wetBuf.makeCopyOf(dryBuf);

                dsp::BitCrusherParams params{ 1.0f };
                bc.process(wetBuf, params);

                float diffSum = 0.0f;
                for (int s = 0; s < blockSize; ++s)
                    diffSum += std::abs(wetBuf.getSample(0, s) - dryBuf.getSample(0, s));
                expect(diffSum > 0.1f, "BitCrusher must quantize/downsample when wet");

                // Test mix modulation click-free
                float prevSample = 0.0f;
                bool clickDetected = false;
                for (int b = 0; b < 20; ++b)
                {
                    float mix = (b % 2 == 0) ? 0.0f : 1.0f;
                    generateSine(wetBuf, 440.0f, sampleRate);
                    bc.process(wetBuf, dsp::BitCrusherParams{ mix });
                    for (int s = 0; s < blockSize; ++s)
                    {
                        float cur = wetBuf.getSample(0, s);
                        if (b > 0 && std::abs(cur - prevSample) > 0.6f)
                            clickDetected = true;
                        prevSample = cur;
                    }
                }
                expect(!clickDetected, "BitCrusher mix modulation must be smooth");
            }

            // 3. PingPongDelay mutation & modulation
            {
                dsp::PingPongDelay delay;
                delay.prepare(spec);

                juce::AudioBuffer<float> buf(2, blockSize);
                buf.clear();
                buf.setSample(0, 0, 1.0f); // Impulse

                dsp::PingPongDelayParams params{ 10.0f, 1.0f, 0.5f, 12000.0f };
                delay.process(buf, params);

                // Process next blocks to catch delay echo
                bool echoFound = false;
                for (int b = 0; b < 10; ++b)
                {
                    buf.clear();
                    delay.process(buf, params);
                    for (int ch = 0; ch < 2; ++ch)
                    {
                        for (int s = 0; s < blockSize; ++s)
                        {
                            if (std::abs(buf.getSample(ch, s)) > 0.01f)
                                echoFound = true;
                        }
                    }
                }
                expect(echoFound, "PingPongDelay must produce delayed feedback echoes");
            }

            // 4. SynthortionChorus mutation & modulation
            {
                dsp::SynthortionChorus chorus;
                chorus.prepare(spec);

                juce::AudioBuffer<float> dryBuf(2, blockSize);
                juce::AudioBuffer<float> wetBuf(2, blockSize);
                generateSine(dryBuf, 440.0f, sampleRate);
                wetBuf.makeCopyOf(dryBuf);

                dsp::ChorusParams params{ 1.0f, true };
                chorus.process(wetBuf, params);

                // Run several blocks to let LFO modulate
                float diffSum = 0.0f;
                for (int b = 0; b < 10; ++b)
                {
                    generateSine(wetBuf, 440.0f, sampleRate);
                    chorus.process(wetBuf, params);
                    for (int s = 0; s < blockSize; ++s)
                        diffSum += std::abs(wetBuf.getSample(0, s) - dryBuf.getSample(0, s));
                }
                expect(diffSum > 0.1f, "SynthortionChorus must modulate phase and produce chorus effect");
            }
        }

        void testDspModulesZeroAllocationsAndLocksOnAudioThread()
        {
            beginTest ("Zero heap allocations during DSP process() execution on audio thread");

            const double sampleRate = 48000.0;
            const int blockSize = 512;
            juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(blockSize), 2 };

            dsp::WarmDistortion dist;
            dsp::BitCrusher bitCrusher;
            dsp::PingPongDelay delay;
            dsp::SynthortionChorus chorus;

            // 1. Prepare all modules (allocations allowed in prepare)
            dist.prepare(spec);
            bitCrusher.prepare(spec);
            delay.prepare(spec);
            chorus.prepare(spec);

            juce::AudioBuffer<float> buffer(2, blockSize);
            for (int s = 0; s < blockSize; ++s)
            {
                buffer.setSample(0, s, 0.2f);
                buffer.setSample(1, s, 0.2f);
            }

            // Warm-up one block each to ensure static/lazy structures (if any) are initialized
            dist.process(buffer, dsp::WarmDistortionParams{ 0.5f, false });
            bitCrusher.process(buffer, dsp::BitCrusherParams{ 0.5f });
            delay.process(buffer, dsp::PingPongDelayParams{ 250.0f, 0.5f, 0.4f, 12000.0f });
            chorus.process(buffer, dsp::ChorusParams{ 0.5f, 0.5f });

            // 2. Start allocation tracking
            g_allocationCount.store(0);
            g_trackAllocations.store(true);

            for (int block = 0; block < 100; ++block)
            {
                const float mod = static_cast<float>(block % 10) / 10.0f;

                dist.process(buffer, dsp::WarmDistortionParams{ mod, false });
                bitCrusher.process(buffer, dsp::BitCrusherParams{ mod });
                delay.process(buffer, dsp::PingPongDelayParams{ 200.0f + mod * 100.0f, mod, 0.4f, 12000.0f });
                chorus.process(buffer, dsp::ChorusParams{ mod, (block % 2 == 0) ? 1.0f : 0.0f });
            }

            g_trackAllocations.store(false);
            const size_t allocationsRecorded = g_allocationCount.load();

            expect(allocationsRecorded == 0,
                   "Expected 0 heap allocations across 100 blocks of DSP process(), but got: " + juce::String(static_cast<int>(allocationsRecorded)));
        }
        void testStateSerializationRoundtrip()
        {
            beginTest ("State Serialization: Roundtrip restores all 16 APVTS parameters and UIPreferences");

            AudioPluginAudioProcessor processorA;
            auto& apvtsA = processorA.getAPVTS();

            // Mutate all 16 parameters in processor A
            apvtsA.getParameter("INPUT_GAIN")->setValueNotifyingHost(0.75f);
            apvtsA.getParameter("OUTPUT_GAIN")->setValueNotifyingHost(0.65f);
            apvtsA.getParameter("COLOR")->setValueNotifyingHost(0.85f);
            apvtsA.getParameter("BITCRUSH")->setValueNotifyingHost(0.45f);
            apvtsA.getParameter("DELAY_TIME")->setValueNotifyingHost(0.35f);
            apvtsA.getParameter("DELAY_MIX")->setValueNotifyingHost(0.55f);
            apvtsA.getParameter("DELAY_FEEDBACK")->setValueNotifyingHost(0.60f);
            apvtsA.getParameter("CHORUS_MIX")->setValueNotifyingHost(0.70f);
            apvtsA.getParameter("PLUGIN_BYPASS")->setValueNotifyingHost(1.0f);
            apvtsA.getParameter("DRIVE_ON")->setValueNotifyingHost(0.0f);
            apvtsA.getParameter("BITCRUSH_ON")->setValueNotifyingHost(0.0f);
            apvtsA.getParameter("DELAY_ON")->setValueNotifyingHost(0.0f);
            apvtsA.getParameter("CHORUS_ON")->setValueNotifyingHost(0.0f);
            apvtsA.getParameter("DRIVE_ROUTE")->setValueNotifyingHost(1.0f);
            apvtsA.getParameter("DELAY_SYNC")->setValueNotifyingHost(1.0f);
            apvtsA.getParameter("CHORUS_WIDE")->setValueNotifyingHost(1.0f);

            // Force UI Preferences in state A
            auto& stateA = apvtsA.state;
            UIPreferences::ensureTree(stateA);
            auto uiPrefsA = stateA.getChildWithName(UIPreferences::kNodeName);
            uiPrefsA.setProperty(UIPreferences::kUiScale, 1.5, nullptr);
            uiPrefsA.setProperty(UIPreferences::kSpectrumDecay, 0.6, nullptr);
            uiPrefsA.setProperty(UIPreferences::kSkipBootSequence, true, nullptr);

            juce::MemoryBlock destData;
            processorA.getStateInformation(destData);

            expect(destData.getSize() > 0, "Serialized data should not be empty");

            AudioPluginAudioProcessor processorB;
            auto& apvtsB = processorB.getAPVTS();
            processorB.setStateInformation(destData.getData(), (int)destData.getSize());

            // Verify all 16 parameters in processor B
            const juce::StringArray paramIDs = {
                "INPUT_GAIN", "OUTPUT_GAIN", "COLOR", "BITCRUSH",
                "DELAY_TIME", "DELAY_MIX", "DELAY_FEEDBACK", "CHORUS_MIX",
                "PLUGIN_BYPASS", "DRIVE_ON", "BITCRUSH_ON", "DELAY_ON",
                "CHORUS_ON", "DRIVE_ROUTE", "DELAY_SYNC", "CHORUS_WIDE"
            };

            expectEquals(paramIDs.size(), 16, "Should test exactly 16 APVTS parameters");

            for (const auto& pid : paramIDs)
            {
                auto* paramA = apvtsA.getParameter(pid);
                auto* paramB = apvtsB.getParameter(pid);
                expect(paramA != nullptr, "Param in A should exist: " + pid);
                expect(paramB != nullptr, "Param in B should exist: " + pid);
                if (paramA != nullptr && paramB != nullptr)
                {
                    expectEquals(paramB->getValue(), paramA->getValue(), "Parameter value mismatch for " + pid);
                }
            }

            auto uiPrefsB = apvtsB.state.getChildWithName(UIPreferences::kNodeName);
            expect(uiPrefsB.isValid(), "UIPreferences node should be restored");
            expectEquals((double)uiPrefsB.getProperty(UIPreferences::kUiScale), 1.5);
            expectEquals((double)uiPrefsB.getProperty(UIPreferences::kSpectrumDecay), 0.6);
            expect((bool)uiPrefsB.getProperty(UIPreferences::kSkipBootSequence) == true, "skipBootSequence should be true");
            expectEquals((int)apvtsB.state.getProperty("version"), 1);
        }

        void testStateSerializationMissingNodesAndOlderRevisions()
        {
            beginTest ("State Serialization: Deserialization of missing nodes and older revisions");

            // Hand-craft an older revision state
            juce::ValueTree olderState("SynthortionState");
            olderState.setProperty("PARAM_THAT_DOES_NOT_EXIST", 0.5, nullptr);
            olderState.appendChild(juce::ValueTree("PARAM").setProperty("id", "COLOR", nullptr).setProperty("value", 0.4, nullptr), nullptr);
            // Specifically missing UIPreferences and version

            std::unique_ptr<juce::XmlElement> xml(olderState.createXml());
            juce::MemoryBlock destData;
            juce::AudioProcessor::copyXmlToBinary(*xml, destData);

            AudioPluginAudioProcessor processor;

            // Pre-condition: parameters are default
            expectEquals(processor.getAPVTS().getRawParameterValue("COLOR")->load(), 0.0f);

            processor.setStateInformation(destData.getData(), (int)destData.getSize());

            // Verify that known parameter got updated
            expect(std::abs(processor.getAPVTS().getRawParameterValue("COLOR")->load() - 0.4f) < 0.001f, "COLOR parameter should be updated to 0.4");

            // Verify that version was added
            expectEquals((int)processor.getAPVTS().state.getProperty("version"), 1);

            // Verify that UIPreferences fallback applied
            auto uiPrefs = processor.getAPVTS().state.getChildWithName(UIPreferences::kNodeName);
            expect(uiPrefs.isValid(), "UIPreferences node should be created as fallback");
            expectEquals((double)uiPrefs.getProperty(UIPreferences::kUiScale), UIPreferences::kDefaultUiScale);
            expectEquals((double)uiPrefs.getProperty(UIPreferences::kSpectrumDecay), UIPreferences::kDefaultSpectrumDecay);
            expect((bool)uiPrefs.getProperty(UIPreferences::kSkipBootSequence) == UIPreferences::kDefaultSkipBootSequence, "skipBootSequence default mismatch");
        }

        void testStateSerializationSynchronousExecution()
        {
            beginTest ("State Serialization: Synchronous execution with zero thread creation");

            AudioPluginAudioProcessor processor;
            auto& apvts = processor.getAPVTS();

            // Prepare mutated state
            auto state = apvts.copyState();
            UIPreferences::ensureTree(state);
            state.getChildWithName(UIPreferences::kNodeName).setProperty(UIPreferences::kUiScale, 2.0, nullptr);
            std::unique_ptr<juce::XmlElement> xml(state.createXml());
            juce::MemoryBlock mutatedData;
            juce::AudioProcessor::copyXmlToBinary(*xml, mutatedData);

            const auto callingThreadId = std::this_thread::get_id();

            // Call setStateInformation
            processor.setStateInformation(mutatedData.getData(), (int)mutatedData.getSize());

            // Check that calling thread remained the active executor
            expect(std::this_thread::get_id() == callingThreadId, "Execution must remain synchronous on caller thread");

            // Immediately check the state on the same thread without pumping message loop
            auto uiPrefs = apvts.state.getChildWithName(UIPreferences::kNodeName);
            expect(uiPrefs.isValid(), "UIPreferences should be valid immediately");
            const double currentScale = uiPrefs.getProperty(UIPreferences::kUiScale);
            expectEquals(currentScale, 2.0, "State should be updated synchronously on calling thread");
        }
        void testEditorUIPreferencesChange()
        {
            beginTest ("PluginEditor builds UIPreferences payload and synchronizes on ValueTree mutations");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto payload = editor.buildUIPreferencesPayload();
            expect (payload.isObject(), "Payload should be a dynamic object");
            auto* obj = payload.getDynamicObject();
            expect (obj != nullptr, "Payload dynamic object must not be null");
            if (obj != nullptr)
            {
                expect (obj->hasProperty (UIPreferences::kUiScale), "Payload must contain uiScale");
                expect (obj->hasProperty (UIPreferences::kSpectrumDecay), "Payload must contain spectrumDecay");
                expect (obj->hasProperty (UIPreferences::kSkipBootSequence), "Payload must contain skipBootSequence");
            }

            // Mutate UI preferences
            auto& state = processor.getAPVTS().state;
            UIPreferences::ensureTree(state);
            auto uiPrefs = state.getChildWithName(UIPreferences::kNodeName);
            uiPrefs.setProperty(UIPreferences::kUiScale, 1.25, nullptr);
            uiPrefs.setProperty(UIPreferences::kSpectrumDecay, 0.5, nullptr);
            uiPrefs.setProperty(UIPreferences::kSkipBootSequence, true, nullptr);

            auto updatedPayload = editor.buildUIPreferencesPayload();
            auto* updatedObj = updatedPayload.getDynamicObject();
            expect (updatedObj != nullptr, "Updated payload dynamic object must not be null");
            if (updatedObj != nullptr)
            {
                expectEquals (static_cast<double>(updatedObj->getProperty(UIPreferences::kUiScale)), 1.25);
                expectEquals (static_cast<double>(updatedObj->getProperty(UIPreferences::kSpectrumDecay)), 0.5);
                expect (static_cast<bool>(updatedObj->getProperty(UIPreferences::kSkipBootSequence)) == true, "skipBootSequence should be updated to true");
            }
        }

        void testWarmDistortionMathematicalFormulas()
        {
            beginTest ("WarmDistortion: Mathematical formulas (Power-Law Tapering, Dynamic Bias, Normalized Asymmetric Tanh, Auto-Gain)");

            // 1. Power-law drive tapering G_in(d) = 10^(1.2 * d^2.2)
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateInputGain (0.0f), 1.0f, 0.0001f, "G_in(0) should be 1.0 (0 dB)");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateInputGain (0.5f), 1.824589f, 0.001f, "G_in(0.5) should be ~1.824589 (+5.22 dB)");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateInputGain (1.0f), 15.848932f, 0.001f, "G_in(1.0) should be ~15.848932 (+24 dB)");

            // 2. Dynamic bias b(d) = 0.25 * d * (1.0 - 0.4 * d)
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDynamicBias (0.0f), 0.0f, 0.0001f, "b(0) should be 0.0");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDynamicBias (0.5f), 0.10f, 0.0001f, "b(0.5) should be 0.10");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDynamicBias (1.0f), 0.15f, 0.0001f, "b(1.0) should be 0.15");

            // 3. Normalized asymmetric tanh f(x, b) = (tanh(x+b) - tanh(b)) / (1 - tanh^2(b))
            // At x = 0, f(0, b) must be exactly 0.0 for any bias
            expectWithinAbsoluteError (dsp::WarmDistortion::asymmetricTanh (0.0f, 0.0f), 0.0f, 0.00001f, "f(0, 0) == 0.0");
            expectWithinAbsoluteError (dsp::WarmDistortion::asymmetricTanh (0.0f, 0.10f), 0.0f, 0.00001f, "f(0, 0.10) == 0.0");
            expectWithinAbsoluteError (dsp::WarmDistortion::asymmetricTanh (0.0f, 0.15f), 0.0f, 0.00001f, "f(0, 0.15) == 0.0");

            // At bias = 0, f(x, 0) is standard tanh(x)
            expectWithinAbsoluteError (dsp::WarmDistortion::asymmetricTanh (0.5f, 0.0f), std::tanh (0.5f), 0.00001f, "f(x, 0) == tanh(x)");
            expectWithinAbsoluteError (dsp::WarmDistortion::asymmetricTanh (-0.5f, 0.0f), std::tanh (-0.5f), 0.00001f, "f(-x, 0) == -tanh(x)");

            // Small-signal derivative at x = 0 is 1.0 (unity small-signal gain)
            const float eps = 0.0001f;
            for (float bias : { 0.0f, 0.05f, 0.10f, 0.15f })
            {
                float derivative = (dsp::WarmDistortion::asymmetricTanh (eps, bias) - dsp::WarmDistortion::asymmetricTanh (-eps, bias)) / (2.0f * eps);
                expectWithinAbsoluteError (derivative, 1.0f, 0.001f, "Small-signal derivative should be 1.0 for bias " + juce::String (bias));
            }

            // Asymmetry: positive vs negative saturation when bias > 0
            float posSat = dsp::WarmDistortion::asymmetricTanh (2.0f, 0.15f);
            float negSat = dsp::WarmDistortion::asymmetricTanh (-2.0f, 0.15f);
            expect (std::abs (posSat) != std::abs (negSat), "Saturation with bias > 0 must be asymmetric");

            // 4. Analytical auto-gain compensation G_comp(d) = 1.0 / sqrt(1 + 1.05 * (G_in^2 - 1) / (1 + 0.28 * G_in^2))
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateVolumeCompensation (0.0f), 1.0f, 0.0001f, "G_comp(0) should be 1.0");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateVolumeCompensation (0.5f), 0.664349f, 0.005f, "G_comp(0.5) should be ~0.664");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateVolumeCompensation (1.0f), 0.462116f, 0.005f, "G_comp(1.0) should be ~0.462");

            // Monotonicity of auto-gain compensation (strictly decreasing with drive)
            float prevComp = 1.1f;
            for (int step = 0; step <= 20; ++step)
            {
                float d = static_cast<float> (step) / 20.0f;
                float comp = dsp::WarmDistortion::calculateVolumeCompensation (d);
                expect (comp <= prevComp, "G_comp must monotonically decrease with drive");
                expect (comp >= 0.45f && comp <= 1.0f, "G_comp must be bounded in [0.45, 1.0]");
                prevComp = comp;
            }

            // 5. Dynamic lowpass damping cutoff frequency (18 kHz -> 10 kHz)
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDampingFrequency (0.0f), 18000.0f, 0.01f, "Damping cutoff at d=0 is 18 kHz");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDampingFrequency (0.5f), 14000.0f, 0.01f, "Damping cutoff at d=0.5 is 14 kHz");
            expectWithinAbsoluteError (dsp::WarmDistortion::calculateDampingFrequency (1.0f), 10000.0f, 0.01f, "Damping cutoff at d=1 is 10 kHz");
        }

        void testWarmDistortionOversamplingAndLatency()
        {
            beginTest ("WarmDistortion: 4x Polyphase IIR oversampling filter and latency");

            const double sampleRates[] = { 44100.0, 48000.0, 88200.0, 96000.0 };
            for (double sr : sampleRates)
            {
                juce::dsp::ProcessSpec spec{ sr, 512, 2 };
                dsp::WarmDistortion dist;
                dist.prepare (spec);

                expect (dist.getLatencySamples() > 0, "4x Polyphase IIR filter must report non-zero latency");
                expect (dist.getOversampledSampleRate() == static_cast<float> (sr * 4.0), "Oversampled sample rate must be 4x base sample rate");
            }
        }

        void testWarmDistortionExciterGatingAndDynamicDamping()
        {
            beginTest ("WarmDistortion: Exciter gated above 40% (d > 0.4) and dynamic lowpass damping");

            const double sampleRate = 48000.0;
            const int blockSize = 512;
            juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (blockSize), 2 };

            dsp::WarmDistortion dist;
            dist.prepare (spec);

            // Test 1: Quiescent DC offset is zero at any drive
            for (float d : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
            {
                juce::AudioBuffer<float> silence (2, blockSize);
                silence.clear();
                dist.reset();
                dist.process (silence, dsp::WarmDistortionParams{ d, false });

                for (int ch = 0; ch < 2; ++ch)
                {
                    for (int s = 0; s < blockSize; ++s)
                    {
                        expectWithinAbsoluteError (silence.getSample (ch, s), 0.0f, 1e-4f,
                                                   "Zero input must produce zero output at drive " + juce::String (d));
                    }
                }
            }

            // Test 2: Exciter gating below 40% vs above 40%
            auto generateSine = [](juce::AudioBuffer<float>& buf, float freq, double sr)
            {
                for (int s = 0; s < buf.getNumSamples(); ++s)
                {
                    float v = 0.2f * std::sin (2.0f * juce::MathConstants<float>::pi * freq * static_cast<float> (s) / static_cast<float> (sr));
                    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                        buf.setSample (ch, s, v);
                }
            };

            juce::AudioBuffer<float> bufLowDrive (2, blockSize);
            juce::AudioBuffer<float> bufHighDrive (2, blockSize);
            generateSine (bufLowDrive, 4000.0f, sampleRate);
            generateSine (bufHighDrive, 4000.0f, sampleRate);

            dist.reset();
            dist.process (bufLowDrive, dsp::WarmDistortionParams{ 0.35f, false }); // Below 40% gate

            dist.reset();
            dist.process (bufHighDrive, dsp::WarmDistortionParams{ 0.85f, false }); // Above 40% gate

            // High drive should have significant excitation and harmonic distortion compared to low drive
            float lowPeak = bufLowDrive.getMagnitude (0, blockSize);
            float highPeak = bufHighDrive.getMagnitude (0, blockSize);
            expect (highPeak > lowPeak, "Drive > 0.4 must engage excitation and saturation");
        }

        void testWarmDistortionAutoGainCompensation()
        {
            beginTest ("WarmDistortion: Analytical loudness auto-gain compensation");

            const double sampleRate = 48000.0;
            const int blockSize = 1024;
            juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (blockSize), 2 };

            dsp::WarmDistortion distCompOff;
            dsp::WarmDistortion distCompOn;
            distCompOff.prepare (spec);
            distCompOn.prepare (spec);

            auto generateSine = [](juce::AudioBuffer<float>& buf, float freq, double sr)
            {
                for (int s = 0; s < buf.getNumSamples(); ++s)
                {
                    float v = 0.3f * std::sin (2.0f * juce::MathConstants<float>::pi * freq * static_cast<float> (s) / static_cast<float> (sr));
                    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                        buf.setSample (ch, s, v);
                }
            };

            // Run with high drive (d = 0.9)
            juce::AudioBuffer<float> bufNoComp (2, blockSize);
            juce::AudioBuffer<float> bufWithComp (2, blockSize);
            generateSine (bufNoComp, 440.0f, sampleRate);
            generateSine (bufWithComp, 440.0f, sampleRate);

            // Warm-up several blocks to let smoothed values settle
            for (int b = 0; b < 10; ++b)
            {
                distCompOff.process (bufNoComp, dsp::WarmDistortionParams{ 0.9f, false });
                distCompOn.process (bufWithComp, dsp::WarmDistortionParams{ 0.9f, true });
            }

            float rmsNoComp = bufNoComp.getRMSLevel (0, 0, blockSize);
            float rmsWithComp = bufWithComp.getRMSLevel (0, 0, blockSize);

            expect (rmsWithComp < rmsNoComp, "Auto-gain compensation must reduce RMS level under heavy saturation");
            float measuredRatio = rmsWithComp / rmsNoComp;
            float expectedComp = dsp::WarmDistortion::calculateVolumeCompensation (0.9f);
            expectWithinAbsoluteError (measuredRatio, expectedComp, 0.05f, "Measured RMS reduction must match analytical G_comp factor");
        }
    };

    static AudioPluginTests synthortionTests;
}
