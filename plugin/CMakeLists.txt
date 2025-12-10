juce_add_plugin(Synthortion
    COMPANY_NAME "WavesCrate"
    BUNDLE_ID com.wavescrate.synthortion
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE MACP
    PLUGIN_CODE P729
    FORMATS AU VST3
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(Synthortion 
PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/WarmDistortion.cpp
    src/SynthortionLookAndFeel.cpp
    src/SpectrumAnalyzer.cpp
    src/ParametricEQ.cpp
    src/BitCrusher.cpp
    src/PingPongDelay.cpp
    src/SynthortionChorus.cpp
)

target_include_directories(Synthortion
PRIVATE
    include
)

target_link_libraries(Synthortion
PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(Synthortion
PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)