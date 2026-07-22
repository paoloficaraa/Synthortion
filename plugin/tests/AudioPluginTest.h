#pragma once

#include <cmath>
#include <JuceHeader.h>
#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/AnimationController.h"
#include "Synthortion/AudioScopeRingBuffer.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/GlitchOverlay.h"
#include "Synthortion/MeterComponent.h"
#include "Synthortion/OscilloscopeComponent.h"
#include "Synthortion/PanelComponent.h"
#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class SynthortionLookAndFeelTests : public juce::UnitTest
{
public:
    SynthortionLookAndFeelTests()
        : juce::UnitTest("SynthortionLookAndFeel", "Synthortion")
    {
    }

    void runTest() override
    {
        const juce::MessageManagerLock mmLock;

        testBebasNeueFontRouting();
        testMontserratFontRouting();
        testTypographyScale();
    }

private:
    void testBebasNeueFontRouting()
    {
        beginTest("BebasNeue font routing returns a non-null typeface");

        SynthortionLookAndFeel lookAndFeel;
        auto typeface = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue"));

        expect(typeface != nullptr, "Expected non-null BebasNeue typeface");
    }

    void testMontserratFontRouting()
    {
        beginTest("Default sans-serif font routing returns Montserrat typeface");

        SynthortionLookAndFeel lookAndFeel;
        auto montserrat = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat"));
        auto defaultSans = lookAndFeel.getTypefaceForFont(
            juce::FontOptions().withName(juce::Font::getDefaultSansSerifFontName()));

        expect(montserrat != nullptr, "Expected non-null Montserrat typeface");
        expect(defaultSans == montserrat, "Expected default sans-serif to resolve to Montserrat");
    }

    void testTypographyScale()
    {
        beginTest("Typography scale: BebasNeue 22/14/16 headings+labels, Montserrat 12 numeric values per issue #28");

        SynthortionLookAndFeel lookAndFeel;
        const auto bebas = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue"));
        const auto montserrat = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat"));

        auto heading = lookAndFeel.getSectionHeadingFont();
        expect (juce::roundToInt(heading.getHeight()) == 22,
                "Section heading should be 22pt BebasNeue (panel titles) per issue #25");
        expect (heading.getTypefaceName().containsIgnoreCase("Bebas"),
                "Section heading should request the BebasNeue typeface");
        expect (lookAndFeel.getTypefaceForFont(heading) == bebas,
                "Section heading should resolve to the BebasNeue typeface");
        expect (std::abs (heading.getExtraKerningFactor() - SynthortionLookAndFeel::kTightKerning) < 1.0e-6f,
                "Section heading should apply -0.5 tight kerning per issue #25");

        auto paramLabel = lookAndFeel.getParameterLabelFont();
        expect (juce::roundToInt(paramLabel.getHeight()) == 14,
                "Parameter label should be 14pt BebasNeue (knob titles) per issue #25");
        expect (paramLabel.getTypefaceName().containsIgnoreCase("Bebas"),
                "Parameter label should request the BebasNeue typeface");
        expect (lookAndFeel.getTypefaceForFont(paramLabel) == bebas,
                "Parameter label should resolve to the BebasNeue typeface");
        expect (std::abs (paramLabel.getExtraKerningFactor() - SynthortionLookAndFeel::kTightKerning) < 1.0e-6f,
                "Parameter label should apply -0.5 tight kerning per issue #25");

        auto paramValue = lookAndFeel.getParameterValueFont();
        expect (juce::roundToInt(paramValue.getHeight()) == 12,
                "Parameter value should be 12pt Montserrat (numeric knob readouts) per issue #28");
        expect (paramValue.getTypefaceName().containsIgnoreCase("Montserrat"),
                "Parameter value should request the Montserrat typeface per issue #28");
        expect (lookAndFeel.getTypefaceForFont(paramValue) == montserrat,
                "Parameter value should resolve to the Montserrat typeface per issue #28");
        expect (std::abs (paramValue.getExtraKerningFactor()) < 1.0e-6f,
                "Parameter value (Montserrat) should not apply tight kerning per issue #28");

        auto bypass = lookAndFeel.getBypassLabelFont();
        expect (juce::roundToInt(bypass.getHeight()) == 16,
                "Bypass label should be 16pt BebasNeue (Block toggle) per issue #25");
        expect (bypass.getTypefaceName().containsIgnoreCase("Bebas"),
                "Bypass label should request the BebasNeue typeface");
        expect (lookAndFeel.getTypefaceForFont(bypass) == bebas,
                "Bypass label should resolve to the BebasNeue typeface");
        expect (std::abs (bypass.getExtraKerningFactor() - SynthortionLookAndFeel::kTightKerning) < 1.0e-6f,
                "Bypass label should apply -0.5 tight kerning per issue #25");
    }
};

