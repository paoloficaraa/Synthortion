#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/SynthortionLookAndFeel.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    startTimerHz(30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::paint(juce::Graphics &g)
{
    auto outer = getLocalBounds().toFloat().reduced(2.0f);

    // Background panel
    juce::ColourGradient backgroundGrad(juce::Colour(0xff1D1D1D), outer.getTopLeft(),
                                        juce::Colour(0xff111111), outer.getBottomRight(), false);
    g.setGradientFill(backgroundGrad);
    g.fillRoundedRectangle(outer, 8.0f);

    // Plot area with some padding for axis labels
    auto plot = outer.reduced(30.0f, 18.0f);

    // Border
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(outer, 8.0f, 1.0f);

    // Grid and axes: Frequency (X) and Gain (Y)
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.5f));

    // Log-spaced frequency grid lines (limit to Nyquist)
    const float nyquist = (float)(sampleRate * 0.5);
    const float freqs[] = {20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f};
    const int numFreqs = (int)(sizeof(freqs) / sizeof(freqs[0]));
    auto freqToX = [&](float f)
    {
        f = juce::jlimit(20.0f, nyquist, f);
        float norm = (std::log10(f) - std::log10(20.f)) / (std::log10(nyquist) - std::log10(20.f));
        return plot.getX() + norm * plot.getWidth();
    };

    for (int i = 0; i < numFreqs; ++i)
    {
        const float x = freqToX(freqs[i]);
        g.drawLine(x, plot.getY(), x, plot.getBottom(), 1.0f);
    }

    // Gain grid lines and labels (-60 to 0 dB)
    auto gainMin = -60.f;
    auto gainMax = 0.f;
    auto gainToY = [&](float dB)
    {
        float norm = juce::jmap(dB, gainMin, gainMax, 0.0f, 1.0f);
        return plot.getBottom() - norm * plot.getHeight();
    };

    for (int dB = (int)gainMin; dB <= (int)gainMax; dB += 10)
    {
        const float y = gainToY((float)dB);
        g.drawLine(plot.getX(), y, plot.getRight(), y, 1.0f);
    }

    // Axis labels
    g.setColour(juce::Colour(0xffB4B4B4).withAlpha(0.7f));
    {
        juce::Font f;
        f.setHeight(10.0f);
        f.setBold(false);
        g.setFont(f);
    }

    // Frequency labels on X axis
    for (int i = 0; i < numFreqs; ++i)
    {
        const float x = freqToX(freqs[i]);
        juce::String label = (freqs[i] >= 1000.f ? juce::String(freqs[i] / 1000.f, 1) + " kHz" : juce::String((int)freqs[i]) + " Hz");
        const int rx = juce::roundToInt(x - 25.0f);
        const int ry = juce::roundToInt(plot.getBottom() + 2.0f);
        g.drawFittedText(label,
                         juce::Rectangle<int>(rx, ry, 50, 14),
                         juce::Justification::centred, 1);
    }

    // Gain labels on Y axis
    for (int dB = (int)gainMin; dB <= (int)gainMax; dB += 10)
    {
        const float y = gainToY((float)dB);
        const int rx = juce::roundToInt(plot.getX() - 42.0f);
        const int ry = juce::roundToInt(y - 7.0f);
        g.drawFittedText(juce::String(dB) + " dB",
                         juce::Rectangle<int>(rx, ry, 40, 14),
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

    for (int i = 0; i < scopeSize; ++i)
    {
        // Map scope index to FFT bin index linearly
        const float proportion = (float)i / (float)scopeSize;
        const int fftIndex = juce::jlimit(0, fftSize / 2, (int)(proportion * (float)fftSize * 0.5f));

        float mag = fftData[fftIndex] * scale;
        float dB = juce::Decibels::gainToDecibels(juce::jmax(mag, 1.0e-9f));
        float level = juce::jlimit(0.0f, 1.0f, juce::jmap(dB, mindB, maxdB, 0.0f, 1.0f));
        scopeData[i] = level;
    }
}

void SpectrumAnalyzer::drawFrame(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(30.0f, 18.0f);
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    const float nyquist = (float)sampleRate * 0.5f;

    // Function to map a normalized linear position (0-1) to a log-scaled X coordinate
    auto mapX = [&](float linearPos)
    {
        float freq = linearPos * nyquist;
        freq = juce::jmax(20.0f, freq); // Clamp to min frequency
        float logPos = (std::log10(freq) - std::log10(20.0f)) / (std::log10(nyquist) - std::log10(20.0f));
        return bounds.getX() + logPos * width;
    };

    juce::Path spectrumPath;
    bool started = false;

    for (int i = 1; i < scopeSize; ++i)
    {
        const float linearProportion = (float)i / (float)scopeSize;
        auto x = mapX(linearProportion);
        auto y = bounds.getBottom() - juce::jmap(scopeData[i], 0.0f, 1.0f, 0.0f, height);

        if (!started)
        {
            spectrumPath.startNewSubPath(x, y);
            started = true;
        }
        else
        {
            spectrumPath.lineTo(x, y);
        }
    }

    // Fill under the spectrum with gradient
    juce::Path filledPath = spectrumPath;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    juce::ColourGradient spectrumGradient(
        PURPLE.withAlpha(0.35f),
        bounds.getCentreX(), bounds.getY(),
        PURPLE_DARK.withAlpha(0.12f),
        bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(spectrumGradient);
    g.fillPath(filledPath);

    // Draw spectrum line
    g.setColour(PURPLE);
    g.strokePath(spectrumPath, juce::PathStrokeType(2.0f));
}
