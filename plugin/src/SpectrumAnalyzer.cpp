#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/SynthortionLookAndFeel.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    // Initialize smoothed data array to zero
    juce::zeromem(smoothedScopeData, sizeof(smoothedScopeData));

    // Pre-allocate cached points for performance
    cachedPoints.ensureStorageAllocated(scopeSize);

    startTimerHz(60); // Increased from 30Hz to 60Hz for smoother animation
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::paint(juce::Graphics &g)
{
    auto outer = getLocalBounds().toFloat().reduced(2.0f);

    // Enhanced background with subtle texture
    juce::ColourGradient backgroundGrad(juce::Colour(0xff1a1a1a), outer.getTopLeft(),
                                        juce::Colour(0xff0f0f0f), outer.getBottomRight(), false);
    g.setGradientFill(backgroundGrad);
    g.fillRoundedRectangle(outer, 8.0f);

    // Plot area with some padding for axis labels
    auto plot = outer.reduced(30.0f, 18.0f);

    // Subtle border with inner glow
    g.setColour(juce::Colour(0xff333333).withAlpha(0.8f));
    g.drawRoundedRectangle(outer, 8.0f, 1.0f);

    // Inner subtle highlight
    g.setColour(juce::Colour(0xff444444).withAlpha(0.3f));
    g.drawRoundedRectangle(outer.reduced(1.0f), 7.0f, 0.5f);

    // Enhanced grid with better frequency distribution like SPAN
    const float nyquist = (float)(sampleRate * 0.5);

    // More balanced frequency grid lines optimized for uniform distribution
    const float majorFreqs[] = {20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f};
    const float minorFreqs[] = {25.f, 30.f, 40.f, 60.f, 80.f, 120.f, 150.f, 250.f, 300.f, 400.f, 600.f, 800.f,
                                1200.f, 1500.f, 2500.f, 3000.f, 4000.f, 6000.f, 8000.f, 12000.f, 15000.f, 16000.f};
    const int numMajorFreqs = (int)(sizeof(majorFreqs) / sizeof(majorFreqs[0]));
    const int numMinorFreqs = (int)(sizeof(minorFreqs) / sizeof(minorFreqs[0]));

    auto freqToX = [&](float f)
    {
        f = juce::jlimit(20.0f, nyquist, f);

        // Professional frequency mapping for uniform distribution like SPAN
        const float minFreq = 20.0f;
        const float maxFreq = nyquist;

        // Use SPAN-style distribution with multiple compression stages
        float logMin = std::log10(minFreq);
        float logMax = std::log10(maxFreq);
        float logF = std::log10(f);

        // Basic logarithmic normalization
        float norm = (logF - logMin) / (logMax - logMin);

        // Multi-stage frequency redistribution for professional appearance
        if (norm <= 0.5f)
        {
            // Compress low frequencies more aggressively (20Hz - 1kHz range)
            norm = std::pow(norm, 0.4f) * 0.3f;
        }
        else
        {
            // Mid and high frequencies get more space (1kHz - 20kHz range)
            float highNorm = (norm - 0.5f) / 0.5f;
            norm = 0.3f + std::pow(highNorm, 0.8f) * 0.7f;
        }

        return plot.getX() + norm * plot.getWidth();
    };

    // Minor grid lines (thinner, more transparent)
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.25f));
    for (int i = 0; i < numMinorFreqs; ++i)
    {
        if (minorFreqs[i] <= nyquist)
        {
            const float x = freqToX(minorFreqs[i]);
            g.drawLine(x, plot.getY(), x, plot.getBottom(), 0.5f);
        }
    }

    // Major grid lines (slightly thicker)
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.4f));
    for (int i = 0; i < numMajorFreqs; ++i)
    {
        if (majorFreqs[i] <= nyquist)
        {
            const float x = freqToX(majorFreqs[i]);
            g.drawLine(x, plot.getY(), x, plot.getBottom(), 0.8f);
        }
    }

    // Enhanced gain grid with major/minor lines
    auto gainMin = -60.f;
    auto gainMax = 0.f;
    auto gainToY = [&](float dB)
    {
        float norm = juce::jmap(dB, gainMin, gainMax, 0.0f, 1.0f);
        return plot.getBottom() - norm * plot.getHeight();
    };

    // Minor gain lines (every 5dB)
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.2f));
    for (int dB = (int)gainMin; dB <= (int)gainMax; dB += 5)
    {
        if (dB % 10 != 0) // Skip major lines
        {
            const float y = gainToY((float)dB);
            g.drawLine(plot.getX(), y, plot.getRight(), y, 0.5f);
        }
    }

    // Major gain lines (every 10dB)
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.4f));
    for (int dB = (int)gainMin; dB <= (int)gainMax; dB += 10)
    {
        const float y = gainToY((float)dB);
        g.drawLine(plot.getX(), y, plot.getRight(), y, 0.8f);
    }

    // Enhanced axis labels with better typography
    g.setColour(juce::Colour(0xffB4B4B4).withAlpha(0.8f));
    juce::Font labelFont(juce::FontOptions().withHeight(9.5f));
    g.setFont(labelFont);

    // Frequency labels with improved formatting
    const float labelFreqs[] = {20.f, 100.f, 1000.f, 10000.f, 20000.f};
    const int numLabelFreqs = (int)(sizeof(labelFreqs) / sizeof(labelFreqs[0]));

    for (int i = 0; i < numLabelFreqs; ++i)
    {
        if (labelFreqs[i] <= nyquist)
        {
            const float x = freqToX(labelFreqs[i]);
            juce::String label;

            if (labelFreqs[i] >= 1000.f)
                label = juce::String(labelFreqs[i] / 1000.f, (labelFreqs[i] == 1000.f || labelFreqs[i] == 10000.f || labelFreqs[i] == 20000.f) ? 0 : 1) + "k";
            else
                label = juce::String((int)labelFreqs[i]);

            const int rx = juce::roundToInt(x - 20.0f);
            const int ry = juce::roundToInt(plot.getBottom() + 3.0f);
            g.drawFittedText(label,
                             juce::Rectangle<int>(rx, ry, 40, 12),
                             juce::Justification::centred, 1);
        }
    }

    // Gain labels with improved spacing
    for (int dB = (int)gainMin; dB <= (int)gainMax; dB += 20) // Show every 20dB for cleaner look
    {
        const float y = gainToY((float)dB);
        const int rx = juce::roundToInt(plot.getX() - 28.0f);
        const int ry = juce::roundToInt(y - 6.0f);
        g.drawFittedText(juce::String(dB),
                         juce::Rectangle<int>(rx, ry, 25, 12),
                         juce::Justification::centredRight, 1);
    }

    drawFrame(g);
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
        repaint();
    }
    else
    {
        // Even without new FFT data, repaint for smooth animation
        // This ensures consistent 60fps visual updates
        repaint();
    }
}

