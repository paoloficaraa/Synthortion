#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/ParametricEQ.h"
#include <array>

SpectrumAnalyzer::SpectrumAnalyzer()
{
    cachedPoints.ensureStorageAllocated(kScopeSize);
    startTimerHz(kTimerHz);
}

float SpectrumAnalyzer::frequencyToX(float freq, const juce::Rectangle<float>& plot) const
{
    const float nyquist = static_cast<float>(sampleRate * 0.5);
    freq = juce::jlimit(kMinFreq, nyquist, freq);

    const float logMin = std::log10(kMinFreq);
    const float logMax = std::log10(nyquist);
    const float logF = std::log10(freq);

    float normalizedLog = (logF - logMin) / (logMax - logMin);
    normalizedLog = juce::jlimit(0.0f, 1.0f, normalizedLog);

    return plot.getX() + normalizedLog * plot.getWidth();
}

float SpectrumAnalyzer::gainToY(float gainDb, const juce::Rectangle<float>& plot) const
{
    const float norm = juce::jmap(gainDb, -kGainRangeDb, kMaxGainDb, 0.0f, 1.0f);
    return plot.getBottom() - norm * plot.getHeight();
}

void SpectrumAnalyzer::drawGrid(juce::Graphics &g, const juce::Rectangle<float>& plot)
{
    if (sampleRate <= 0.0)
        return;

    const float nyquist = static_cast<float>(sampleRate * 0.5);

    static constexpr std::array majorFreqs{20.f, 100.f, 1000.f, 10000.f, 20000.f};
    static constexpr std::array minorFreqs{30.f, 40.f, 50.f, 60.f, 70.f, 80.f, 90.f, 
                                           200.f, 300.f, 400.f, 500.f, 600.f, 700.f, 800.f, 900.f, 
                                           2000.f, 3000.f, 4000.f, 5000.f, 6000.f, 7000.f, 8000.f, 9000.f};

    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.25f));
    for (const float freq : minorFreqs)
    {
        if (freq <= nyquist)
        {
            const float x = frequencyToX(freq, plot);
            g.drawLine(x, plot.getY(), x, plot.getBottom(), 0.5f);
        }
    }

    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.4f));
    for (const float freq : majorFreqs)
    {
        if (freq <= nyquist)
        {
            const float x = frequencyToX(freq, plot);
            g.drawLine(x, plot.getY(), x, plot.getBottom(), 0.8f);
        }
    }

    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.2f));
    for (int dB = -static_cast<int>(kGainRangeDb); dB <= 0; dB += 5)
    {
        if (dB % 10 != 0)
        {
            const float y = gainToY(static_cast<float>(dB), plot);
            g.drawLine(plot.getX(), y, plot.getRight(), y, 0.5f);
        }
    }

    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.4f));
    for (int dB = -static_cast<int>(kGainRangeDb); dB <= 0; dB += 10)
    {
        const float y = gainToY(static_cast<float>(dB), plot);
        g.drawLine(plot.getX(), y, plot.getRight(), y, 0.8f);
    }
}

void SpectrumAnalyzer::drawFrequencyLabels(juce::Graphics &g, const juce::Rectangle<float>& plot)
{
    if (sampleRate <= 0.0)
        return;

    const float nyquist = static_cast<float>(sampleRate * 0.5);
    static constexpr std::array labelFreqs{20.f, 100.f, 500.f, 1000.f, 5000.f, 10000.f, 20000.f};

    g.setColour(juce::Colour(0xffB4B4B4).withAlpha(0.8f));
    g.setFont(juce::Font(juce::FontOptions().withHeight(9.5f)));

    for (const float freq : labelFreqs)
    {
        if (freq > nyquist)
            continue;

        const float x = frequencyToX(freq, plot);
        juce::String label;

        if (freq >= 1000.f && static_cast<int>(freq) % 1000 == 0)
            label = juce::String(static_cast<int>(freq / 1000.f)) + "K";
        else
            label = juce::String(static_cast<int>(freq));

        const int rx = juce::roundToInt(x - 20.0f);
        const int ry = juce::roundToInt(plot.getBottom() + 3.0f);

        g.drawFittedText(label, juce::Rectangle<int>(rx, ry, 40, 12),
                        juce::Justification::centred, 1);
    }
}

