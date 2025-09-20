#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/SynthortionLookAndFeel.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    // Initialize smoothed data array to zero
    juce::zeromem(smoothedScopeData, sizeof(smoothedScopeData));
    juce::zeromem(targetScopeData, sizeof(targetScopeData));
    juce::zeromem(currentScopeData, sizeof(currentScopeData));
    juce::zeromem(peakHoldData, sizeof(peakHoldData));
    juce::zeromem(peakHoldTimer, sizeof(peakHoldTimer));

    // Pre-allocate cached points for performance
    cachedPoints.ensureStorageAllocated(scopeSize);

    startTimerHz(60); // Timer più veloce per interpolazione fluida
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::paint(juce::Graphics &g)
{
    // Safety check for valid sample rate
    if (sampleRate <= 0.0)
    {
        g.fillAll(juce::Colours::black);
        return;
    }

    auto outer = getLocalBounds().toFloat().reduced(2.0f);

    // Enhanced background with subtle texture
    juce::ColourGradient backgroundGrad(juce::Colour(0xff1a1a1a), outer.getTopLeft(),
                                        juce::Colour(0xff0f0f0f), outer.getBottomRight(), false);
    g.setGradientFill(backgroundGrad);
    g.fillRoundedRectangle(outer, 8.0f);

    // Plot area ottimizzato - padding ridotto a sinistra senza etichette Y
    auto plot = outer.reduced(15.0f, 18.0f); // Ridotto padding sinistro da 30 a 15

    // Subtle border with inner glow
    g.setColour(juce::Colour(0xff333333).withAlpha(0.8f));
    g.drawRoundedRectangle(outer, 8.0f, 1.0f);

    // Inner subtle highlight
    g.setColour(juce::Colour(0xff444444).withAlpha(0.3f));
    g.drawRoundedRectangle(outer.reduced(1.0f), 7.0f, 0.5f);

    // Enhanced grid with better frequency distribution like SPAN
    const float nyquist = (float)(sampleRate * 0.5);

    // More balanced frequency grid lines ottimizzato come Klangfreund
    const float majorFreqs[] = {20.f, 100.f, 1000.f, 10000.f, 20000.f};
    const float minorFreqs[] = {30.f, 40.f, 50.f, 60.f, 70.f, 80.f, 90.f, 200.f, 300.f, 400.f, 500.f, 600.f, 700.f, 800.f, 900.f, 2000.f, 3000.f, 4000.f, 5000.f, 6000.f, 7000.f, 8000.f, 9000.f};
    const int numMajorFreqs = (int)(sizeof(majorFreqs) / sizeof(majorFreqs[0]));
    const int numMinorFreqs = (int)(sizeof(minorFreqs) / sizeof(minorFreqs[0]));

    auto freqToX = [&](float f) -> float
    {
        f = juce::jlimit(20.0f, nyquist, f);

        // Safe logarithmic mapping to prevent NaN/infinite values
        const float minFreq = 20.0f;
        const float maxFreq = nyquist;

        // Use standard log mapping with proper bounds checking
        const float logMin = std::log10(minFreq);
        const float logMax = std::log10(maxFreq);
        const float logF = std::log10(f);

        // Safe normalization with bounds checking
        float normalizedLog = (logF - logMin) / (logMax - logMin);
        normalizedLog = juce::jlimit(0.0f, 1.0f, normalizedLog);

        return plot.getX() + normalizedLog * plot.getWidth();
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

    // Frequency labels selezionate: 20, 100, 500, 1K, 5K, 10K, 20K
    const float labelFreqs[] = {20.f, 100.f, 500.f, 1000.f, 5000.f, 10000.f, 20000.f};
    const int numLabelFreqs = (int)(sizeof(labelFreqs) / sizeof(labelFreqs[0]));

    for (int i = 0; i < numLabelFreqs; ++i)
    {
        if (labelFreqs[i] <= nyquist)
        {
            const float x = freqToX(labelFreqs[i]);
            juce::String label;

            // Formattazione: 1000Hz = "1K", 5000Hz = "5K", 10000Hz = "10K"
            if (labelFreqs[i] >= 1000.f && (int)labelFreqs[i] % 1000 == 0)
                label = juce::String((int)(labelFreqs[i] / 1000.f)) + "K";
            else
                label = juce::String((int)labelFreqs[i]);

            const int rx = juce::roundToInt(x - 20.0f);
            const int ry = juce::roundToInt(plot.getBottom() + 3.0f);

            // Rendering diretto senza controllo sovrapposizione (abbiamo solo 7 etichette ben distanziate)
            g.drawFittedText(label,
                             juce::Rectangle<int>(rx, ry, 40, 12),
                             juce::Justification::centred, 1);
        }
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

        // Aggiorna i target values ma non repaint subito
        // L'interpolazione farà il resto
        for (int i = 0; i < scopeSize; ++i)
        {
            targetScopeData[i] = smoothedScopeData[i];
        }
    }

    // Sistema di interpolazione temporale continua per fluidità perfetta
    bool hasChanges = false;

    for (int i = 0; i < scopeSize; ++i)
    {
        // Interpolazione fluida verso il target
        float oldValue = currentScopeData[i];
        currentScopeData[i] += (targetScopeData[i] - currentScopeData[i]) * interpolationSpeed;

        // Decay continuo del target per animazione naturale
        targetScopeData[i] *= 0.995f; // Decay molto graduale

        // Verifica se c'è un cambiamento visibile
        if (std::abs(oldValue - currentScopeData[i]) > 0.002f)
            hasChanges = true;

        // Update peak hold con interpolazione
        if (peakHoldTimer[i] > 0)
        {
            peakHoldTimer[i]--;
        }
        else
        {
            // Peak hold decay ancora più graduale
            peakHoldData[i] *= 0.992f; // Decay ultra-graduale per picchi
        }

        // Aggiorna scopeData con i valori interpolati
        scopeData[i] = currentScopeData[i];
    }

    // Repaint continuo per animazione fluida
    if (hasChanges)
        repaint();
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
    // Improved normalization factor accounting for window function and FFT size
    const float scale = 2.0f / (float)fftSize;
    const float windowCorrection = 1.5f; // Compensation for Hann window energy loss
    const float nyquist = (float)sampleRate * 0.5f;

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
        float mag1 = fftData[fftIndex] * scale * windowCorrection;
        float mag2 = (fftIndex + 1 < fftSize / 2) ? fftData[fftIndex + 1] * scale * windowCorrection : mag1;
        float mag = mag1 + frac * (mag2 - mag1);

        float dB = juce::Decibels::gainToDecibels(juce::jmax(mag, 1.0e-9f));
        float level = juce::jlimit(0.0f, 1.0f, juce::jmap(dB, mindB, maxdB, 0.0f, 1.0f));

        // Apply temporal smoothing molto forte per evitare scatti
        smoothedScopeData[i] = smoothedScopeData[i] * smoothingFactor + level * (1.0f - smoothingFactor);

        // Peak hold logic like professional spectrum analyzers
        if (level > peakHoldData[i])
        {
            peakHoldData[i] = level;         // New peak
            peakHoldTimer[i] = peakHoldTime; // Reset timer
        }

        // Non aggiornare direttamente scopeData - sarà fatto dall'interpolazione
        // scopeData[i] = smoothedScopeData[i]; // Commentato per interpolazione fluida
    }
}