void SpectrumAnalyzer::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            juce::zeromem(fftData, sizeof(fftData));
            memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void SpectrumAnalyzer::drawNextFrameOfSpectrum()
{
    window.multiplyWithWindowingTable(fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    const float mindB = -100.0f;
    const float maxdB = 0.0f;
    const float scale = 2.0f / (float)fftSize; // Correct normalization for JUCE FFT
    const float nyquist = (float)sampleRate * 0.5f;
    const float minFreq = 20.0f;

    for (int i = 0; i < scopeSize; ++i)
    {
        // Calculate logarithmic frequency for this scope bin
        float logPos = (float)i / (float)(scopeSize - 1); // 0 to 1
        float freq = minFreq * std::pow(nyquist / minFreq, logPos);

        // Convert frequency to FFT bin index
        float binFloat = freq * (float)fftSize / (float)sampleRate;
        int fftIndex = juce::jlimit(0, fftSize / 2 - 1, (int)binFloat);

        // Interpolate between adjacent bins for smoother results
        float frac = binFloat - (float)fftIndex;
        float mag1 = fftData[fftIndex] * scale;
        float mag2 = (fftIndex + 1 < fftSize / 2) ? fftData[fftIndex + 1] * scale : mag1;
        float mag = mag1 + frac * (mag2 - mag1);

        float dB = juce::Decibels::gainToDecibels(juce::jmax(mag, 1.0e-9f));
        float level = juce::jlimit(0.0f, 1.0f, juce::jmap(dB, mindB, maxdB, 0.0f, 1.0f));

        // Apply temporal smoothing for fluid animation
        smoothedScopeData[i] = smoothedScopeData[i] * smoothingFactor + level * (1.0f - smoothingFactor);
        scopeData[i] = smoothedScopeData[i];
    }
}

void SpectrumAnalyzer::drawFrame(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(30.0f, 18.0f);
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    const float nyquist = (float)sampleRate * 0.5f;

    // Enable high-quality anti-aliasing
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    // Update cached points only when needed
    cachedPoints.clearQuick();
    cachedPoints.ensureStorageAllocated(scopeSize);

    // Collect points with optimized loop using consistent logarithmic mapping
    for (int i = 1; i < scopeSize; ++i)
    {
        // Use same professional frequency mapping as in paint() method
        float logPos = (float)i / (float)(scopeSize - 1); // 0 to 1
        float freq = 20.0f * std::pow(nyquist / 20.0f, logPos);

        // Apply the same professional frequency distribution for consistency
        const float minFreq = 20.0f;
        const float maxFreq = nyquist;

        float logMin = std::log10(minFreq);
        float logMax = std::log10(maxFreq);
        float logF = std::log10(freq);

        // Apply the same multi-stage distribution for consistent appearance
        float norm = (logF - logMin) / (logMax - logMin);

        // Multi-stage frequency redistribution (same as paint method)
        if (norm <= 0.5f)
        {
            // Compress low frequencies more aggressively (20Hz - 1kHz range)
            norm = std::pow(norm, 0.4f) * 0.3f;
        }
        else
        {
            // Mid and high frequencies get more space (1kHz - 20kHz range)
            float highNorm = (norm - 0.5f) / 0.5f;
            norm = 0.3f + std::pow(highNorm, 0.8f) * 0.7f;
        }

        auto x = bounds.getX() + norm * width;
        auto y = bounds.getBottom() - juce::jmap(scopeData[i], 0.0f, 1.0f, 0.0f, height);
        cachedPoints.add({x, y});
    }

    juce::Path spectrumPath;

    // Create smooth path using optimized curve fitting
    if (cachedPoints.size() > 2)
    {
        spectrumPath.preallocateSpace(cachedPoints.size() * 3); // Pre-allocate for performance
        spectrumPath.startNewSubPath(cachedPoints[0]);

        // Use quadratic curves for smoother appearance with optimized control points
        for (int i = 1; i < cachedPoints.size() - 1; ++i)
        {
            auto p1 = cachedPoints[i];
            auto p2 = cachedPoints[i + 1];

            // Optimized control point calculation
            auto controlX = (p1.x + p2.x) * 0.5f;
            auto controlY = juce::jmin(p1.y, p2.y);

            spectrumPath.quadraticTo(p1.x, p1.y, controlX, controlY);
        }

        if (cachedPoints.size() > 1)
            spectrumPath.lineTo(cachedPoints.getLast());
    }

    // Fill under the spectrum with enhanced gradient
    juce::Path filledPath = spectrumPath;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    // Multi-stop gradient for more depth
    juce::ColourGradient spectrumGradient(
        PURPLE.withAlpha(0.45f),
        bounds.getCentreX(), bounds.getY(),
        PURPLE_DARK.withAlpha(0.05f),
        bounds.getCentreX(), bounds.getBottom(), false);

    spectrumGradient.addColour(0.3, PURPLE.withAlpha(0.25f));

    g.setGradientFill(spectrumGradient);
    g.fillPath(filledPath);

    // Draw main spectrum line with optimized glow effect
    juce::PathStrokeType strokeType(2.0f);
    strokeType.setJointStyle(juce::PathStrokeType::curved);
    strokeType.setEndStyle(juce::PathStrokeType::rounded);

    // Reduced glow layers for better performance while maintaining quality
    const int glowLayers = 2; // Reduced from 3
    for (int glow = glowLayers; glow >= 1; --glow)
    {
        float glowWidth = 2.0f + (glow * 1.0f); // Reduced glow spread
        float alpha = 0.25f / glow;

        g.setColour(PURPLE.withAlpha(alpha));
        juce::PathStrokeType glowStroke(glowWidth);
        glowStroke.setJointStyle(juce::PathStrokeType::curved);
        glowStroke.setEndStyle(juce::PathStrokeType::rounded);
        g.strokePath(spectrumPath, glowStroke);
    }

    // Main line with enhanced brightness
    g.setColour(PURPLE.brighter(0.3f));
    g.strokePath(spectrumPath, strokeType);
}
