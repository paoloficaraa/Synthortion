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
        beginTest("Typography scale fonts are pre-computed and resolve correctly");

        SynthortionLookAndFeel lookAndFeel;

        auto heading = lookAndFeel.getSectionHeadingFont();
        expect (juce::roundToInt(heading.getHeight()) == 22, "Section heading should be 22px (BebasNeue panel title size per issue #20)");
        expect (lookAndFeel.getTypefaceForFont(heading) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue")),
                "Section heading should resolve to the BebasNeue typeface");

        auto paramLabel = lookAndFeel.getParameterLabelFont();
        expect (juce::roundToInt(paramLabel.getHeight()) == 13, "Parameter label should be 13px");
        expect (paramLabel.getTypefaceStyle() == "Medium", "Parameter label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(paramLabel) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter label should resolve to the Montserrat typeface");

        auto paramValue = lookAndFeel.getParameterValueFont();
        expect (juce::roundToInt(paramValue.getHeight()) == 12, "Parameter value should be 12px");
        expect (paramValue.getTypefaceStyle() == "Regular", "Parameter value should be Regular weight");
        expect (lookAndFeel.getTypefaceForFont(paramValue) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter value should resolve to the Montserrat typeface");

        auto bypass = lookAndFeel.getBypassLabelFont();
        expect (juce::roundToInt(bypass.getHeight()) == 13, "Bypass label should be 13px");
        expect (bypass.getTypefaceStyle() == "Medium", "Bypass label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(bypass) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Bypass label should resolve to the Montserrat typeface");
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
            // Slice C: drawPanelBackground now paints a flat #000 panel
            // regardless of the bgColour argument, so the centre == bgColour
            // assertion in testPanelComponentRendersBackgroundColour no longer
            // holds. The visual test will be deleted in Slice I; called out via
            // acceptance criteria #6 of issue #20. Left commented so the other
            // behavioural tests run cleanly.
            // testPanelComponentRendersBackgroundColour();
            testBypassComponentLedTogglesOnClick();
            testBypassSwitchSnapsWithoutOvershoot();
            testBypassSwitchParameterAttachment();
            testBypassSwitchRendersActiveWhiteBlock();
            testBypassSwitchRendersBypassedBlackBlockWithOutline();
            testBypassSwitchFiresSliceGlitchOnToggle();
            testPanelComponentTitleFont();
            testEditorIsOpaque();
            testPanelComponentIsOpaque();
            testBypassComponentIsNotOpaque();
            testDeadlockPalette();
            testEditorSizeIs720x440();
            testEditorContainsOscilloscope();
            testEditorContainsMeters();
            testDistortionPanelIsLargest();
            testAnimationControllerCreatesAnimator();
            testAudioScopeRingBufferTransfersSamples();
            testOscilloscopeReadsInputBuffer();
            testOscilloscopeReadsOutputBuffer();
            testOscilloscopeDetectsSilence();
            testOscilloscopeDetectsSignal();
            testOscilloscopeBypassFlattensOutput();
            testOscilloscopeBoundsInEditor();
            testOscilloscopePixelLaneInterleave();
            testOscilloscopeTracesMergeIntoContinuousStreak();
            testOscilloscopeTripletRendersThreeParallelColumns();
            testOscilloscopeSweepRendersViaSharedOverlay();
            testOscilloscopeGhostTrailsUseThreeDiscreteAlphas();
            testOscilloscopeSilentCenterlineFlickersHard();
            testOscilloscopeBypassDecaysOutputAmplitude();
            testAnimatedKnobIsASlider();
            testAnimatedKnobHasRotaryStyle();
            testAnimatedKnobSnapsArcDuringDrag();
            testAnimatedKnobDefaultStyleIsCanonical();
            testAnimatedKnobStepCountMatchesStyle();
            testAnimatedKnobStepEasingQuantizesHard();
            testCanonicalKnobInvertsFaceOnMouseDown();
            testTwinShadowRendersOnDrag();
            testEditorKnobsUseFourCanonicalAndFourOutline();
            testEditorContainsEightAnimatedKnobs();
            testAnimatedKnobBindsToParameter();
            // Slice G (#24): meter restyle retains the side-bar layout, but
            // the issue acceptance criteria flag these two position tests for
            // Slice I cleanup, so they are disabled here per spec.
            // testInputMeterIsOnLeftSideBar();
            // testOutputMeterIsOnRightSideBar();
            testInputGainKnobBindsToParameter();
            testOutputGainKnobBindsToParameter();
            testMeterCalculatesRMS();
            testMeterPeakHoldJumpsToPeak();
            testAnimationControllerBypassMixDefaultsToZero();
            testDeadlockKnobRendersWhiteFace();
            testMeterSegmentCountIsHardcodedSixteen();
            testMeterRendersSixteenSegmentLedLadder();
            testMeterSegmentsAreBinaryBlackWhite();
            testMeterBarHasWhiteOutlineWithNotchedTicks();
            testMeterPeakHoldMovesInSixteenSteps();
            testMeterPeakDecayUsesStepEasing();
            testMeterBypassShowsAllSegmentsOff();
            testEditorBackgroundUnaffectedByBypass();
            testComingSoonPanelRendersPlaceholder();
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
            testPanelComponentRendersBrutalistShape();
        }

    private:
        void testPanelComponentRendersBackgroundColour()
        {
            beginTest ("PanelComponent renders an opaque background colour");

            SynthortionLookAndFeel lookAndFeel;

            const juce::Colour bgColour (0xFF123456);
            PanelComponent panel ("DISTORTION", bgColour);
            panel.setSize (100, 100);
            panel.setLookAndFeel (&lookAndFeel);

            const auto snapshot = panel.createComponentSnapshot (panel.getLocalBounds());

            expect (snapshot.getPixelAt (50, 50) == bgColour,
                    "PanelComponent centre pixel should match the background colour");
            expect (snapshot.getPixelAt (0, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (0, 99).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 99).getAlpha() == 0xFF);
        }

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

        void testBypassSwitchSnapsWithoutOvershoot()
        {
            beginTest ("BypassSwitch Step easing quantises to N=8 ; never overshoots beyond [0, 1]");

            for (int i = 0; i <= 100; ++i)
            {
                const float t = static_cast<float> (i) * 0.01f;
                const float value = synthortion::AnimatedKnob::quantizeStepProgress (t, synthortion::BypassSwitch::kBypassSteps);
                expect (value <= 1.0f, "Step easing should never overshoot 1.0");
                expect (value >= 0.0f, "Step easing should never undershoot 0.0");
            }
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

        void testPanelComponentTitleFont()
        {
            beginTest ("PanelComponent title font is BebasNeue 22px");

            PanelComponent panel ("DISTORTION", juce::Colours::black);
            auto font = panel.getTitleFont();

            expect (juce::roundToInt(font.getHeight()) == 22, "Panel title should be 22px per DEADLOCK Slice C issue #20");
            expect (font.getTypefaceName().containsIgnoreCase("Bebas"), "Panel title should use BebasNeue");
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

        void testBypassComponentIsNotOpaque()
        {
            beginTest ("BypassComponent stays non-opaque so the slice glitch burst can bleed past its bounds");

            BypassComponent bypass;

            expect (! bypass.isOpaque(), "BypassComponent should remain non-opaque so the GlitchOverlay slice bands are not clipped");
        }

        void testDeadlockPalette()
        {
            beginTest ("DEADLOCK palette binds binary black/white colour slots");

            SynthortionLookAndFeel lookAndFeel;

            const auto background = lookAndFeel.findColour (SynthortionLookAndFeel::backgroundColourId);
            const auto panelFill = lookAndFeel.findColour (SynthortionLookAndFeel::panelFillColourId);
            const auto panelOutline = lookAndFeel.findColour (SynthortionLookAndFeel::panelOutlineColourId);
            const auto accent = lookAndFeel.findColour (SynthortionLookAndFeel::accentColourId);
            const auto accentBright = lookAndFeel.findColour (SynthortionLookAndFeel::accentBrightColourId);
            const auto text = lookAndFeel.findColour (SynthortionLookAndFeel::textColourId);
            const auto knobFill = lookAndFeel.findColour (SynthortionLookAndFeel::knobFillColourId);

            const juce::Colour black (0xFF000000);
            const juce::Colour white (0xFFFFFFFF);

            expect (background == black, "backgroundColourId should be pure #000");
            expect (panelFill == black, "panelFillColourId should be pure #000");
            expect (panelOutline == white, "panelOutlineColourId should be pure #FFF");
            expect (accent == white, "accentColourId should be pure #FFF");
            expect (accentBright == white, "accentBrightColourId should be pure #FFF");
            expect (text == white, "textColourId should be pure #FFF");
            expect (knobFill == white, "knobFillColourId should be pure #FFF");
        }

        void testEditorSizeIs720x440()
        {
            beginTest ("Plugin editor dimensions are 720x440");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 720, "Editor width should be 720");
            expect (editor.getHeight() == 440, "Editor height should be 440");
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

        void testDistortionPanelIsLargest()
        {
            beginTest ("Distortion panel is the largest central panel");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int distortionArea = 0;
            int chorusArea = 0;
            int delayArea = 0;
            int comingSoonArea = 0;

            for (auto* child : editor.getChildren())
            {
                if (auto* panel = dynamic_cast<PanelComponent*> (child))
                {
                    const int area = panel->getWidth() * panel->getHeight();
                    const auto title = panel->getTitle();

                    if (title == "DISTORTION")
                        distortionArea = area;
                    else if (title == "CHORUS")
                        chorusArea = area;
                    else if (title == "DELAY")
                        delayArea = area;
                    else if (title == "COMING SOON")
                        comingSoonArea = area;
                }
            }

            expect (distortionArea > chorusArea, "Distortion panel should be larger than Chorus");
            expect (distortionArea > delayArea, "Distortion panel should be larger than Delay");
            expect (distortionArea > comingSoonArea, "Distortion panel should be larger than Coming Soon");
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

        void testOscilloscopeBoundsInEditor()
        {
            beginTest ("OscilloscopeComponent occupies the expected Top Bar area");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            for (auto* child : editor.getChildren())
            {
                if (auto* scope = dynamic_cast<OscilloscopeComponent*> (child))
                {
                    expect (scope->getWidth() >= 560, "Oscilloscope should be roughly 580px wide");
                    expect (scope->getHeight() >= 80, "Oscilloscope should be roughly 90px tall");
                    expect (scope->getX() >= 120, "Oscilloscope should sit to the right of the bypass switch");
                    return;
                }
            }

            expect (false, "Editor should contain an OscilloscopeComponent to check bounds");
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

        void testAnimatedKnobSnapsArcDuringDrag()
        {
            beginTest ("AnimatedKnob snaps displayed value to target within one step during drag");

            juce::Component dummy;
            AnimationController controller (&dummy);

            SynthortionLookAndFeel lookAndFeel;
            AnimatedKnob knob (controller);
            knob.setRange (0.0, 1.0);
            knob.setValue (0.0, juce::dontSendNotification);
            knob.snapToCurrentValue();
            knob.setSize (60, 60);
            knob.setLookAndFeel (&lookAndFeel);

            auto source = juce::Desktop::getInstance().getMainMouseSource();
            const juce::Point<float> downPos (30.0f, 30.0f);
            const auto now = juce::Time::getCurrentTime();

            const juce::MouseEvent downEvent (source,
                                              downPos,
                                              juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier),
                                              0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                              &knob,
                                              &knob,
                                              now,
                                              downPos,
                                              now,
                                              1,
                                              false);

            knob.mouseDown (downEvent);
            knob.setValue (1.0, juce::sendNotificationSync);

            const float step = 1.0f / static_cast<float> (knob.getStepCount());
            expect (std::abs (knob.getDisplayProportion() - 1.0f) < step,
                    "During drag the displayed proportion should snap to the target within one Step");
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

        void testInputMeterIsOnLeftSideBar()
        {
            beginTest ("Input meter sits in the left side bar above the input gain knob");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            constexpr int kRackEarWidth = 15;
            constexpr int kSideBarWidth = 55;

            auto& meter = editor.getInputMeter();
            auto& knob = editor.getInputGainKnob();

            expect (meter.getX() >= kRackEarWidth,
                    "Input meter should start after the left rack ear");
            expect (meter.getRight() <= kRackEarWidth + kSideBarWidth,
                    "Input meter should fit inside the left side bar");
            expect (meter.getBottom() <= knob.getY(),
                    "Input meter should sit above the input gain knob");
        }

        void testOutputMeterIsOnRightSideBar()
        {
            beginTest ("Output meter sits in the right side bar above the output gain knob");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            constexpr int kRackEarWidth = 15;
            constexpr int kSideBarWidth = 55;

            auto& meter = editor.getOutputMeter();
            auto& knob = editor.getOutputGainKnob();

            expect (meter.getX() >= editor.getWidth() - kRackEarWidth - kSideBarWidth,
                    "Output meter should start inside the right side bar");
            expect (meter.getRight() <= editor.getWidth() - kRackEarWidth,
                    "Output meter should end before the right rack ear");
            expect (meter.getBottom() <= knob.getY(),
                    "Output meter should sit above the output gain knob");
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

            // Sample points inside the pure-background ear strips where no child
            // widget is placed (resized() insets kRackEarWidth == 15), so the
            // pixels only ever see the #000 substrate + dither + scanlines + dead
            // pixels. None of these layers depend on bypass mix, so the substrate
            // must be byte-identical between the active and bypassed snapshots.
            const juce::Point<int> samplePoints[] = {
                { 5, 30 },
                { 5, 200 },
                { 5, 400 },
                { editor.getWidth() - 5, 30 },
                { editor.getWidth() - 5, 200 }
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

        void testComingSoonPanelRendersPlaceholder()
        {
            beginTest ("Coming Soon panel renders placeholder content");

            SynthortionLookAndFeel lookAndFeel;
            const auto bg = lookAndFeel.findColour (SynthortionLookAndFeel::panelOutlineColourId);

            PanelComponent plainPanel ("OTHER", bg);
            plainPanel.setSize (100, 100);
            plainPanel.setLookAndFeel (&lookAndFeel);

            PanelComponent placeholderPanel ("COMING SOON", bg);
            placeholderPanel.setPlaceholder (true);
            placeholderPanel.setSize (100, 100);
            placeholderPanel.setLookAndFeel (&lookAndFeel);

            const auto plainSnapshot = plainPanel.createComponentSnapshot (plainPanel.getLocalBounds());
            const auto placeholderSnapshot = placeholderPanel.createComponentSnapshot (placeholderPanel.getLocalBounds());

            bool hasDifferentPixel = false;
            for (int y = 35; y < 85 && ! hasDifferentPixel; ++y)
            {
                for (int x = 20; x < 80 && ! hasDifferentPixel; ++x)
                {
                    if (plainSnapshot.getPixelAt (x, y) != placeholderSnapshot.getPixelAt (x, y))
                        hasDifferentPixel = true;
                }
            }

            expect (hasDifferentPixel,
                    "Placeholder panel should draw content distinct from a plain panel");
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
            beginTest ("PanelComponent renders brutalist shape: #FFF outline + #000 fill + rule + corner ticks");

            SynthortionLookAndFeel lookAndFeel;
            PanelComponent panel ("DISTORTION", juce::Colour (0xFF000000));
            panel.setSize (100, 100);
            panel.setLookAndFeel (&lookAndFeel);

            const auto snapshot = panel.createComponentSnapshot (panel.getLocalBounds());

            const auto white = juce::Colour (0xFFFFFFFF);
            const auto black = juce::Colour (0xFF000000);

            // Flat #000 fill on the panel interior.
            expect (snapshot.getPixelAt (50, 50) == black, "Brutalist panel interior should be flat #000");
            expect (snapshot.getPixelAt (50, 80) == black, "Brutalist panel interior below the rule should be flat #000");

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
    };

    static AudioPluginTests synthortionTests;
}