static SynthortionLookAndFeelTests synthortionLookAndFeelTests;

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
            testBypassComponentLedTogglesOnClick();
            testBypassSwitchParameterAttachment();
            testBypassSwitchRendersActiveWhiteBlock();
            testBypassSwitchRendersBypassedBlackBlockWithOutline();
            testBypassSwitchFiresSliceGlitchOnToggle();
            testEditorIsOpaque();
            testPanelComponentIsOpaque();
            testDeadlockPalette();
            testCanvasBackgroundIsPitchBlackWithPermanentTexture();
            testEditorSizeIs800x480();
            testEditorContainsOscilloscope();
            testEditorContainsMeters();
            testAnimationControllerCreatesAnimator();
            testAudioScopeRingBufferTransfersSamples();
            testOscilloscopeReadsInputBuffer();
            testOscilloscopeReadsOutputBuffer();
            testOscilloscopeDetectsSilence();
            testOscilloscopeDetectsSignal();
            testOscilloscopeBypassFlattensOutput();
            testOscilloscopePixelLaneInterleave();
            testOscilloscopeTracesMergeIntoContinuousStreak();
            testOscilloscopeTripletRendersThreeParallelColumns();
            testOscilloscopeSweepRendersViaSharedOverlay();
            testOscilloscopeGhostTrailsUseThreeDiscreteAlphas();
            testOscilloscopeSilentCenterlineFlickersHard();
            testOscilloscopeBypassDecaysOutputAmplitude();
            testAnimatedKnobIsASlider();
            testAnimatedKnobHasRotaryStyle();
            testAnimatedKnobDefaultStyleIsCanonical();
            testAnimatedKnobStepCountMatchesStyle();
            testAnimatedKnobStepEasingQuantizesHard();
            testCanonicalKnobInvertsFaceOnMouseDown();
            testTwinShadowRendersOnDrag();
            testEditorKnobsUseFourCanonicalAndFourOutline();
            testEditorContainsEightAnimatedKnobs();
            testAnimatedKnobBindsToParameter();
            testInputGainKnobBindsToParameter();
            testOutputGainKnobBindsToParameter();
            testMeterCalculatesRMS();
            testMeterPeakHoldJumpsToPeak();
            testAnimationControllerBypassMixDefaultsToZero();
            testAnimationControllerBypassTransitionUsesStepEasing();
            testDeadlockKnobRendersWhiteFace();
            testMeterSegmentCountIsHardcodedSixteen();
            testMeterRendersSixteenSegmentLedLadder();
            testMeterSegmentsAreBinaryBlackWhite();
            testMeterBarHasWhiteOutlineWithNotchedTicks();
            testMeterPeakHoldMovesInSixteenSteps();
            testMeterPeakDecayUsesStepEasing();
            testMeterBypassShowsAllSegmentsOff();
            testEditorBackgroundUnaffectedByBypass();
            testBypassTransitionPropagatesToComponents();
            testGlitchOverlayDitherIsBinary();
            testGlitchOverlayTickAdvancesDitherFrame();
            testGlitchOverlayRerollsDeadPixelsApprox80ms();
            testGlitchOverlayDriftBandDriftsLeftToRight();
            testGlitchOverlayFlickerBlockTogglesEveryThirtyTicks();
            testGlitchOverlayTripletDrawsThreeOffsets();
            testGlitchOverlaySweepAdvancesLeftToRight();
            testGlitchOverlayBypassSliceBurstWindow();
            testGlitchOverlayDrawBypassSlicesRendersBandsWhenActive();
            testGlitchOverlayBootBurstFiresOnceAndRunsForFourHundredMs();
            testGlitchOverlayDrawBootBurstRendersFlashSlicesAndDeadPixels();
            testGlitchOverlayFlickerBurstFiresAtRareIntervals();
            testGlitchOverlayFlickerBurstResolvesAfterNormalDuration();
            testGlitchOverlayBypassAmplifiesFlickerInterval();
            testGlitchOverlayFlickerBurstBypassLastsLongerThanNormal();
            testGlitchOverlayDrawFlickerBurstRendersBandsAndDeadPixels();
            testGlitchOverlayBypassAmplifiesFlickerIntensity();
            testGlitchOverlayFlickerBurstCoexistsWithBypassSlices();
            testEditorBypassToggleAmplifiesGlitchOverlay();
            testPanelComponentRendersBrutalistShape();
            testPanelComponentTitleFontIsBebasNeueAllCaps();
            testEditorDrawsDashedSectionSeparators();
            testKnobValueLabelsUseMontserrat();
            testKnobValueLabelsDoNotOverlapKnobs();
            testAnimationControllerClearsBypassAnimatorOnTeardown();
            testEditorTeardownLifecycleIsCrashFree();
        }

    private:
        void testBypassComponentLedTogglesOnClick()
        {
            beginTest ("BypassComponent toggles to bypassed state on programmatic click");

            BypassComponent bypass;
            bypass.setSize (200, 80);

            expect (! bypass.isBypassed());

            bypass.getToggleButton().triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (bypass.isBypassed());
            expect (bypass.getToggleButton().getToggleState());
        }

        void testBypassSwitchParameterAttachment()
        {
            beginTest ("BypassSwitch is attached to PLUGIN_BYPASS");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto& bypassButton = editor.getBypassComponent().getToggleButton();

            expect (! bypassButton.getToggleState());

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (1.0f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (bypassButton.getToggleState(), "Button should follow PLUGIN_BYPASS parameter turning on");

            bypassButton.triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (processor.getAPVTS().getRawParameterValue ("PLUGIN_BYPASS")->load() < 0.5f,
                    "PLUGIN_BYPASS parameter should follow button click turning off");
        }

        void testEditorIsOpaque()
        {
            beginTest ("Plugin editor is opaque");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.isOpaque(), "Editor should be opaque for rendering performance");
        }

        void testPanelComponentIsOpaque()
        {
            beginTest ("PanelComponent is opaque");

            PanelComponent panel ("DISTORTION", juce::Colours::black);

            expect (panel.isOpaque(), "PanelComponent wrapper should be opaque for rendering performance");
        }

        void testDeadlockPalette()
        {
            beginTest ("DEADLOCK palette binds Canvas/Surface/Ink/Dimmed colour slots per issue #28");

            SynthortionLookAndFeel lookAndFeel;

            const auto background = lookAndFeel.findColour (SynthortionLookAndFeel::backgroundColourId);
            const auto panelFill = lookAndFeel.findColour (SynthortionLookAndFeel::panelFillColourId);
            const auto panelOutline = lookAndFeel.findColour (SynthortionLookAndFeel::panelOutlineColourId);
            const auto accent = lookAndFeel.findColour (SynthortionLookAndFeel::accentColourId);
            const auto accentBright = lookAndFeel.findColour (SynthortionLookAndFeel::accentBrightColourId);
            const auto text = lookAndFeel.findColour (SynthortionLookAndFeel::textColourId);
            const auto knobFill = lookAndFeel.findColour (SynthortionLookAndFeel::knobFillColourId);
            const auto surfaceAlt = lookAndFeel.findColour (SynthortionLookAndFeel::surfaceAltColourId);
            const auto dimmed = lookAndFeel.findColour (SynthortionLookAndFeel::dimmedColourId);

            const juce::Colour black (0xFF000000);
            const juce::Colour white (0xFFFFFFFF);
            const juce::Colour charcoal (0xFF0D0D0E);
            const juce::Colour surfaceAltCol (0xFF121214);
            const juce::Colour dimmedCol (0x66FFFFFF);

            expect (background == black, "backgroundColourId (Canvas) should be pure #000");
            expect (panelFill == charcoal, "panelFillColourId (Surface) should be #0D0D0E per issue #28");
            expect (panelOutline == white, "panelOutlineColourId (Ink) should be pure #FFF");
            expect (accent == white, "accentColourId (Ink) should be pure #FFF");
            expect (accentBright == white, "accentBrightColourId (Ink) should be pure #FFF");
            expect (text == white, "textColourId (Ink) should be pure #FFF");
            expect (knobFill == white, "knobFillColourId (Ink) should be pure #FFF");
            expect (surfaceAlt == surfaceAltCol, "surfaceAltColourId (Surface alt) should be #121214 per issue #28");
            expect (dimmed == dimmedCol, "dimmedColourId (Dimmed) should be #FFF at 0.4 alpha (0x66FFFFFF) per issue #28");
        }

        void testCanvasBackgroundIsPitchBlackWithPermanentTexture()
        {
            beginTest ("Canvas background is solid pitch black #000 with permanent dither/scanline texture per issue #28");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.lookAndFeel.findColour (SynthortionLookAndFeel::backgroundColourId) == juce::Colour (0xFF000000),
                    "Editor LookAndFeel backgroundColourId (Canvas) should be pure #000");

            editor.repaint();
            const auto snapshot = editor.createComponentSnapshot (editor.getLocalBounds());

            bool hasBlack = false;
            bool hasWhite = false;
            for (int y = 95; y < 100 && ! (hasBlack && hasWhite); ++y)
            {
                for (int x = 200; x < 600; ++x)
                {
                    const auto c = snapshot.getPixelAt (x, y);
                    if (c == juce::Colour (0xFF000000))
                        hasBlack = true;
                    else if (c == juce::Colour (0xFFFFFFFF))
                        hasWhite = true;
                }
            }

            expect (hasBlack, "Canvas substrate must contain pure #000 base pixels");
            expect (hasWhite, "Canvas substrate must contain permanent dither/scanline #FFF texture pixels");
        }

        void testEditorSizeIs800x480()
        {
            beginTest ("Plugin editor dimensions are 800x480");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 800, "Editor width should be 800 per DEADLOCK Slice I issue #26");
            expect (editor.getHeight() == 480, "Editor height should be 480 per DEADLOCK Slice I issue #26");
        }

        void testEditorContainsOscilloscope()
        {
            beginTest ("Plugin editor contains an OscilloscopeComponent");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int oscilloscopeCount = 0;
            for (auto* child : editor.getChildren())
                if (dynamic_cast<OscilloscopeComponent*> (child) != nullptr)
                    ++oscilloscopeCount;

            expect (oscilloscopeCount == 1, "Editor should contain exactly one OscilloscopeComponent");
        }

        void testEditorContainsMeters()
        {
            beginTest ("Plugin editor contains two MeterComponents");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int meterCount = 0;
            for (auto* child : editor.getChildren())
                if (dynamic_cast<MeterComponent*> (child) != nullptr)
                    ++meterCount;

            expect (meterCount == 2, "Editor should contain exactly two MeterComponents");
        }

        void testAnimationControllerCreatesAnimator()
        {
            beginTest ("AnimationController creates a ValueAnimator with the expected duration");

            juce::Component dummy;
            AnimationController controller (&dummy);

            auto animator = controller.runAnimator (
                juce::ValueAnimatorBuilder()
                    .withValueChangedCallback ([] (float) {})
                    .withDurationMs (150)
                    .withEasing (juce::Easings::createLinear()));

            expect (std::abs (animator.getDurationMs() - 150.0) < 1.0e-6, "Animator duration should match builder");

            juce::ignoreUnused (controller);
        }

        void testAudioScopeRingBufferTransfersSamples()
        {
            beginTest ("AudioScopeRingBuffer transfers stereo samples");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            writeBuffer.setSample (1, 20, -0.25f);

            buffer.writeInput (writeBuffer);

            juce::AudioBuffer<float> readBuffer (2, 128);
            readBuffer.clear();
            const int read = buffer.readInput (readBuffer);

            expect (read == 64, "Should read back the written sample count");
            expect (std::abs (readBuffer.getSample (0, read - 64 + 10) - 0.5f) < 1.0e-6f, "Channel 0 sample should match");
            expect (std::abs (readBuffer.getSample (1, read - 64 + 20) - -0.25f) < 1.0e-6f, "Channel 1 sample should match");
        }

        void testOscilloscopeReadsInputBuffer()
        {
            beginTest ("OscilloscopeComponent reads the input ring buffer");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            writeBuffer.setSample (1, 20, -0.25f);

            buffer.writeInput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            const auto& input = scope.getCurrentInputBuffer();
            expect (input.getNumChannels() == 2, "Oscilloscope should read a stereo input buffer");
            expect (input.getNumSamples() == OscilloscopeComponent::kWindowSize, "Oscilloscope input buffer should match the window size");
            expect (std::abs (input.getSample (0, 10) - 0.5f) < 1.0e-6f, "Input channel 0 sample should match");
            expect (std::abs (input.getSample (1, 20) - -0.25f) < 1.0e-6f, "Input channel 1 sample should match");
        }

        void testOscilloscopeReadsOutputBuffer()
        {
            beginTest ("OscilloscopeComponent reads the output ring buffer");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 5, 0.75f);
            writeBuffer.setSample (1, 15, -0.5f);

            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            const auto& output = scope.getCurrentOutputBuffer();
            expect (output.getNumChannels() == 2, "Oscilloscope should read a stereo output buffer");
            expect (output.getNumSamples() == OscilloscopeComponent::kWindowSize, "Oscilloscope output buffer should match the window size");
            expect (std::abs (output.getSample (0, 5) - 0.75f) < 1.0e-6f, "Output channel 0 sample should match");
            expect (std::abs (output.getSample (1, 15) - -0.5f) < 1.0e-6f, "Output channel 1 sample should match");
        }

        void testOscilloscopeDetectsSilence()
        {
            beginTest ("OscilloscopeComponent reports silence for zero buffers");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            buffer.writeInput (writeBuffer);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (scope.isSilent(), "Oscilloscope should report silence for zero buffers");
        }

        void testOscilloscopeDetectsSignal()
        {
            beginTest ("OscilloscopeComponent reports non-silence for active buffers");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            buffer.writeInput (writeBuffer);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (! scope.isSilent(), "Oscilloscope should report non-silence for active buffers");
        }

        void testOscilloscopeBypassFlattensOutput()
        {
            beginTest ("OscilloscopeComponent enters bypassed flatline state");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.8f);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (! scope.isBypassed(), "Oscilloscope should start un-bypassed");

            scope.setBypassed (true);
            scope.refresh();

            expect (scope.isBypassed(), "Oscilloscope should report bypassed after setBypassed(true)");
        }

        static juce::AudioBuffer<float> makeStereoSignal (int numSamples, float value)
        {
            juce::AudioBuffer<float> b (2, numSamples);
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < numSamples; ++i)
                    b.setSample (ch, i, value);
            return b;
        }

        static juce::AudioBuffer<float> makeStereoSpike (int numSamples, int spikeIndex, float value)
        {
            juce::AudioBuffer<float> b (2, numSamples);
            for (int ch = 0; ch < 2; ++ch)
                b.setSample (ch, spikeIndex, value);
            return b;
        }

        void testOscilloscopePixelLaneInterleave()
        {
            beginTest ("Oscilloscope traces interleave: input on odd rows, output on even rows, pure #FFF, no AA");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            // Input-only signal: input = +1.0 (full scale), output = 0.0.
            // input trace should paint odd rows; even rows (away from midY) stay #000.
            {
                AudioScopeRingBuffer buffer (512);
                buffer.writeInput (makeStereoSignal (512, 1.0f));
                buffer.writeOutput (makeStereoSignal (512, 0.0f));

                OscilloscopeComponent scope (buffer, nullptr);
                scope.setSize (100, 100);
                GlitchOverlay overlay;
                scope.setGlitchOverlay (&overlay);

                for (int i = 0; i < 3; ++i)
                    scope.refresh();

                const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

                expect (snap.getPixelAt (50, 49) == white,
                        "Input trace should paint odd rows in pure #FFF (no anti-aliasing, no violet)");
                expect (snap.getPixelAt (50, 48) == black,
                        "Even rows outside the output stroke should stay #000 (no input bleed onto even rows)");
            }

            // Output-only signal: input = 0.0, output = +1.0.
            // output trace should paint even rows; odd rows stay #000.
            {
                AudioScopeRingBuffer buffer (512);
                buffer.writeInput (makeStereoSignal (512, 0.0f));
                buffer.writeOutput (makeStereoSignal (512, 1.0f));

                OscilloscopeComponent scope (buffer, nullptr);
                scope.setSize (100, 100);
                GlitchOverlay overlay;
                scope.setGlitchOverlay (&overlay);

                for (int i = 0; i < 3; ++i)
                    scope.refresh();

                const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

                expect (snap.getPixelAt (50, 48) == white,
                        "Output trace should paint even rows in pure #FFF");
                expect (snap.getPixelAt (50, 49) == black,
                        "Odd rows outside the input stroke should stay #000 (no output bleed onto odd rows)");
            }
        }

        void testOscilloscopeTracesMergeIntoContinuousStreak()
        {
            beginTest ("Oscilloscope input==output traces merge into a continuous vertical streak");

            const auto white = juce::Colour (0xFFFFFFFF);

            AudioScopeRingBuffer buffer (512);
            buffer.writeInput (makeStereoSignal (512, 1.0f));
            buffer.writeOutput (makeStereoSignal (512, 1.0f));

            OscilloscopeComponent scope (buffer, nullptr);
            scope.setSize (100, 100);
            GlitchOverlay overlay;
            scope.setGlitchOverlay (&overlay);

            for (int i = 0; i < 3; ++i)
                scope.refresh();

            const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

            expect (snap.getPixelAt (50, 49) == white,
                    "Odd rows should be #FFF from the input trace");
            expect (snap.getPixelAt (50, 48) == white,
                    "Even rows should be #FFF from the output trace; together they form a continuous streak");
        }

        void testOscilloscopeTripletRendersThreeParallelColumns()
        {
            beginTest ("Oscilloscope trace renders as a Triplet of 3 parallel columns via GlitchOverlay");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            AudioScopeRingBuffer buffer (512);
            // Single-spike output so only one column carries a tall stroke; the
            // Triplet then spreads that stroke to x-1, x, x+1.
            buffer.writeInput (makeStereoSignal (512, 0.0f));
            buffer.writeOutput (makeStereoSpike (512, 255, 1.0f));

            OscilloscopeComponent scope (buffer, nullptr);
            scope.setSize (100, 100);
            GlitchOverlay overlay;
            scope.setGlitchOverlay (&overlay);

            for (int i = 0; i < 3; ++i)
                scope.refresh();

            const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

            expect (snap.getPixelAt (49, 48) == white, "Triplet left copy should render at x-1");
            expect (snap.getPixelAt (50, 48) == white, "Triplet centre copy should render at x");
            expect (snap.getPixelAt (51, 48) == white, "Triplet right copy should render at x+1");
            expect (snap.getPixelAt (48, 48) == black, "No triplet stroke one column left of the triplet");
            expect (snap.getPixelAt (52, 48) == black, "No triplet stroke one column right of the triplet");
        }

        void testOscilloscopeSweepRendersViaSharedOverlay()
        {
            beginTest ("Oscilloscope renders the scan-charge Sweep band through the shared GlitchOverlay");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            AudioScopeRingBuffer buffer (512);
            // Small-amplitude signal keeps the trace near midY so the top of the
            // scope is clear except for the Sweep band.
            buffer.writeInput (makeStereoSignal (512, 0.1f));
            buffer.writeOutput (makeStereoSignal (512, 0.1f));

            OscilloscopeComponent scope (buffer, nullptr);
            scope.setSize (100, 100);
            GlitchOverlay overlay;
            scope.setGlitchOverlay (&overlay);

            for (int i = 0; i < 3; ++i)
                scope.refresh();

            const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

            // Fresh overlay => sweep position 0 => 4px band at the left edge.
            expect (snap.getPixelAt (1, 10) == white,
                    "Sweep band should render #FFF at the left edge at position 0");
            expect (snap.getPixelAt (5, 10) == black,
                    "Outside the sweep band and away from the trace the substrate should stay #000");
        }

        void testOscilloscopeGhostTrailsUseThreeDiscreteAlphas()
        {
            beginTest ("Oscilloscope ghost trails use 3 hard discrete opacity levels (no linear fade)");

            const auto alphas = OscilloscopeComponent::ghostAlphasForTests();

            expect (std::abs (alphas[0] - 0.60f) < 1.0e-6f, "Most-recent ghost trail should be 0.60 alpha");
            expect (std::abs (alphas[1] - 0.30f) < 1.0e-6f, "Middle ghost trail should be 0.30 alpha");
            expect (std::abs (alphas[2] - 0.15f) < 1.0e-6f, "Oldest ghost trail should be 0.15 alpha");
            expect (alphas[0] > alphas[1] && alphas[1] > alphas[2],
                    "Ghost trail alphas must be 3 descending discrete steps, not a linear fade formula");
        }

        void testOscilloscopeSilentCenterlineFlickersHard()
        {
            beginTest ("Oscilloscope silent centerline flickers on/off at 2 hard steps (no sine-modulated breath)");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            AudioScopeRingBuffer buffer (512);
            buffer.writeInput (makeStereoSignal (512, 0.0f));
            buffer.writeOutput (makeStereoSignal (512, 0.0f));

            OscilloscopeComponent scope (buffer, nullptr);
            scope.setSize (100, 100); // midY = 50

            scope.refresh();
            const auto onSnap = scope.createComponentSnapshot (scope.getLocalBounds());
            expect (onSnap.getPixelAt (50, 50) == white,
                    "Centerline should be hard full #FFF when on (no sine-modulated dim)");

            for (int i = 0; i < 29; ++i)
                scope.refresh();
            const auto offSnap = scope.createComponentSnapshot (scope.getLocalBounds());
            expect (offSnap.getPixelAt (50, 50) == black,
                    "Centerline should be hard full off (#000) at the second flicker step");

            for (int i = 0; i < 25; ++i)
                scope.refresh();
            const auto onSnap2 = scope.createComponentSnapshot (scope.getLocalBounds());
            expect (onSnap2.getPixelAt (50, 50) == white,
                    "Centerline should toggle back to hard #FFF (2-step flicker)");
        }

        void testOscilloscopeBypassDecaysOutputAmplitude()
        {
            beginTest ("Oscilloscope bypass decays output trace amplitude; trace is pure #FFF (kTraceGrey gone)");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            AudioScopeRingBuffer buffer (512);
            buffer.writeInput (makeStereoSignal (512, 0.0f));
            buffer.writeOutput (makeStereoSignal (512, 1.0f));

            OscilloscopeComponent scope (buffer, nullptr);
            scope.setSize (100, 100);

            scope.refresh(); // un-bypassed: flatlineAmplitude -> 1.0
            scope.setBypassed (true);
            scope.refresh(); // bypassed: flatlineAmplitude -> 1.0 * 0.92 = 0.92

            const auto snap = scope.createComponentSnapshot (scope.getLocalBounds());

            expect (snap.getPixelAt (50, 48) == white,
                    "Bypassed output trace should still render in pure #FFF (kTraceGrey removed)");
            expect (snap.getPixelAt (50, 16) == black,
                    "Bypassed output trace amplitude should decay so the stroke top sits below the full-scale position");
        }

        int countAnimatedKnobsRecursive (juce::Component& parent)
        {
            int count = 0;

            for (auto* child : parent.getChildren())
            {
                if (dynamic_cast<AnimatedKnob*> (child) != nullptr)
                    ++count;

                count += countAnimatedKnobsRecursive (*child);
            }

            return count;
        }

        void testAnimatedKnobIsASlider()
        {
            beginTest ("AnimatedKnob inherits from juce::Slider");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            juce::ignoreUnused (knob);

            expect (std::is_base_of_v<juce::Slider, AnimatedKnob>,
                    "AnimatedKnob should inherit from juce::Slider so attachments work");
        }

        void testAnimatedKnobHasRotaryStyle()
        {
            beginTest ("AnimatedKnob uses a rotary style with no text box");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            expect (knob.getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag,
                    "AnimatedKnob should use RotaryHorizontalVerticalDrag");
            expect (knob.getTextBoxPosition() == juce::Slider::NoTextBox,
                    "AnimatedKnob should hide its text box");
        }

        void testEditorContainsEightAnimatedKnobs()
        {
            beginTest ("Plugin editor contains eight AnimatedKnob instances");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            const int knobCount = countAnimatedKnobsRecursive (editor);
            expect (knobCount == 8, "Editor should contain exactly eight AnimatedKnobs");
        }

        juce::AudioProcessorParameter* findParameterById (juce::AudioProcessor& processor,
                                                           const juce::String& paramId)
        {
            for (auto* param : processor.getParameters())
            {
                if (auto* paramWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
                {
                    if (paramWithId->getParameterID() == paramId)
                        return param;
                }
            }

            return nullptr;
        }

        void collectKnobsAtValue (juce::Component& parent, float value, int& count)
        {
            for (auto* child : parent.getChildren())
            {
                if (auto* knob = dynamic_cast<AnimatedKnob*> (child))
                {
                    if (std::abs (static_cast<float> (knob->getValue()) - value) < 1.0e-6f)
                        ++count;
                }

                collectKnobsAtValue (*child, value, count);
            }
        }

        void testAnimatedKnobBindsToParameter()
        {
            beginTest ("AnimatedKnob binds to an APVTS parameter via SliderAttachment");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* colorParameter = findParameterById (processor, "COLOR");
            jassert (colorParameter != nullptr);
            colorParameter->setValueNotifyingHost (0.75f);

            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            int knobsAtTarget = 0;
            for (auto* child : editor.getChildren())
                collectKnobsAtValue (*child, colorParameter->getValue(), knobsAtTarget);

            expect (knobsAtTarget > 0,
                    "At least one AnimatedKnob in the editor should follow the COLOR parameter");
        }

        void testInputGainKnobBindsToParameter()
        {
            beginTest ("Input gain knob follows the INPUT_GAIN parameter");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* inputGain = findParameterById (processor, "INPUT_GAIN");
            jassert (inputGain != nullptr);

            const float targetDb = -12.0f;
            const float normalized = processor.getAPVTS().getParameterRange ("INPUT_GAIN").convertTo0to1 (targetDb);
            inputGain->setValueNotifyingHost (normalized);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (std::abs (static_cast<float> (editor.getInputGainKnob().getValue()) - targetDb) < 0.1f,
                    "Input gain knob should reflect -12 dB");
        }

        void testOutputGainKnobBindsToParameter()
        {
            beginTest ("Output gain knob follows the OUTPUT_GAIN parameter");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* outputGain = findParameterById (processor, "OUTPUT_GAIN");
            jassert (outputGain != nullptr);

            const float targetDb = -36.0f;
            const float normalized = processor.getAPVTS().getParameterRange ("OUTPUT_GAIN").convertTo0to1 (targetDb);
            outputGain->setValueNotifyingHost (normalized);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (std::abs (static_cast<float> (editor.getOutputGainKnob().getValue()) - targetDb) < 0.1f,
                    "Output gain knob should reflect -36 dB");
        }

        void testMeterCalculatesRMS()
        {
            beginTest ("MeterComponent calculates RMS from a stereo buffer");

            juce::Component dummy;
            AnimationController controller (&dummy);
            MeterComponent meter (controller);
            meter.setSize (40, 200);

            juce::AudioBuffer<float> buffer (2, 64);
            buffer.clear();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                    buffer.setSample (ch, i, 0.5f);

            meter.updateFromBuffer (buffer);

            const float expectedDb = juce::Decibels::gainToDecibels (0.5f);
            expect (std::abs (meter.getRmsDb() - expectedDb) < 0.1f,
                    "RMS should be -6 dB for a 0.5 amplitude signal");
            expect (std::abs (meter.getPeakDb() - expectedDb) < 0.1f,
                    "Peak should match the constant amplitude");
        }

        void testMeterPeakHoldJumpsToPeak()
        {
            beginTest ("MeterComponent peak hold jumps to a transient peak within one segment and starts decay animation");

            juce::Component dummy;
            AnimationController controller (&dummy);
            MeterComponent meter (controller);
            meter.setSize (40, 200);

            juce::AudioBuffer<float> buffer (2, 32);
            buffer.clear();
            buffer.setSample (0, 4, 0.8f);
            buffer.setSample (1, 20, -0.8f);

            meter.updateFromBuffer (buffer);

            const float expectedDb = juce::Decibels::gainToDecibels (0.8f);
            const float meterRange = 6.0f - (-60.0f);
            const float tolerance = meterRange / static_cast<float> (MeterComponent::kSegmentCount);
            expect (std::abs (meter.getAnimatedPeakDb() - expectedDb) < tolerance,
                    "Animated peak hold should jump to within one segment (1/16 of meter range) of the transient peak");
            expect (meter.isPeakHoldAnimating(),
                    "A decay animator should be running after a new peak");
        }

        void testAnimationControllerBypassMixDefaultsToZero()
        {
            beginTest ("AnimationController bypass mix defaults to zero");

            juce::Component dummy;
            AnimationController controller (&dummy);

            expect (std::abs (controller.getBypassMix()) < 1.0e-6f, "Bypass mix should start fully active");

            controller.setBypassMix (0.75f);
            expect (std::abs (controller.getBypassMix() - 0.75f) < 1.0e-6f,
                    "setBypassMix should update the mix value");
        }

        void testAnimationControllerBypassTransitionUsesStepEasing()
        {
            beginTest ("AnimationController bypass transition easing quantises to N=8 hard steps");

            expect (AnimationController::kBypassTransitionSteps == 8,
                    "Bypass transition Step count must be N=8 per DEADLOCK Slice I issue #26");

            expect (std::abs (AnimationController::quantizeBypassProgress (0.0f) - 0.0f) < 1.0e-6f,
                    "progress 0 should quantise to step 0/8");
            expect (std::abs (AnimationController::quantizeBypassProgress (1.0f) - 1.0f) < 1.0e-6f,
                    "progress 1 should quantise to step 8/8");
            expect (std::abs (AnimationController::quantizeBypassProgress (0.5f) - 0.5f) < 1.0e-6f,
                    "progress 0.5 should quantise to step 4/8 = 0.5");
            expect (std::abs (AnimationController::quantizeBypassProgress (0.59f) - 0.625f) < 1.0e-6f,
                    "progress 0.59 should quantise up to step 5/8 = 0.625");
            expect (std::abs (AnimationController::quantizeBypassProgress (0.03f) - 0.0f) < 1.0e-6f,
                    "progress 0.03 should quantise down to step 0/8 = 0");
            expect (std::abs (AnimationController::quantizeBypassProgress (0.10f) - 0.125f) < 1.0e-6f,
                    "progress 0.10 should quantise up to step 1/8 = 0.125");
            expect (std::abs (AnimationController::quantizeBypassProgress (0.34f) - 0.375f) < 1.0e-6f,
                    "progress 0.34 should quantise to step 3/8 = 0.375");

            // The 300 ms duration is preserved from the pre-Slice-I transition;
            // the easing is flipped from createEaseOut() to the quantised Step
            // above. The duration is exercised end-to-end only in a host with a
            // visible VBlank component; the unit test scope is the easing shape.
            juce::Component dummy;
            AnimationController controller (&dummy);
            controller.startBypassTransition (true);
            expect (controller.getBypassMix() >= 0.0f && controller.getBypassMix() <= 1.0f,
                    "Bypass mix should stay in [0, 1] after a transition is started");
        }

        void testDeadlockKnobRendersWhiteFace()
        {
            beginTest ("Knob renders a flat white canonical face in the DEADLOCK palette");

            juce::Component dummy;
            AnimationController controller (&dummy);
            SynthortionLookAndFeel lookAndFeel;
            lookAndFeel.setBypassMix (0.0f);

            AnimatedKnob knob (controller);
            knob.setRange (0.0, 1.0);
            knob.setValue (1.0);
            knob.snapToCurrentValue();
            knob.setSize (60, 60);
            knob.setLookAndFeel (&lookAndFeel);

            const auto snapshot = knob.createComponentSnapshot (knob.getLocalBounds());
            const auto facePixel = snapshot.getPixelAt (15, 30);

            expect (facePixel.getBrightness() > 0.9f,
                    "Canonical knob face should render pure white in the binary DEADLOCK palette");
            expect (std::abs (lookAndFeel.getBypassMix()) < 1.0e-6f,
                    "LnF bypass mix should default to the active state");
        }

        void testMeterSegmentCountIsHardcodedSixteen()
        {
            beginTest ("MeterComponent segment count is hardcoded N=16 in the header");

            expect (MeterComponent::kSegmentCount == 16,
                    "Meter segment count must be the hardcoded N=16 constant per issue #24");
        }

        void testMeterRendersSixteenSegmentLedLadder()
        {
            beginTest ("MeterComponent renders a 16-segment LED ladder filling from the bottom up");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            juce::Component dummy;
            AnimationController controller (&dummy);

            juce::AudioBuffer<float> signalBuffer (2, 64);
            signalBuffer.clear();
            for (int ch = 0; ch < signalBuffer.getNumChannels(); ++ch)
                for (int i = 0; i < signalBuffer.getNumSamples(); ++i)
                    signalBuffer.setSample (ch, i, 0.5f); // rms ~= -6 dB -> 14 lit segments

            MeterComponent meter (controller);
            meter.setSize (40, 160); // segmentHeight = 10 px exactly
            meter.updateFromBuffer (signalBuffer);

            const auto snap = meter.createComponentSnapshot (meter.getLocalBounds());

            // barX = 10, barW = 20, barBottom = 160, segmentHeight = 10.
            // Segment 8 (from bottom) spans y in [70, 79] and is ON for 14 lit segments.
            expect (snap.getPixelAt (20, 75) == white,
                    "Mid-bar pixel inside a lit segment should be pure #FFF (LED ON)");
            // Segment 14 spans y in [10, 19] and is OFF for 14 lit segments.
            expect (snap.getPixelAt (20, 15) == black,
                    "Pixel inside an unlit segment above the RMS level should be #000 (LED OFF)");
            // Segment 15 (top) spans y in [0, 9] and is OFF.
            expect (snap.getPixelAt (20, 5) == black,
                    "Top segment above the RMS level should be #000 (LED OFF)");

            juce::AudioBuffer<float> silentBuffer (2, 64);
            silentBuffer.clear();
            MeterComponent silentMeter (controller);
            silentMeter.setSize (40, 160);
            silentMeter.updateFromBuffer (silentBuffer);

            const auto silentSnap = silentMeter.createComponentSnapshot (silentMeter.getLocalBounds());
            expect (silentSnap.getPixelAt (20, 75) == black,
                    "Silent meter should render all segments OFF (#000)");
            expect (silentSnap.getPixelAt (20, 5) == black,
                    "Silent meter top segment should also be OFF (#000)");
        }

        void testMeterSegmentsAreBinaryBlackWhite()
        {
            beginTest ("MeterComponent segments render as binary #FFF or #000 (no alpha-blended greys)");

            juce::Component dummy;
            AnimationController controller (&dummy);

            juce::AudioBuffer<float> signalBuffer (2, 64);
            signalBuffer.clear();
            for (int ch = 0; ch < signalBuffer.getNumChannels(); ++ch)
                for (int i = 0; i < signalBuffer.getNumSamples(); ++i)
                    signalBuffer.setSample (ch, i, 0.3f); // mid-level signal

            MeterComponent meter (controller);
            meter.setSize (40, 160);
            meter.updateFromBuffer (signalBuffer);

            const auto snap = meter.createComponentSnapshot (meter.getLocalBounds());

            int greyPixelCount = 0;
            for (int y = 0; y < 160; ++y)
            {
                const auto pixel = snap.getPixelAt (20, y); // mid-bar column (no outline/tick overlap)
                const auto brightness = pixel.getBrightness();
                if (brightness > 0.01f && brightness < 0.99f)
                    ++greyPixelCount;
            }

            expect (greyPixelCount == 0,
                    "Mid-bar column should contain only hard #FFF or #000 pixels, no alpha-blended greys");
        }

        void testMeterBarHasWhiteOutlineWithNotchedTicks()
        {
            beginTest ("MeterComponent bar has a 1px #FFF outline with notched reference ticks at -6/-12/-24 dB");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            juce::Component dummy;
            AnimationController controller (&dummy);

            juce::AudioBuffer<float> signalBuffer (2, 64);
            signalBuffer.clear();
            for (int ch = 0; ch < signalBuffer.getNumChannels(); ++ch)
                for (int i = 0; i < signalBuffer.getNumSamples(); ++i)
                    signalBuffer.setSample (ch, i, 0.5f);

            MeterComponent meter (controller);
            meter.setSize (40, 160);
            meter.updateFromBuffer (signalBuffer);

            const auto snap = meter.createComponentSnapshot (meter.getLocalBounds());

            // barX = 10, barW = 20 -> left outline at x = 10, right outline at x = 29.
            expect (snap.getPixelAt (10, 80) == white,
                    "Left outline pixel should be #FFF");
            expect (snap.getPixelAt (29, 80) == white,
                    "Right outline pixel should be #FFF");
            expect (snap.getPixelAt (20, 0) == white,
                    "Top outline pixel should be #FFF");
            expect (snap.getPixelAt (20, 159) == white,
                    "Bottom outline pixel should be #FFF");

            // Notched ticks: tickY = roundToInt(160 - levelToHeight(db)).
            // -6 dB  -> tickY = 29, -12 dB -> tickY = 44, -24 dB -> tickY = 73.
            expect (snap.getPixelAt (10, 29) == black,
                    "-6 dB notch should carve a 1px #000 gap in the left outline");
            expect (snap.getPixelAt (29, 29) == black,
                    "-6 dB notch should carve a 1px #000 gap in the right outline");
            expect (snap.getPixelAt (10, 28) == white,
                    "Left outline just above the -6 dB notch should remain #FFF");
            expect (snap.getPixelAt (10, 30) == white,
                    "Left outline just below the -6 dB notch should remain #FFF");

            expect (snap.getPixelAt (10, 44) == black,
                    "-12 dB notch should carve a 1px #000 gap in the left outline");
            expect (snap.getPixelAt (29, 44) == black,
                    "-12 dB notch should carve a 1px #000 gap in the right outline");

            expect (snap.getPixelAt (10, 73) == black,
                    "-24 dB notch should carve a 1px #000 gap in the left outline");
            expect (snap.getPixelAt (29, 73) == black,
                    "-24 dB notch should carve a 1px #000 gap in the right outline");

            // No stray tick lines floating outside the bar.
            expect (snap.getPixelAt (9, 29) == black,
                    "Outside the left outline there should be no stray tick line (#000 substrate)");
            expect (snap.getPixelAt (30, 29) == black,
                    "Outside the right outline there should be no stray tick line (#000 substrate)");
        }

        void testMeterPeakHoldMovesInSixteenSteps()
        {
            beginTest ("MeterComponent peak hold maps dB to discrete 1/16 segment indices (no smooth slide)");

            juce::Component dummy;
            AnimationController controller (&dummy);
            MeterComponent meter (controller);
            meter.setSize (40, 160);

            expect (meter.dbToSegmentIndex (-60.0f) == 0,
                    "kMinDb should map to segment index 0 (no marker, no lit segments)");
            expect (meter.dbToSegmentIndex (6.0f) == MeterComponent::kSegmentCount,
                    "kMaxDb should map to segment index 16 (all segments lit)");
            // -6 dB  -> t = 54/66 -> scaled = 13.09 -> ceil = 14
            expect (meter.dbToSegmentIndex (-6.0f) == 14,
                    "-6 dB should map to segment index 14 (peak marker at the 14th step boundary)");
            // -12 dB -> t = 48/66 -> scaled = 11.64 -> ceil = 12
            expect (meter.dbToSegmentIndex (-12.0f) == 12,
                    "-12 dB should map to segment index 12");
            // -24 dB -> t = 36/66 -> scaled = 8.73 -> ceil = 9
            expect (meter.dbToSegmentIndex (-24.0f) == 9,
                    "-24 dB should map to segment index 9");

            // The marker position in paint is barBottom - peakSeg * segmentHeight, i.e. a 1/16 step.
            // For peakSeg = 14 and barH = 160, markerY = 160 - 14*10 = 20 (a 1/16 step boundary).
            juce::AudioBuffer<float> buffer (2, 32);
            buffer.clear();
            buffer.setSample (0, 4, 0.5f);
            buffer.setSample (1, 20, -0.5f);
            meter.updateFromBuffer (buffer);

            const auto snap = meter.createComponentSnapshot (meter.getLocalBounds());
            const auto white = juce::Colour (0xFFFFFFFF);
            // peakSeg = 14 -> markerY = 20; the marker is a 1px #FFF line at y = 20 across the bar.
            expect (snap.getPixelAt (20, 20) == white,
                    "Peak hold marker should render as a 1px #FFF line at the quantised 1/16 step boundary");
        }

        void testMeterPeakDecayUsesStepEasing()
        {
            beginTest ("MeterComponent peak decay uses a quantised Step easing with N=16 (no smooth slide)");

            expect (std::abs (MeterComponent::quantizeStepProgress (0.0f, 16) - 0.0f) < 1.0e-6f,
                    "progress 0 should quantise to step 0/16");
            expect (std::abs (MeterComponent::quantizeStepProgress (1.0f, 16) - 1.0f) < 1.0e-6f,
                    "progress 1 should quantise to step 16/16");
            expect (std::abs (MeterComponent::quantizeStepProgress (0.5f, 16) - 0.5f) < 1.0e-6f,
                    "progress 0.5 should quantise to step 8/16 = 0.5");
            expect (std::abs (MeterComponent::quantizeStepProgress (0.55f, 16) - 0.5625f) < 1.0e-6f,
                    "progress 0.55 should quantise up to step 9/16 = 0.5625");
            expect (std::abs (MeterComponent::quantizeStepProgress (0.03f, 16) - 0.0f) < 1.0e-6f,
                    "progress 0.03 should quantise down to step 0/16 = 0");
            expect (std::abs (MeterComponent::quantizeStepProgress (0.04f, 16) - 0.0625f) < 1.0e-6f,
                    "progress 0.04 should quantise up to step 1/16 = 0.0625");
            expect (std::abs (MeterComponent::quantizeStepProgress (0.3f, 16) - 0.3125f) < 1.0e-6f,
                    "progress 0.3 should quantise to step 5/16 = 0.3125");
        }

        void testMeterBypassShowsAllSegmentsOff()
        {
            beginTest ("MeterComponent bypass renders all segments off (#000) with the 1px #FFF outline preserved");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            juce::AudioBuffer<float> buffer (2, 64);
            buffer.clear();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                    buffer.setSample (ch, i, 0.5f); // rms ~= -6 dB

            juce::Component dummy;
            AnimationController controller (&dummy);

            MeterComponent activeMeter (controller);
            activeMeter.setSize (40, 160);
            activeMeter.updateFromBuffer (buffer);

            const auto activeSnap = activeMeter.createComponentSnapshot (activeMeter.getLocalBounds());
            // Segment 8 (y in [70, 79]) is ON for -6 dB (14 lit segments).
            expect (activeSnap.getPixelAt (20, 75) == white,
                    "Active meter should light segments for a -6 dB signal");

            MeterComponent bypassedMeter (controller);
            bypassedMeter.setBypassed (true);
            bypassedMeter.setSize (40, 160);
            bypassedMeter.updateFromBuffer (buffer);

            const auto bypassedSnap = bypassedMeter.createComponentSnapshot (bypassedMeter.getLocalBounds());
            expect (bypassedSnap.getPixelAt (20, 75) == black,
                    "Bypassed meter should render all segments OFF (#000 inside the bar)");
            expect (bypassedSnap.getPixelAt (20, 5) == black,
                    "Bypassed meter top segment should also be OFF (#000)");
            // Outline preserved on both edges.
            expect (bypassedSnap.getPixelAt (10, 80) == white,
                    "Bypassed meter should preserve the 1px #FFF left outline");
            expect (bypassedSnap.getPixelAt (29, 80) == white,
                    "Bypassed meter should preserve the 1px #FFF right outline");
            expect (bypassedSnap.getPixelAt (20, 0) == white,
                    "Bypassed meter should preserve the 1px #FFF top outline");
            expect (bypassedSnap.getPixelAt (20, 159) == white,
                    "Bypassed meter should preserve the 1px #FFF bottom outline");
        }

        void testEditorBackgroundUnaffectedByBypass()
        {
            beginTest ("Plugin editor black substrate is unaffected by bypass");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            editor.getAnimationController().setBypassMix (0.0f);
            editor.repaint();
            const auto activeSnapshot = editor.createComponentSnapshot (editor.getLocalBounds());

            editor.getAnimationController().setBypassMix (1.0f);
            editor.repaint();
            const auto bypassedSnapshot = editor.createComponentSnapshot (editor.getLocalBounds());

            // Sample points inside the 10 px gap strips between child widgets
            // (Slice I removed the rack-ear strips; the gaps between bypass /
            // oscilloscope / side bars / center panels are the only pure
            // substrate real-estate left). None of the substrate layers (dither
            // + scanlines + dead pixels) depend on bypass mix, so the substrate
            // must be byte-identical between the active and bypassed snapshots.
            const juce::Point<int> samplePoints[] = {
                { 135, 30 },   // bypass <-> oscilloscope gap in the top bar
                { 200, 95 },   // top bar <-> center area gap
                { 60, 200 },   // left side bar <-> center area gap
                { 740, 200 },  // center area <-> right side bar gap
                { 400, 475 }   // center area <-> editor bottom gap
            };

            for (const auto& p : samplePoints)
            {
                expect (activeSnapshot.getPixelAt (p.x, p.y) == bypassedSnapshot.getPixelAt (p.x, p.y),
                        "Pure-black DEADLOCK substrate must not dim with bypass mix");
            }
        }

        void testGlitchOverlayDitherIsBinary()
        {
            beginTest ("GlitchOverlay dither noise renders only pure #000 or #FFF pixels");

            GlitchOverlay overlay;
            juce::Image image (juce::Image::ARGB, GlitchOverlay::tileSizeForTests(), GlitchOverlay::tileSizeForTests(), true);
            juce::Graphics g (image);
            overlay.drawDitherNoise (g, juce::Rectangle<int> (0, 0, GlitchOverlay::tileSizeForTests(), GlitchOverlay::tileSizeForTests()));

            bool onlyBinary = true;
            bool hasWhite = false;

            for (int y = 0; y < GlitchOverlay::tileSizeForTests() && onlyBinary; ++y)
            {
                for (int x = 0; x < GlitchOverlay::tileSizeForTests(); ++x)
                {
                    const auto c = image.getPixelAt (x, y);

                    if (c == juce::Colour (0xFFFFFFFF))
                        hasWhite = true;
                    else if (c != juce::Colour (0xFF000000))
                        onlyBinary = false;
                }
            }

            expect (onlyBinary, "Dither tile must contain only pure #000 or #FFF pixels (no grey)");
            expect (hasWhite, "Dither tile should contain duty-cycle #FFF pixels");
        }

        void testGlitchOverlayTickAdvancesDitherFrame()
        {
            beginTest ("GlitchOverlay::tick advances the dither frame index");

            GlitchOverlay overlay;

            expect (overlay.getGrainFrameIndex() == 0, "Dither frame index should start at 0");

            overlay.tick();
            expect (overlay.getGrainFrameIndex() == 1, "Dither frame index should advance to 1 after one tick");
        }

        void testGlitchOverlayRerollsDeadPixelsApprox80ms()
        {
            beginTest ("GlitchOverlay::tick re-rolls dead pixels every ~80 ms (5 ticks at 60 Hz)");

            GlitchOverlay overlay;
            const int initial = overlay.getDeadPixelRerollCount();

            for (int i = 0; i < 5; ++i)
                overlay.tick();

            expect (overlay.getDeadPixelRerollCount() == initial + 1,
                    "Dead pixels should re-roll once after 5 ticks (~83 ms)");

            for (int i = 0; i < 4; ++i)
                overlay.tick();

            expect (overlay.getDeadPixelRerollCount() == initial + 1,
                    "Dead pixels should not re-roll again before the next 5-tick window");

            overlay.tick();

            expect (overlay.getDeadPixelRerollCount() == initial + 2,
                    "Dead pixels should re-roll a second time after 10 ticks");
        }

        void testBypassTransitionPropagatesToComponents()
        {
            beginTest ("Bypass transition propagates to oscilloscope and meters");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (! editor.getBypassComponent().getToggleButton().getToggleState());
            expect (! editor.getOscilloscope().isBypassed());

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (1.0f);
            editor.timerCallback();

            expect (editor.getBypassComponent().getToggleButton().getToggleState(),
                    "Bypass button should follow PLUGIN_BYPASS");
            expect (editor.getOscilloscope().isBypassed(),
                    "Oscilloscope should receive bypass state");
        }

        void testGlitchOverlayDriftBandDriftsLeftToRight()
        {
            beginTest ("GlitchOverlay::drawHorizontalBand drifts L->R in 16 hard steps");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;
            expect (overlay.getDriftBandStep() == 0, "Drift band starts at step 0");

            const auto drawBand = [&overlay] (juce::Image& img) {
                img.clear (img.getBounds(), juce::Colours::transparentBlack);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawHorizontalBand (g, img.getBounds());
            };

            juce::Image img (juce::Image::ARGB, 64, 32, true);
            drawBand (img);

            // Band is 2 px tall (y=0..1) and starts flush with the left edge.
            expect (img.getPixelAt (0, 0) == white, "Drift band left edge should be #FFF at step 0");
            expect (img.getPixelAt (0, 1) == white, "Drift band should be 2 px tall at y=1");
            expect (img.getPixelAt (0, 2) == black, "Drift band should stop at y=2");

            // kDriftBandStepTicks (8 ticks at 60 Hz) advance the band one step.
            for (int i = 0; i < GlitchOverlay::driftBandStepTicksForTests(); ++i)
                overlay.tick();

            expect (overlay.getDriftBandStep() == 1, "Drift band step should advance to 1 after kDriftBandStepTicks ticks");

            drawBand (img);
            expect (img.getPixelAt (0, 0) == black, "Drift band should leave the left edge after one step");
            const int stride = juce::jmax (1, 64 / GlitchOverlay::driftBandStepForTests());
            expect (img.getPixelAt (stride, 0) == white, "Drift band should land on step 1 at the next stride cell");
        }

        void testGlitchOverlayFlickerBlockTogglesEveryThirtyTicks()
        {
            beginTest ("GlitchOverlay::drawFlickerBlock toggles on/off every 30 ticks (500 ms at 60 Hz)");

            const auto white = juce::Colour (0xFFFFFFFF);

            GlitchOverlay overlay;

            auto blockVisibleAtCentre = [&overlay, white] {
                juce::Image img (juce::Image::ARGB, 32, 32, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawFlickerBlock (g, img.getBounds());
                return img.getPixelAt (img.getWidth() / 2, img.getHeight() / 2) == white;
            };

            expect (overlay.isFlickerBlockVisible(), "Flicker block should be on at tick 0");
            expect (blockVisibleAtCentre(), "Flicker block should render #FFF at tick 0");

            for (int i = 0; i < GlitchOverlay::flickerPeriodTicksForTests(); ++i)
                overlay.tick();

            expect (! overlay.isFlickerBlockVisible(), "Flicker block should turn off after kFlickerPeriodTicks ticks");
            expect (! blockVisibleAtCentre(), "Flicker block should draw nothing when hidden");

            for (int i = 0; i < GlitchOverlay::flickerPeriodTicksForTests(); ++i)
                overlay.tick();

            expect (overlay.isFlickerBlockVisible(), "Flicker block should turn back on after kFlickerPeriodTicks more ticks");
            expect (blockVisibleAtCentre(), "Flicker block should re-render #FFF after the second 30-tick window");
        }

        void testGlitchOverlayTripletDrawsThreeOffsets()
        {
            beginTest ("GlitchOverlay::drawTriplet strokes at horizontal offsets -1, 0, +1");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;

            juce::Image img (juce::Image::ARGB, 32, 8, true);
            juce::Graphics g (img);
            g.fillAll (juce::Colours::black);

            const int baseX = 16;
            const int y = 4;
            overlay.drawTriplet (g, [baseX, y, &g] (juce::Point<float> off)
            {
                g.setColour (juce::Colours::white);
                g.fillRect (baseX + static_cast<int> (off.x), y, 1, 1);
            });

            expect (img.getPixelAt (baseX - 1, y) == white, "Triplet left stroke should render at x-1");
            expect (img.getPixelAt (baseX, y) == white, "Triplet centre stroke should render at x");
            expect (img.getPixelAt (baseX + 1, y) == white, "Triplet right stroke should render at x+1");
            expect (img.getPixelAt (baseX - 2, y) == black, "No triplet stroke at x-2");
            expect (img.getPixelAt (baseX + 2, y) == black, "No triplet stroke at x+2");
        }

        void testGlitchOverlaySweepAdvancesLeftToRight()
        {
            beginTest ("GlitchOverlay::drawSweep wipes a 4px #FFF band L->R in 16 hard steps");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;
            expect (overlay.getSweepStep() == 0, "Sweep should start at step 0");
            expect (std::abs (overlay.getSweepPosition() - 0.0f) < 1.0e-6f, "Sweep position should start at 0");

            auto drawSweepAt = [&overlay] (float position)
            {
                juce::Image img (juce::Image::ARGB, 64, 16, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawSweep (g, img.getBounds(), position);
                return img;
            };

            const auto img0 = drawSweepAt (0.0f);
            expect (img0.getPixelAt (0, 8) == white, "Sweep at position 0 should start at the left edge");
            expect (img0.getPixelAt (3, 8) == white, "Sweep band should be kSweepWidth (4px) wide");
            expect (img0.getPixelAt (4, 8) == black, "Sweep band should stop at x = kSweepWidth");

            const auto imgMid = drawSweepAt (0.5f);
            expect (imgMid.getPixelAt (30, 8) == white, "Sweep at position 0.5 should land at x = round(0.5 * (64-4)) = 30");
            expect (imgMid.getPixelAt (33, 8) == white, "Sweep band should span 4px from x=30");
            expect (imgMid.getPixelAt (29, 8) == black, "Sweep band should not bleed left of x=30");
            expect (imgMid.getPixelAt (34, 8) == black, "Sweep band should not bleed right of x=33");

            const auto imgEnd = drawSweepAt (1.0f);
            expect (imgEnd.getPixelAt (60, 8) == white, "Sweep at position 1 should reach the right edge");
            expect (imgEnd.getPixelAt (63, 8) == white, "Sweep band should fit within bounds at the right edge");

            for (int i = 0; i < GlitchOverlay::sweepStepTicksForTests(); ++i)
                overlay.tick();

            expect (overlay.getSweepStep() == 1, "Sweep step should advance to 1 after kSweepStepTicks ticks");
            expect (std::abs (overlay.getSweepPosition() - (1.0f / static_cast<float> (GlitchOverlay::sweepStepsForTests()))) < 1.0e-6f,
                    "Sweep position should be 1/16 after one step");
        }

        void testPanelComponentRendersBrutalistShape()
        {
            beginTest ("PanelComponent renders brutalist shape: #FFF outline + #0D0D0E fill + rule + corner ticks");

            SynthortionLookAndFeel lookAndFeel;
            PanelComponent panel ("DISTORTION", juce::Colour (0xFF0D0D0E));
            panel.setSize (100, 100);
            panel.setLookAndFeel (&lookAndFeel);

            const auto snapshot = panel.createComponentSnapshot (panel.getLocalBounds());

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto charcoal = juce::Colour (0xFF0D0D0E);

            // Flat #0D0D0E fill on the panel interior (Surface per issue #28).
            expect (snapshot.getPixelAt (50, 50) == charcoal, "Brutalist panel interior should be flat #0D0D0E");
            expect (snapshot.getPixelAt (50, 80) == charcoal, "Brutalist panel interior below the rule should be flat #0D0D0E");

            // 1 px #FFF hard-square outline on every edge.
            expect (snapshot.getPixelAt (50, 0) == white, "Top edge outline should be #FFF");
            expect (snapshot.getPixelAt (50, 99) == white, "Bottom edge outline should be #FFF");
            expect (snapshot.getPixelAt (0, 50) == white, "Left edge outline should be #FFF");
            expect (snapshot.getPixelAt (99, 50) == white, "Right edge outline should be #FFF");

            // 2 px #FFF corner ticks stacking over the outline at every corner.
            expect (snapshot.getPixelAt (0, 0) == white && snapshot.getPixelAt (1, 1) == white,
                    "Top-left 2x2 corner tick should be #FFF");
            expect (snapshot.getPixelAt (99, 0) == white && snapshot.getPixelAt (98, 1) == white,
                    "Top-right 2x2 corner tick should be #FFF");
            expect (snapshot.getPixelAt (0, 99) == white && snapshot.getPixelAt (1, 98) == white,
                    "Bottom-left 2x2 corner tick should be #FFF");
            expect (snapshot.getPixelAt (99, 99) == white && snapshot.getPixelAt (98, 98) == white,
                    "Bottom-right 2x2 corner tick should be #FFF");

            // 1 px #FFF horizontal rule directly beneath the 22 pt title row.
            expect (snapshot.getPixelAt (50, 22) == white, "Rule beneath the title row should be #FFF");
        }

        void testPanelComponentTitleFontIsBebasNeueAllCaps()
        {
            beginTest ("PanelComponent section header is BebasNeue all-caps white with -0.5 kerning per issue #28");

            SynthortionLookAndFeel lookAndFeel;
            PanelComponent panel ("DISTORTION", lookAndFeel.findColour (SynthortionLookAndFeel::panelFillColourId));
            panel.setLookAndFeel (&lookAndFeel);

            const auto titleFont = panel.getTitleFont();
            expect (juce::roundToInt (titleFont.getHeight()) == 22,
                    "Panel title font should be 22pt BebasNeue per issue #25");
            expect (titleFont.getTypefaceName().containsIgnoreCase ("Bebas"),
                    "Panel title font should request the BebasNeue typeface");
            expect (lookAndFeel.getTypefaceForFont (titleFont)
                        == lookAndFeel.getTypefaceForFont (juce::FontOptions().withName ("BebasNeue")),
                    "Panel title font should resolve to the BebasNeue typeface");
            expect (std::abs (titleFont.getExtraKerningFactor() - SynthortionLookAndFeel::kTightKerning) < 1.0e-6f,
                    "Panel title font should apply -0.5 tight kerning per issue #25");

            expect (panel.getTitle().toUpperCase() == panel.getTitle(),
                    "Panel title text should be all-caps (DISTORTION) per issue #28");

            const auto textColour = lookAndFeel.findColour (SynthortionLookAndFeel::textColourId);
            expect (textColour == juce::Colour (0xFFFFFFFF),
                    "Panel section header text colour (Ink) should be pure #FFF per issue #28");
        }

        void testEditorDrawsDashedSectionSeparators()
        {
            beginTest ("Editor draws dashed low-alpha grid lines between panel sections per issue #28");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            editor.repaint();
            const auto snapshot = editor.createComponentSnapshot (editor.getLocalBounds());

            const juce::Colour dimmed (0xFF666666);

            auto scanHLine = [&] (int y, int x1, int x2)
            {
                int dimmedPixels = 0;
                int nonDimmedPixels = 0;
                for (int x = x1; x < x2; ++x)
                {
                    const auto c = snapshot.getPixelAt (x, y);
                    if (c == dimmed)
                        ++dimmedPixels;
                    else
                        ++nonDimmedPixels;
                }
                return dimmedPixels > 0 && nonDimmedPixels > 0;
            };

            auto scanVLine = [&] (int x, int y1, int y2)
            {
                int dimmedPixels = 0;
                int nonDimmedPixels = 0;
                for (int y = y1; y < y2; ++y)
                {
                    const auto c = snapshot.getPixelAt (x, y);
                    if (c == dimmed)
                        ++dimmedPixels;
                    else
                        ++nonDimmedPixels;
                }
                return dimmedPixels > 0 && nonDimmedPixels > 0;
            };

            expect (scanHLine (95, 200, 600),
                    "Dashed dimmed separator should span the top-bar/center gap at y=95");
            expect (scanHLine (475, 200, 600),
                    "Dashed dimmed separator should span the center/bottom gap at y=475");
            expect (scanVLine (60, 200, 400),
                    "Dashed dimmed separator should span the left-bar/center gap at x=60");
            expect (scanVLine (740, 200, 400),
                    "Dashed dimmed separator should span the center/right-bar gap at x=740");
            expect (scanHLine (308, 200, 600),
                    "Dashed dimmed separator should span the distortion/bottom-row gap at y=308");
            expect (scanVLine (308, 350, 450),
                    "Dashed dimmed separator should span the chorus/delay gap at x=308");
            expect (scanVLine (620, 350, 450),
                    "Dashed dimmed separator should span the delay/coming-soon gap at x=620");
        }

        void testKnobValueLabelsUseMontserrat()
        {
            beginTest ("Numeric knob value labels use Montserrat font while titles stay BebasNeue per issue #28");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto assertValueIsMontserrat = [&] (juce::Label& valueLabel, const juce::String& labelName)
            {
                const auto f = valueLabel.getFont();
                expect (f.getTypefaceName().containsIgnoreCase ("Montserrat"),
                        "Knob value label " + labelName + " should request the Montserrat typeface per issue #28");
            };

            auto assertTitleIsBebas = [&] (juce::Label& titleLabel, const juce::String& labelName)
            {
                const auto f = titleLabel.getFont();
                expect (f.getTypefaceName().containsIgnoreCase ("Bebas"),
                        "Knob title label " + labelName + " should stay BebasNeue per issue #28");
            };

            assertValueIsMontserrat (editor.driveLabel, "COLOR");
            assertValueIsMontserrat (editor.bitCrushLabel, "BITCRUSH");
            assertValueIsMontserrat (editor.chorusMixLabel, "CHORUS_MIX");
            assertValueIsMontserrat (editor.delayTimeLabel, "DELAY_TIME");
            assertValueIsMontserrat (editor.delayFeedbackLabel, "DELAY_FEEDBACK");
            assertValueIsMontserrat (editor.delayMixLabel, "DELAY_MIX");
            assertValueIsMontserrat (editor.inputGainLabel, "INPUT_GAIN");
            assertValueIsMontserrat (editor.outputGainLabel, "OUTPUT_GAIN");

            assertTitleIsBebas (editor.driveTitleLabel, "COLOR");
            assertTitleIsBebas (editor.delayTimeTitleLabel, "TIME");
            assertTitleIsBebas (editor.inputGainTitleLabel, "INPUT");
        }

        void testKnobValueLabelsDoNotOverlapKnobs()
        {
            beginTest ("Montserrat knob value labels have spacing and do not overlap knobs per issue #28");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto assertNoOverlap = [&] (AnimatedKnob& knob, juce::Label& titleLabel, juce::Label& valueLabel,
                                         const juce::String& labelName)
            {
                const auto knobBounds = knob.getBounds();
                const auto titleBounds = titleLabel.getBounds();
                const auto valueBounds = valueLabel.getBounds();

                expect (! titleBounds.intersects (knobBounds),
                        "Title label " + labelName + " must not overlap its knob");
                expect (! valueBounds.intersects (knobBounds),
                        "Value label " + labelName + " must not overlap its knob");
                expect (valueBounds.getY() >= knobBounds.getBottom() - 1,
                        "Montserrat value label " + labelName + " should sit below its knob with spacing per issue #28");
            };

            assertNoOverlap (editor.driveKnob, editor.driveTitleLabel, editor.driveLabel, "COLOR");
            assertNoOverlap (editor.bitCrushKnob, editor.bitCrushTitleLabel, editor.bitCrushLabel, "BITCRUSH");
            assertNoOverlap (editor.chorusMixKnob, editor.chorusMixTitleLabel, editor.chorusMixLabel, "CHORUS_MIX");
            assertNoOverlap (editor.delayTimeKnob, editor.delayTimeTitleLabel, editor.delayTimeLabel, "TIME");
            assertNoOverlap (editor.delayFeedbackKnob, editor.delayFeedbackTitleLabel, editor.delayFeedbackLabel, "FB");
            assertNoOverlap (editor.delayMixKnob, editor.delayMixTitleLabel, editor.delayMixLabel, "MIX");
            assertNoOverlap (editor.inputGainKnob, editor.inputGainTitleLabel, editor.inputGainLabel, "INPUT");
            assertNoOverlap (editor.outputGainKnob, editor.outputGainTitleLabel, editor.outputGainLabel, "OUTPUT");
        }

        void countKnobsByStyleRecursive (juce::Component& parent, int& canonical, int& outline)
        {
            for (auto* child : parent.getChildren())
            {
                if (auto* knob = dynamic_cast<AnimatedKnob*> (child))
                {
                    if (knob->getKnobStyle() == AnimatedKnob::KnobStyle::Canonical)
                        ++canonical;
                    else
                        ++outline;
                }

                countKnobsByStyleRecursive (*child, canonical, outline);
            }
        }

        juce::MouseEvent makeMouseDownEvent (juce::Component& target)
        {
            auto source = juce::Desktop::getInstance().getMainMouseSource();
            const juce::Point<float> pos (static_cast<float> (target.getWidth()) * 0.5f,
                                          static_cast<float> (target.getHeight()) * 0.5f);
            const auto now = juce::Time::getCurrentTime();

            return juce::MouseEvent (source,
                                    pos,
                                    juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier),
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    &target,
                                    &target,
                                    now,
                                    pos,
                                    now,
                                    1,
                                    false);
        }

        void testAnimatedKnobDefaultStyleIsCanonical()
        {
            beginTest ("AnimatedKnob defaults to the Canonical brutalist style");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            expect (knob.getKnobStyle() == AnimatedKnob::KnobStyle::Canonical,
                    "Newly constructed AnimatedKnobs should default to the Canonical large style");
        }

        void testAnimatedKnobStepCountMatchesStyle()
        {
            beginTest ("AnimatedKnob exposes Step counts of 16 (Canonical) and 8 (Outline)");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            expect (knob.getStepCount() == 16, "Canonical knob Step count should be 16");

            knob.setKnobStyle (AnimatedKnob::KnobStyle::Outline);
            expect (knob.getStepCount() == 8, "Outline knob Step count should be 8");
        }

        void testAnimatedKnobStepEasingQuantizesHard()
        {
            beginTest ("AnimatedKnob::quantizeStepProgress jumps in hard steps with no interpolation");

            expect (std::abs (AnimatedKnob::quantizeStepProgress (0.0f, 16) - 0.0f) < 1.0e-6f,
                    "progress 0 should quantise to the 0/N step");
            expect (std::abs (AnimatedKnob::quantizeStepProgress (0.499f, 16) - 0.5f) < 1.0e-6f,
                    "progress ~0.5 with N=16 should snap to the 8/16 step (0.5)");
            expect (std::abs (AnimatedKnob::quantizeStepProgress (1.0f, 16) - 1.0f) < 1.0e-6f,
                    "progress 1 should quantise to the N/N step (1)");
            expect (std::abs (AnimatedKnob::quantizeStepProgress (0.34f, 8) - 0.375f) < 1.0e-6f,
                    "progress ~0.34 with N=8 should snap to the 3/8 step (0.375)");
        }

        void testCanonicalKnobInvertsFaceOnMouseDown()
        {
            beginTest ("Canonical knob inverts its disc from #FFF to #000 on mouse-down");

            juce::Component dummy;
            AnimationController controller (&dummy);
            SynthortionLookAndFeel lookAndFeel;

            AnimatedKnob knob (controller);
            knob.setRange (0.0, 1.0);
            knob.setValue (0.5, juce::dontSendNotification);
            knob.snapToCurrentValue();
            knob.setSize (60, 60);
            knob.setLookAndFeel (&lookAndFeel);

            const auto idleSnapshot = knob.createComponentSnapshot (knob.getLocalBounds());
            expect (idleSnapshot.getPixelAt (15, 30).getBrightness() > 0.9f,
                    "Canonical knob face should be pure #FFF at rest");

            knob.mouseDown (makeMouseDownEvent (knob));

            const auto pressedSnapshot = knob.createComponentSnapshot (knob.getLocalBounds());
            expect (pressedSnapshot.getPixelAt (15, 30).getBrightness() < 0.1f,
                    "Canonical knob face should invert to pure #000 on mouse-down");
        }

        void testTwinShadowRendersOnDrag()
        {
            beginTest ("AnimatedKnob renders a hard twin shadow ring on mouse-down");

            juce::Component dummy;
            AnimationController controller (&dummy);
            SynthortionLookAndFeel lookAndFeel;

            AnimatedKnob knob (controller);
            knob.setRange (0.0, 1.0);
            knob.setValue (0.5, juce::dontSendNotification);
            knob.snapToCurrentValue();
            knob.setSize (60, 60);
            knob.setLookAndFeel (&lookAndFeel);

            const auto idleSnapshot = knob.createComponentSnapshot (knob.getLocalBounds());
            knob.mouseDown (makeMouseDownEvent (knob));
            const auto pressedSnapshot = knob.createComponentSnapshot (knob.getLocalBounds());

            const auto idlePixel = idleSnapshot.getPixelAt (50, 51);
            const auto pressedPixel = pressedSnapshot.getPixelAt (50, 51);

            expect (idlePixel.getAlpha() == 0 || idlePixel.getBrightness() < 0.1f,
                    "Pixel outside the rest knob disc should be the transparent substrate");
            expect (pressedPixel.getAlpha() > 0 && pressedPixel.getBrightness() > idlePixel.getBrightness(),
                    "On mouse-down the twin-shadow #FFF ring should render a light pixel lower-right of the disc");
        }

        void testEditorKnobsUseFourCanonicalAndFourOutline()
        {
            beginTest ("Editor knobs split 4 Canonical (large) + 4 Outline (small)");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int canonical = 0;
            int outline = 0;
            countKnobsByStyleRecursive (editor, canonical, outline);

            expect (canonical == 4, "Editor should expose exactly four Canonical (large) knobs");
            expect (outline == 4, "Editor should expose exactly four Outline (small) knobs");
        }

        void testBypassSwitchRendersActiveWhiteBlock()
        {
            beginTest ("BypassSwitch active state renders a solid #FFF Block with #000 BYPASS text, no outline");

            SynthortionLookAndFeel lookAndFeel;

            BypassSwitch bypass (nullptr);
            bypass.setSize (120, 80);
            bypass.setLookAndFeel (&lookAndFeel);
            bypass.setToggleState (false, juce::sendNotificationSync);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            const auto snap = bypass.createComponentSnapshot (bypass.getLocalBounds());
            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            expect (snap.getPixelAt (15, 70) == white,
                    "Active state: solid #FFF block fill at the lower-left interior, away from the BYPASS label");
            expect (snap.getPixelAt (10, 10) == white,
                    "Active state: no #000 outline ring at the upper-left interior");

            bool hasBlackPixel = false;
            for (int y = 28; y < 52 && ! hasBlackPixel; ++y)
                for (int x = 30; x < 90 && ! hasBlackPixel; ++x)
                    if (snap.getPixelAt (x, y).getBrightness() < 0.05f)
                        hasBlackPixel = true;

            expect (hasBlackPixel, "Active state: #000 BYPASS label BebasNeue 16pt must render");
        }

        void testBypassSwitchRendersBypassedBlackBlockWithOutline()
        {
            beginTest ("BypassSwitch bypassed state renders a solid #000 block with a 1 px #FFF outline");

            SynthortionLookAndFeel lookAndFeel;

            BypassSwitch bypass (nullptr);
            bypass.setSize (120, 80);
            bypass.setLookAndFeel (&lookAndFeel);
            bypass.setToggleState (true, juce::sendNotificationSync);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            const auto snap = bypass.createComponentSnapshot (bypass.getLocalBounds());
            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            expect (snap.getPixelAt (10, 70) == black,
                    "Bypassed state: solid #000 block fill at the lower-left interior, away from the BYPASS label");

            expect (snap.getPixelAt (0, 40) == white,
                    "Bypassed state: 1 px #FFF outline ring on the left edge of the block");
            expect (snap.getPixelAt (60, 79) == white,
                    "Bypassed state: 1 px #FFF outline ring on the bottom edge of the block");

            bool hasWhitePixel = false;
            for (int y = 28; y < 52 && ! hasWhitePixel; ++y)
                for (int x = 30; x < 90 && ! hasWhitePixel; ++x)
                    if (snap.getPixelAt (x, y).getBrightness() > 0.95f)
                        hasWhitePixel = true;

            expect (hasWhitePixel, "Bypassed state: #FFF BYPASS label BebasNeue 16pt must render");
        }

        void testBypassSwitchFiresSliceGlitchOnToggle()
        {
            beginTest ("BypassSwitch fires GlitchOverlay::triggerBypassSlices on a state transition");

            GlitchOverlay overlay;
            BypassSwitch bypass (nullptr);
            bypass.setGlitchOverlay (&overlay);
            bypass.setSize (120, 80);

            expect (! overlay.isBypassSliceActive(),
                    "Slice glitch should be inactive before any toggle");

            bypass.setToggleState (true, juce::sendNotificationSync);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

            expect (overlay.isBypassSliceActive(),
                    "Toggling BypassSwitch to bypassed should fire the slice glitch burst");
        }

        void testGlitchOverlayBypassSliceBurstWindow()
        {
            beginTest ("GlitchOverlay::triggerBypassSlices launches a ~150 ms burst then snaps back");

            GlitchOverlay overlay;

            expect (! overlay.isBypassSliceActive(),
                    "Slice glitch should be inactive at rest");

            overlay.triggerBypassSlices();
            expect (overlay.isBypassSliceActive(),
                    "Slice glitch should activate immediately after triggerBypassSlices");

            for (int i = 0; i < GlitchOverlay::bypassSliceDurationTicksForTests(); ++i)
                overlay.tick();

            expect (! overlay.isBypassSliceActive(),
                    "Slice glitch should snap back after the ~150 ms burst window (kBypassSliceDurationTicks ticks)");
        }

        void testGlitchOverlayDrawBypassSlicesRendersBandsWhenActive()
        {
            beginTest ("GlitchOverlay::drawBypassSlices renders hard #FFF bands during the burst and no bands at rest");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;

            // At rest: no bands drawn.
            {
                juce::Image img (juce::Image::ARGB, 64, 64, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawBypassSlices (g, img.getBounds());
                expect (img.getPixelAt (20, 16) == black,
                        "Inactive slice bands should not paint over the substrate");
            }

            // First tick on a freshly-triggered burst: bands render at their base positions.
            overlay.triggerBypassSlices();

            juce::Image img (juce::Image::ARGB, 64, 64, true);
            juce::Graphics g (img);
            g.fillAll (juce::Colours::black);
            overlay.drawBypassSlices (g, img.getBounds());

            expect (img.getPixelAt (20, 16) == white,
                    "Slice band 1 (yFrac=0.25, thickness=2) should render at y=16");
            expect (img.getPixelAt (20, 17) == white,
                    "Slice band 1 should be exactly 2 px thick");
            expect (img.getPixelAt (20, 18) == black,
                    "Slice band 1 should end after 2 px");
            expect (img.getPixelAt (20, 32) == white,
                    "Slice band 2 (yFrac=0.50, thickness=2) should render at y=32");
            expect (img.getPixelAt (20, 48) == white,
                    "Slice band 3 (yFrac=0.75, thickness=2) should render at y=48");
        }

        void testGlitchOverlayBootBurstFiresOnceAndRunsForFourHundredMs()
        {
            beginTest ("GlitchOverlay::triggerBootBurst fires once and runs for ~400 ms (24 ticks at 60 Hz)");

            GlitchOverlay overlay;

            expect (! overlay.isBootBurstActive(),
                    "Boot Burst should be inactive at rest");
            expect (! overlay.isBootBurstFired(),
                    "Boot Burst should not be marked fired before the first trigger");

            overlay.triggerBootBurst();
            expect (overlay.isBootBurstActive(),
                    "Boot Burst should be active immediately after triggerBootBurst");
            expect (overlay.isBootBurstFired(),
                    "Boot Burst should be marked fired after the first trigger");
            expect (overlay.getBootBurstElapsedTicks() == 0,
                    "Boot Burst elapsed ticks should start at 0");
            expect (std::abs (overlay.getBootBurstProgress() - 0.0f) < 1.0e-6f,
                    "Boot Burst progress should start at 0");

            const int durationTicks = GlitchOverlay::bootBurstDurationTicksForTests();
            expect (durationTicks == 24,
                    "Boot Burst duration must be 24 ticks (~400 ms at 60 Hz) per DEADLOCK Slice I issue #26");
            const int flashTicks = GlitchOverlay::bootBurstFlashTicksForTests();
            expect (flashTicks == 6,
                    "Boot Burst flash window must be 6 ticks (~100 ms at 60 Hz)");
            expect (GlitchOverlay::bootBurstBandCountForTests() == 6,
                    "Boot Burst must compose 6 random horizontal Slice displacements");
            expect (GlitchOverlay::bootBurstDeadPixelCountForTests() == 60,
                    "Boot Burst must paint a dense Dead pixel field of 60 pixels");

            for (int i = 0; i < durationTicks; ++i)
            {
                overlay.tick();
                if (i + 1 < durationTicks)
                    expect (overlay.isBootBurstActive(),
                            "Boot Burst should stay active until the 24-tick window elapses");
            }

            expect (! overlay.isBootBurstActive(),
                    "Boot Burst should snap back to inactive after the 24-tick window");

            // Re-triggering after the first fire is a no-op: the per-process
            // bootBurstFired guard prevents the Burst from ever firing again.
            overlay.triggerBootBurst();
            expect (! overlay.isBootBurstActive(),
                    "triggerBootBurst must be a no-op once bootBurstFired is set");
        }

        void testGlitchOverlayDrawBootBurstRendersFlashSlicesAndDeadPixels()
        {
            beginTest ("GlitchOverlay::drawBootBurst renders a #FFF flash, 6 slice bands and a dense dead pixel field");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;

            // At rest: drawBootBurst paints nothing.
            {
                juce::Image img (juce::Image::ARGB, 64, 64, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawBootBurst (g, img.getBounds(), 0.0f);
                expect (img.getPixelAt (32, 32) == black,
                        "Inactive boot Burst should not paint over the substrate");
            }

            overlay.triggerBootBurst();

            // First ~100 ms (progress < 6/24 = 0.25): full-bounds #FFF flash.
            {
                juce::Image img (juce::Image::ARGB, 64, 64, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawBootBurst (g, img.getBounds(), 0.1f);
                expect (img.getPixelAt (0, 0) == white,
                        "Boot Burst flash should fill the whole bounds with #FFF for the first 100 ms");
                expect (img.getPixelAt (63, 63) == white,
                        "Boot Burst flash should reach every corner of the bounds");
            }

            // Late in the burst (progress = 1.0): flash has faded, but the 6
            // slice bands + dense dead pixel field still render hard #FFF.
            {
                juce::Image img (juce::Image::ARGB, 64, 64, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawBootBurst (g, img.getBounds(), 1.0f);

                // The flash has fully faded at progress = 1.0, so the substrate
                // shows through where no band/dead pixel paints.
                expect (img.getPixelAt (32, 32) == black || img.getPixelAt (32, 32) == white,
                        "Late boot Burst pixels should be hard #000 or #FFF (flash faded, bands/dead pixels remain)");

                int whitePixelCount = 0;
                for (int y = 0; y < 64; ++y)
                    for (int x = 0; x < 64; ++x)
                        if (img.getPixelAt (x, y) == white)
                            ++whitePixelCount;

                // 6 slice bands of 1-3 px thickness across a 64 px wide bounds
                // => at least 6 * 1 * 64 = 384 white pixels, plus 60 dead
                // pixels => at least 444 white pixels total.
                expect (whitePixelCount >= 400,
                        "Late boot Burst should still paint the 6 slice bands + dense dead pixel field in hard #FFF");
            }
        }

        void testGlitchOverlayFlickerBurstFiresAtRareIntervals()
        {
            beginTest ("GlitchOverlay rare Flicker burst fires at ~10-30s intervals (600-1800 ticks at 60 Hz) per issue #33");

            GlitchOverlay overlay;

            expect (! overlay.isFlickerBurstActive(),
                    "Flicker burst should be inactive at rest");
            expect (! overlay.isBypassAmplified(),
                    "Flicker burst should start in normal (non-amplified) mode");

            const int minTicks = GlitchOverlay::flickerNormalMinIntervalTicksForTests();
            const int maxTicks = GlitchOverlay::flickerNormalMaxIntervalTicksForTests();
            expect (minTicks == 600, "Normal min interval must be 600 ticks (~10s) per issue #33");
            expect (maxTicks == 1800, "Normal max interval must be 1800 ticks (~30s) per issue #33");

            const int scheduled = overlay.getTicksToNextFlicker();
            expect (scheduled >= minTicks && scheduled <= maxTicks,
                    "Initial scheduled flicker interval must fall in the ~10-30s normal range");

            // Tick (scheduled - 1) times: the countdown has not yet reached
            // zero, so the burst must remain inactive.
            for (int i = 0; i < scheduled - 1; ++i)
                overlay.tick();

            expect (! overlay.isFlickerBurstActive(),
                    "Flicker burst should not fire before the scheduled interval elapses");
            expect (overlay.getFlickerBurstCount() == 0,
                    "Flicker burst count should still be zero before the first fire");

            // The scheduled-th tick decrements the countdown to <= 0 and
            // triggers the burst.
            overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should fire exactly when the scheduled interval elapses");
            expect (overlay.getFlickerBurstCount() == 1,
                    "Flicker burst count should increment to 1 on the first fire");
            expect (overlay.getFlickerBurstElapsedTicks() == 0,
                    "Flicker burst elapsed ticks should reset to 0 on fire");
        }

        void testGlitchOverlayFlickerBurstResolvesAfterNormalDuration()
        {
            beginTest ("GlitchOverlay Flicker burst resolves after ~300ms (18 ticks at 60 Hz) and reschedules per issue #33");

            GlitchOverlay overlay;

            const int scheduled = overlay.getTicksToNextFlicker();
            for (int i = 0; i < scheduled; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should be active immediately after firing");

            const int duration = GlitchOverlay::flickerNormalDurationTicksForTests();
            expect (duration == 18, "Normal flicker burst duration must be 18 ticks (~300ms) per issue #33");

            // (duration - 1) ticks keep the burst active; the duration-th
            // tick snaps it back to inactive and reschedules the next interval.
            for (int i = 0; i < duration - 1; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should stay active until the duration window elapses");

            overlay.tick();

            expect (! overlay.isFlickerBurstActive(),
                    "Flicker burst should snap back to inactive after the ~300ms duration window");

            const int nextScheduled = overlay.getTicksToNextFlicker();
            const int minTicks = GlitchOverlay::flickerNormalMinIntervalTicksForTests();
            const int maxTicks = GlitchOverlay::flickerNormalMaxIntervalTicksForTests();
            expect (nextScheduled >= minTicks && nextScheduled <= maxTicks,
                    "Flicker burst should reschedule a fresh ~10-30s interval after resolving");
        }

        void testGlitchOverlayBypassAmplifiesFlickerInterval()
        {
            beginTest ("GlitchOverlay::setBypassAmplified shortens the flicker interval to ~3-8s (180-480 ticks) per issue #33");

            GlitchOverlay overlay;

            const int normalMin = GlitchOverlay::flickerNormalMinIntervalTicksForTests();
            const int normalMax = GlitchOverlay::flickerNormalMaxIntervalTicksForTests();
            const int bypassMin = GlitchOverlay::flickerBypassMinIntervalTicksForTests();
            const int bypassMax = GlitchOverlay::flickerBypassMaxIntervalTicksForTests();
            expect (bypassMin == 180, "Bypass min interval must be 180 ticks (~3s) per issue #33");
            expect (bypassMax == 480, "Bypass max interval must be 480 ticks (~8s) per issue #33");
            expect (bypassMax < normalMin,
                    "Bypass interval range must sit entirely below the normal range (frequency increases)");

            expect (overlay.getTicksToNextFlicker() >= normalMin
                        && overlay.getTicksToNextFlicker() <= normalMax,
                    "Initial interval must be in the normal range before bypass is engaged");

            overlay.setBypassAmplified (true);
            expect (overlay.isBypassAmplified(),
                    "setBypassAmplified(true) should flip the amplified flag");
            const int bypassScheduled = overlay.getTicksToNextFlicker();
            expect (bypassScheduled >= bypassMin && bypassScheduled <= bypassMax,
                    "Engaging bypass should reschedule the flicker interval into the ~3-8s bypass range");

            // Re-asserting the same state is a no-op (no reschedule churn).
            const int beforeNoop = overlay.getTicksToNextFlicker();
            overlay.setBypassAmplified (true);
            expect (overlay.getTicksToNextFlicker() == beforeNoop,
                    "setBypassAmplified with an unchanged state should not reschedule");

            overlay.setBypassAmplified (false);
            expect (! overlay.isBypassAmplified(),
                    "setBypassAmplified(false) should clear the amplified flag");
            const int normalScheduled = overlay.getTicksToNextFlicker();
            expect (normalScheduled >= normalMin && normalScheduled <= normalMax,
                    "Disengaging bypass should reschedule the flicker interval back into the ~10-30s normal range");
        }

        void testGlitchOverlayFlickerBurstBypassLastsLongerThanNormal()
        {
            beginTest ("GlitchOverlay bypass-amplified flicker burst lasts ~600ms (36 ticks) vs ~300ms (18 ticks) normal per issue #33");

            GlitchOverlay overlay;

            expect (GlitchOverlay::flickerNormalDurationTicksForTests() == 18,
                    "Normal flicker duration must be 18 ticks");
            expect (GlitchOverlay::flickerBypassDurationTicksForTests() == 36,
                    "Bypass flicker duration must be 36 ticks (double the normal duration)");

            overlay.setBypassAmplified (true);

            const int scheduled = overlay.getTicksToNextFlicker();
            for (int i = 0; i < scheduled; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Bypass-amplified flicker burst should fire after the bypass interval elapses");

            // 18 ticks (the normal duration) must NOT resolve a bypass burst.
            for (int i = 0; i < GlitchOverlay::flickerNormalDurationTicksForTests(); ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Bypass-amplified burst should still be active after the normal 18-tick duration (it lasts 36 ticks)");

            // The remaining (36 - 18 - 1) = 17 ticks keep it active; the
            // 36th tick resolves it.
            for (int i = 0; i < GlitchOverlay::flickerBypassDurationTicksForTests()
                                - GlitchOverlay::flickerNormalDurationTicksForTests() - 1; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Bypass-amplified burst should stay active until the 36-tick window elapses");

            overlay.tick();

            expect (! overlay.isFlickerBurstActive(),
                    "Bypass-amplified burst should snap back to inactive after the 36-tick duration window");
        }

        void testGlitchOverlayDrawFlickerBurstRendersBandsAndDeadPixels()
        {
            beginTest ("GlitchOverlay::drawFlickerBurst renders hard #FFF slice bands + dead pixel field during the burst and nothing at rest per issue #33");

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            GlitchOverlay overlay;

            // At rest: drawFlickerBurst paints nothing.
            {
                juce::Image img (juce::Image::ARGB, 64, 64, true);
                juce::Graphics g (img);
                g.fillAll (juce::Colours::black);
                overlay.drawFlickerBurst (g, img.getBounds());
                expect (img.getPixelAt (32, 32) == black,
                        "Inactive flicker burst should not paint over the substrate");
            }

            // Fire the burst by ticking through the scheduled interval.
            const int scheduled = overlay.getTicksToNextFlicker();
            for (int i = 0; i < scheduled; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should be active after the interval elapses");

            juce::Image img (juce::Image::ARGB, 64, 64, true);
            juce::Graphics g (img);
            g.fillAll (juce::Colours::black);
            overlay.drawFlickerBurst (g, img.getBounds());

            // Normal burst: 3 bands of 1-3 px thickness across 64 px width
            // => at least 3 * 1 * 64 = 192 band pixels, plus 20 dead
            // pixels => at least 212 white pixels total.
            int whitePixelCount = 0;
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x)
                    if (img.getPixelAt (x, y) == white)
                        ++whitePixelCount;

            expect (whitePixelCount >= 200,
                    "Active flicker burst should paint the 3 slice bands + 20 dead pixel field in hard #FFF");

            bool onlyBinary = true;
            for (int y = 0; y < 64 && onlyBinary; ++y)
                for (int x = 0; x < 64; ++x)
                    if (img.getPixelAt (x, y) != white && img.getPixelAt (x, y) != black)
                        onlyBinary = false;

            expect (onlyBinary,
                    "Flicker burst pixels must be hard #000 or #FFF (strict DEADLOCK palette)");
        }

        void testGlitchOverlayBypassAmplifiesFlickerIntensity()
        {
            beginTest ("GlitchOverlay bypass-amplified flicker burst draws more bands + dead pixels (6 bands / 40 dead) per issue #33");

            const auto white = juce::Colour (0xFFFFFFFF);

            expect (GlitchOverlay::flickerNormalBandCountForTests() == 3,
                    "Normal flicker burst must compose 3 slice bands");
            expect (GlitchOverlay::flickerBypassBandCountForTests() == 6,
                    "Bypass-amplified flicker burst must compose 6 slice bands (double)");
            expect (GlitchOverlay::flickerNormalDeadPixelCountForTests() == 20,
                    "Normal flicker burst must scatter 20 dead pixels");
            expect (GlitchOverlay::flickerBypassDeadPixelCountForTests() == 40,
                    "Bypass-amplified flicker burst must scatter 40 dead pixels (double)");
            expect (GlitchOverlay::flickerBypassMaxShiftForTests() > GlitchOverlay::flickerNormalMaxShiftForTests(),
                    "Bypass-amplified max displacement must exceed the normal max displacement");

            GlitchOverlay overlay;
            overlay.setBypassAmplified (true);

            const int scheduled = overlay.getTicksToNextFlicker();
            for (int i = 0; i < scheduled; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Bypass-amplified flicker burst should be active after the interval elapses");

            juce::Image img (juce::Image::ARGB, 64, 64, true);
            juce::Graphics g (img);
            g.fillAll (juce::Colours::black);
            overlay.drawFlickerBurst (g, img.getBounds());

            // Bypass burst: 6 bands of 1-3 px thickness across 64 px width
            // => at least 6 * 1 * 64 = 384 band pixels, plus 40 dead
            // pixels => at least 424 white pixels total.
            int whitePixelCount = 0;
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x)
                    if (img.getPixelAt (x, y) == white)
                        ++whitePixelCount;

            expect (whitePixelCount >= 400,
                    "Bypass-amplified flicker burst should paint the 6 slice bands + 40 dead pixel field in hard #FFF");
        }

        void testGlitchOverlayFlickerBurstCoexistsWithBypassSlices()
        {
            beginTest ("GlitchOverlay Flicker burst coexists with the existing bypass Slice burst (preserved) per issue #33");

            const auto white = juce::Colour (0xFFFFFFFF);

            GlitchOverlay overlay;

            // Fire a flicker burst, then fire the bypass Slice burst on top
            // of it: both systems must be simultaneously active and render
            // without interfering with each other.
            const int scheduled = overlay.getTicksToNextFlicker();
            for (int i = 0; i < scheduled; ++i)
                overlay.tick();

            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should be active after its interval elapses");

            overlay.triggerBypassSlices();
            expect (overlay.isBypassSliceActive(),
                    "Bypass Slice burst should activate on triggerBypassSlices (preserved)");
            expect (overlay.isFlickerBurstActive(),
                    "Flicker burst should remain active while the bypass Slice burst is also active");

            juce::Image img (juce::Image::ARGB, 64, 64, true);
            juce::Graphics g (img);
            g.fillAll (juce::Colours::black);
            overlay.drawBypassSlices (g, img.getBounds());
            overlay.drawFlickerBurst (g, img.getBounds());

            int whitePixelCount = 0;
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x)
                    if (img.getPixelAt (x, y) == white)
                        ++whitePixelCount;

            expect (whitePixelCount >= 200,
                    "Both the bypass Slice bands and the flicker burst bands + dead pixels should render together");

            // The bypass Slice burst must still resolve on its own ~150ms
            // (9-tick) window, unaffected by the flicker system.
            for (int i = 0; i < GlitchOverlay::bypassSliceDurationTicksForTests(); ++i)
                overlay.tick();

            expect (! overlay.isBypassSliceActive(),
                    "Bypass Slice burst should still snap back after its 9-tick window with the flicker system present");
        }

        void testEditorBypassToggleAmplifiesGlitchOverlay()
        {
            beginTest ("Plugin editor bypass toggle amplifies the GlitchOverlay flicker system per issue #33");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (! editor.glitchOverlay.isBypassAmplified(),
                    "Editor GlitchOverlay should start in normal (non-amplified) flicker mode");

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (1.0f);
            editor.timerCallback();

            expect (editor.glitchOverlay.isBypassAmplified(),
                    "Engaging bypass should amplify the editor GlitchOverlay flicker system");
            expect (editor.glitchOverlay.getTicksToNextFlicker()
                        >= GlitchOverlay::flickerBypassMinIntervalTicksForTests()
                    && editor.glitchOverlay.getTicksToNextFlicker()
                        <= GlitchOverlay::flickerBypassMaxIntervalTicksForTests(),
                    "Engaging bypass should reschedule the editor flicker interval into the ~3-8s bypass range");

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (0.0f);
            editor.timerCallback();

            expect (! editor.glitchOverlay.isBypassAmplified(),
                    "Disengaging bypass should return the editor GlitchOverlay flicker system to normal mode");
        }

        void testAnimationControllerClearsBypassAnimatorOnTeardown()
        {
            beginTest ("AnimationController clears the active bypass animator on editor teardown");

            juce::Component dummy;
            AnimationController controller (&dummy);

            expect (! controller.hasActiveBypassAnimator(),
                    "AnimationController should report no active bypass animator before a transition is started");

            controller.startBypassTransition (true);
            expect (controller.hasActiveBypassAnimator(),
                    "startBypassTransition should register an active bypass animator");

            const float mixBeforeClear = controller.getBypassMix();
            controller.clearAllAnimators();
            expect (! controller.hasActiveBypassAnimator(),
                    "clearAllAnimators should stop and clear the active bypass animator");
            expect (std::abs (controller.getBypassMix() - mixBeforeClear) < 1.0e-6f,
                    "clearAllAnimators should freeze the bypass mix at its last value rather than snapping it");
        }

        void testEditorTeardownLifecycleIsCrashFree()
        {
            beginTest ("Plugin editor teardown lifecycle does not crash on repeated close/reopen");

            for (int i = 0; i < 5; ++i)
            {
                AudioPluginAudioProcessor processor;
                AudioPluginAudioProcessorEditor editor (processor);

                // Start a bypass transition so an animator is mid-flight when
                // the editor is destructed, exercising the stopTimer ->
                // clearAllAnimators -> member destruct ordering fix for
                // issue #27.
                editor.getAnimationController().startBypassTransition (true);
            }

            // A fresh editor after the close/reopen loop must still construct
            // with its 60 Hz timer running, proving no lifecycle state was
            // corrupted by the teardown path.
            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);
            expect (editor.isTimerRunning(),
                    "Editor 60 Hz timer should be running after a close/reopen lifecycle loop");
        }
    };

    static AudioPluginTests synthortionTests;
}