void SpectrumAnalyzer::drawFrame(juce::Graphics &g)
{
    // Safety check for valid sample rate
    if (sampleRate <= 0.0)
        return;

    auto bounds = getLocalBounds().toFloat().reduced(15.0f, 18.0f); // Consistente con paint()
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
        // Calculate logarithmic frequency for this scope bin
        float logPos = (float)i / (float)(scopeSize - 1); // 0 to 1
        float freq = 20.0f * std::pow(nyquist / 20.0f, logPos);

        // Apply the same safe logarithmic mapping as in paint() method for consistency
        freq = juce::jlimit(20.0f, nyquist, freq);
        
        const float maxFreq = nyquist;

        // Use standard log mapping with proper bounds checking
        const float logMin = std::log10(minFreq);
        const float logMax = std::log10(maxFreq);
        const float logF = std::log10(freq);

        // Safe normalization with bounds checking
        float normalizedLog = (logF - logMin) / (logMax - logMin);
        normalizedLog = juce::jlimit(0.0f, 1.0f, normalizedLog);

        auto x = bounds.getX() + normalizedLog * width;
        auto y = bounds.getBottom() - juce::jmap(scopeData[i], 0.0f, 1.0f, 0.0f, height);

        // Additional safety check for valid coordinates
        if (std::isfinite(x) && std::isfinite(y))
        {
            cachedPoints.add({x, y});
        }
    }
    juce::Path spectrumPath;

    // Create ultra-smooth path usando cubic curves per fluidità massima
    if (cachedPoints.size() > 3)
    {
        spectrumPath.preallocateSpace(cachedPoints.size() * 4); // Pre-allocate per cubic curves
        spectrumPath.startNewSubPath(cachedPoints[0]);

        // Use cubic curves per smoothness massima
        for (int i = 1; i < cachedPoints.size() - 2; ++i)
        {
            auto p0 = cachedPoints[i - 1];
            auto p1 = cachedPoints[i];
            auto p2 = cachedPoints[i + 1];
            auto p3 = cachedPoints[i + 2];

            // Catmull-Rom spline per smoothness naturale
            auto cp1x = p1.x + (p2.x - p0.x) * 0.16f;
            auto cp1y = p1.y + (p2.y - p0.y) * 0.16f;
            auto cp2x = p2.x - (p3.x - p1.x) * 0.16f;
            auto cp2y = p2.y - (p3.y - p1.y) * 0.16f;

            spectrumPath.cubicTo(cp1x, cp1y, cp2x, cp2y, p2.x, p2.y);
        }

        if (cachedPoints.size() > 1)
            spectrumPath.lineTo(cachedPoints.getLast());
    }
    else if (cachedPoints.size() > 1)
    {
        // Fallback per pochi punti
        spectrumPath.startNewSubPath(cachedPoints[0]);
        for (int i = 1; i < cachedPoints.size(); ++i)
        {
            spectrumPath.lineTo(cachedPoints[i]);
        }
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
