#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    //==============================================================================
    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p),
          webView{juce::WebBrowserComponent::Options{}
                      .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                      .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
                                                  .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))
                                                  .withBackgroundColour(juce::Colours::transparentBlack))}
    {
        juce::ignoreUnused(processorRef);

        addAndMakeVisible(webView);

#if JUCE_DEBUG
        webView.goToURL("http://localhost:5173");
#else
        // In a production build, you would likely serve the UI from bundled resources.
        // For now, this is left as an exercise for the developer.
        // A simple approach would be to go to a blank page.
        webView.goToURL("about:blank");
#endif

        setResizable(false, false);
        setSize(1024, 650);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
        webView.setBounds(getLocalBounds());
    }
}