void SpectrumAnalyzer::paint(juce::Graphics &g)
{
    if (sampleRate <= 0.0)
    {
        g.fillAll(juce::Colours::black);
        return;
    }

    const auto outer = getLocalBounds().toFloat().reduced(kOuterPadding);

    juce::ColourGradient backgroundGrad(juce::Colour(0xff1a1a1a), outer.getTopLeft(),
                                        juce::Colour(0xff0f0f0f), outer.getBottomRight(), false);
    g.setGradientFill(backgroundGrad);
    g.fillRoundedRectangle(outer, kCornerRadius);

    const auto plot = outer.reduced(kPlotPaddingH, kPlotPaddingV);

    g.setColour(juce::Colour(0xff333333).withAlpha(0.8f));
    g.drawRoundedRectangle(outer, kCornerRadius, 1.0f);

    g.setColour(juce::Colour(0xff444444).withAlpha(0.3f));
    g.drawRoundedRectangle(outer.reduced(1.0f), kInnerCornerRadius, 0.5f);

    drawGrid(g, plot);
    drawFrequencyLabels(g, plot);
    drawFrame(g);

    if (eqReference != nullptr)
        drawEQCurve(g);
}

void SpectrumAnalyzer::resized()
{
}

void SpectrumAnalyzer::timerCallback()
{
    if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;

        for (int i = 0; i < kScopeSize; ++i)
            targetScopeData[i] = smoothedScopeData[i];
    }

    bool hasChanges = false;

    for (int i = 0; i < kScopeSize; ++i)
    {
        const float oldValue = currentScopeData[i];
        currentScopeData[i] += (targetScopeData[i] - currentScopeData[i]) * kInterpolationSpeed;

        targetScopeData[i] *= kTargetDecay;

        if (std::abs(oldValue - currentScopeData[i]) > kChangeThreshold)
            hasChanges = true;

        if (peakHoldTimer[i] > 0)
        {
            peakHoldTimer[i]--;
        }
        else
        {
            peakHoldData[i] *= kPeakDecay;
        }

        scopeData[i] = currentScopeData[i];
    }

    if (hasChanges || eqReference != nullptr)
        repaint();
}

