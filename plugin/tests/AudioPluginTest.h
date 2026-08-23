#pragma once

#include <JuceHeader.h>

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
        }
    private:
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
            auto& apvts = processor.getAPVTS();

            // Test handleMessage with COLOR parameter
            processor.handleMessage ("{\"parameterId\":\"COLOR\",\"value\":0.75}");
            if (auto* param = apvts.getParameter ("COLOR"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.75f, 0.001f, "COLOR normalized value should be 0.75");
            }

            // Test INPUT_GAIN: -60 dB -> 0.0, +12 dB -> 1.0, 0 dB -> (60/72)
            processor.handleMessage ("{\"parameterId\":\"INPUT_GAIN\",\"value\":0.0}");
            if (auto* param = apvts.getParameter ("INPUT_GAIN"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.0f, 0.001f, "INPUT_GAIN at 0.0 should be -60dB");
            }

            if (auto* param = apvts.getParameter ("INPUT_GAIN"))
            {
                const float zeroDbNormalized = param->getNormalisableRange().convertTo0to1 (0.0f);
                processor.handleMessage ("{\"parameterId\":\"INPUT_GAIN\",\"value\":" + juce::String (zeroDbNormalized) + "}");
                expectWithinAbsoluteError (param->getValue(), zeroDbNormalized, 0.001f, "INPUT_GAIN 0dB normalized value");
            }

            // Test PLUGIN_BYPASS
            processor.handleMessage ("{\"parameterId\":\"PLUGIN_BYPASS\",\"value\":1.0}");
            if (auto* param = apvts.getParameter ("PLUGIN_BYPASS"))
            {
                expectWithinAbsoluteError (param->getValue(), 1.0f, 0.001f, "PLUGIN_BYPASS should be active");
            }

            // Test handleMessage with spec 'id' key
            processor.handleMessage ("{\"id\":\"BITCRUSH\",\"value\":0.42}");
            if (auto* param = apvts.getParameter ("BITCRUSH"))
            {
                expectWithinAbsoluteError (param->getValue(), 0.42f, 0.001f, "BITCRUSH normalized value via 'id' should be 0.42");
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
    };

    static AudioPluginTests synthortionTests;
}
