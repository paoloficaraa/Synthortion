#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"

namespace synthortion
{
    static juce::WebBrowserComponent::Options createBrowserOptions (AudioPluginAudioProcessor& processor, AudioPluginAudioProcessorEditor& editor)
    {
        return juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled (true)
            .withEventListener ("setParameter", [&processor] (const juce::var& data)
            {
                if (auto* obj = data.getDynamicObject())
                {
                    auto parameterId = obj->getProperty ("parameterId").toString();
                    auto value = static_cast<float> (obj->getProperty ("value"));
                    if (auto* param = processor.getAPVTS().getParameter (parameterId))
                    {
                        param->setValueNotifyingHost (value);
                    }
                }
            })
            .withEventListener ("connect", [&editor] (const juce::var&)
            {
                editor.parameterChanged ("", 0.0f);
            });
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p),
          processorRef (p)
    {
        webView = std::make_unique<juce::WebBrowserComponent> (createBrowserOptions (processorRef, *this));

#if defined(SYNTHORTION_DEV_SERVER_URL)
        webView->goToURL (SYNTHORTION_DEV_SERVER_URL);
#else
        webView->goToURL ("http://localhost:5173");
#endif

        addAndMakeVisible (webView.get());

        for (auto* param : processorRef.getAPVTS().processor.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                processorRef.getAPVTS().addParameterListener (pWithId->paramID, this);
            }
        }

        setOpaque (true);
        setResizable (false, false);
        setSize (kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        for (auto* param : processorRef.getAPVTS().processor.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                processorRef.getAPVTS().removeParameterListener (pWithId->paramID, this);
            }
        }
    }

    void AudioPluginAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
    {
        juce::ignoreUnused (parameterID, newValue);
        if (webView == nullptr) return;

        juce::MessageManager::callAsync ([this]()
        {
            if (webView == nullptr) return;

            for (auto* param : processorRef.getAPVTS().processor.getParameters())
            {
                if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
                {
                    const float val = pWithId->getValue();
                    const auto id = pWithId->paramID;

                    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
                    obj->setProperty ("parameterId", id);
                    obj->setProperty ("value", val);

                    webView->emitEventIfBrowserIsVisible ("parameterChange", juce::var (obj.get()));
                    webView->evaluateJavascript (
                        "if (window.__SYNTORTION_BRIDGE__ && window.__SYNTORTION_BRIDGE__.onParameterChange) { "
                        "window.__SYNTORTION_BRIDGE__.onParameterChange('" + id + "', " + juce::String (val) + "); }"
                    );
                }
            }
        });
    }

    void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::black);
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        if (webView != nullptr)
            webView->setBounds (getLocalBounds());
    }
}