void SpectrumAnalyzer::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == kFftSize)
    {
        if (!nextFFTBlockReady)
        {
            fftData.fill(0.0f);
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void SpectrumAnalyzer::drawNextFrameOfSpectrum()
{
    window.multiplyWithWindowingTable(fftData.data(), kFftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

    const float scale = kFftScale / static_cast<float>(kFftSize) * kWindowCorrection;
    const float nyquist = static_cast<float>(sampleRate * 0.5);

    for (int i = 0; i < kScopeSize; ++i)
    {
        const float logPos = static_cast<float>(i) / static_cast<float>(kScopeSize - 1);
        const float freq = kMinFreq * std::pow(nyquist / kMinFreq, logPos);

        const float binFloat = freq * static_cast<float>(kFftSize) / static_cast<float>(sampleRate);
        const int fftIndex = juce::jlimit(0, kFftSize / 2 - 1, static_cast<int>(binFloat));

        const float frac = binFloat - static_cast<float>(fftIndex);
        const float mag1 = fftData[fftIndex] * scale;
        const float mag2 = (fftIndex + 1 < kFftSize / 2) ? fftData[fftIndex + 1] * scale : mag1;
        const float mag = mag1 + frac * (mag2 - mag1);

        const float dB = juce::Decibels::gainToDecibels(juce::jmax(mag, kMinMagnitude));
        const float level = juce::jlimit(0.0f, 1.0f, 
                                        juce::jmap(dB, kMinGainDb, kMaxGainDb, 0.0f, 1.0f));

        smoothedScopeData[i] = smoothedScopeData[i] * kSmoothingFactor + 
                               level * (1.0f - kSmoothingFactor);

        if (level > peakHoldData[i])
        {
            peakHoldData[i] = level;
            peakHoldTimer[i] = kPeakHoldTime;
        }
    }
}

void SpectrumAnalyzer::drawFrame(juce::Graphics &g)
{
    if (sampleRate <= 0.0)
        return;

    const auto bounds = getLocalBounds().toFloat().reduced(kPlotPaddingH, kPlotPaddingV);
    const auto height = bounds.getHeight();
    const float nyquist = static_cast<float>(sampleRate * 0.5);

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    cachedPoints.clearQuick();
    cachedPoints.ensureStorageAllocated(kScopeSize);

    for (int i = 1; i < kScopeSize; ++i)
    {
        const float logPos = static_cast<float>(i) / static_cast<float>(kScopeSize - 1);
        const float freq = kMinFreq * std::pow(nyquist / kMinFreq, logPos);

        const float x = frequencyToX(freq, bounds);
        const float y = bounds.getBottom() - juce::jmap(scopeData[i], 0.0f, 1.0f, 0.0f, height);

        if (std::isfinite(x) && std::isfinite(y))
            cachedPoints.add({x, y});
    }

    juce::Path spectrumPath;

    if (cachedPoints.size() > 3)
    {
        spectrumPath.preallocateSpace(cachedPoints.size() * 4);
        spectrumPath.startNewSubPath(cachedPoints[0]);

        for (int i = 1; i < cachedPoints.size() - 2; ++i)
        {
            const auto p0 = cachedPoints[i - 1];
            const auto p1 = cachedPoints[i];
            const auto p2 = cachedPoints[i + 1];
            const auto p3 = cachedPoints[i + 2];

            const auto cp1x = p1.x + (p2.x - p0.x) * 0.16f;
            const auto cp1y = p1.y + (p2.y - p0.y) * 0.16f;
            const auto cp2x = p2.x - (p3.x - p1.x) * 0.16f;
            const auto cp2y = p2.y - (p3.y - p1.y) * 0.16f;

            spectrumPath.cubicTo(cp1x, cp1y, cp2x, cp2y, p2.x, p2.y);
        }

        if (cachedPoints.size() > 1)
            spectrumPath.lineTo(cachedPoints.getLast());
    }
    else if (cachedPoints.size() > 1)
    {
        spectrumPath.startNewSubPath(cachedPoints[0]);
        for (int i = 1; i < cachedPoints.size(); ++i)
            spectrumPath.lineTo(cachedPoints[i]);
    }

    juce::Path filledPath = spectrumPath;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    juce::ColourGradient spectrumGradient(
        PURPLE.withAlpha(0.45f), bounds.getCentreX(), bounds.getY(),
        PURPLE_DARK.withAlpha(0.05f), bounds.getCentreX(), bounds.getBottom(), false);
    spectrumGradient.addColour(0.3, PURPLE.withAlpha(0.25f));

    g.setGradientFill(spectrumGradient);
    g.fillPath(filledPath);

    juce::PathStrokeType strokeType(kMainLineWidth);
    strokeType.setJointStyle(juce::PathStrokeType::curved);
    strokeType.setEndStyle(juce::PathStrokeType::rounded);

    for (int glow = kGlowLayers; glow >= 1; --glow)
    {
        const float glowWidth = kMainLineWidth + (glow * 1.0f);
        const float alpha = 0.25f / glow;

        g.setColour(PURPLE.withAlpha(alpha));
        juce::PathStrokeType glowStroke(glowWidth);
        glowStroke.setJointStyle(juce::PathStrokeType::curved);
        glowStroke.setEndStyle(juce::PathStrokeType::rounded);
        g.strokePath(spectrumPath, glowStroke);
    }

    g.setColour(PURPLE.brighter(0.3f));
    g.strokePath(spectrumPath, strokeType);
}

void SpectrumAnalyzer::drawEQCurve(juce::Graphics &g)
{
    if (eqReference == nullptr)
        return;

    const auto outer = getLocalBounds().toFloat().reduced(kOuterPadding);
    const auto plot = outer.reduced(kPlotPaddingH, kPlotPaddingV);

    std::vector<float> frequencies;
    frequencies.reserve(kEqCurvePoints);

    for (int i = 0; i < kEqCurvePoints; ++i)
    {
        const float ratio = static_cast<float>(i) / static_cast<float>(kEqCurvePoints - 1);
        const float freq = kMinFreq * std::pow(kMaxFreq / kMinFreq, ratio);
        frequencies.push_back(freq);
    }

    const auto magnitudes = eqReference->getFrequencyResponse(frequencies);

    juce::Path eqPath;
    bool firstPoint = true;

    for (size_t i = 0; i < frequencies.size() && i < magnitudes.size(); ++i)
    {
        const float x = frequencyToX(frequencies[i], plot);
        const float y = juce::jlimit(plot.getY(), plot.getBottom(),
                                     plot.getY() + plot.getHeight() * 0.5f * (1.0f - magnitudes[i] / 20.0f));

        if (firstPoint)
        {
            eqPath.startNewSubPath(x, y);
            firstPoint = false;
        }
        else
        {
            eqPath.lineTo(x, y);
        }
    }

    if (!eqPath.isEmpty())
    {
        juce::PathStrokeType strokeType(kEqLineWidth);
        strokeType.setJointStyle(juce::PathStrokeType::curved);
        strokeType.setEndStyle(juce::PathStrokeType::rounded);

        g.setColour(PURPLE.withAlpha(0.5f));
        juce::PathStrokeType glowStroke(kGlowWidth);
        glowStroke.setJointStyle(juce::PathStrokeType::curved);
        g.strokePath(eqPath, glowStroke);

        g.setColour(PURPLE.brighter(0.4f));
        g.strokePath(eqPath, strokeType);

        g.setColour(PURPLE_DARK.withAlpha(0.6f));
        const float centerY = plot.getY() + plot.getHeight() * 0.5f;
        g.drawHorizontalLine(static_cast<int>(centerY), plot.getX(), plot.getRight());
    }
}
