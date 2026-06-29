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

juce_add_binary_data(SynthortionResources
    HEADER_NAME SynthortionResources.h
    NAMESPACE SynthortionResources
    SOURCES
        resources/BebasNeue-Regular.ttf
        resources/Montserrat-Medium.ttf
        resources/Montserrat-Regular.ttf
)

target_sources(Synthortion
PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/AnalogLookAndFeel.cpp
    src/PanelComponent.cpp
    src/BypassComponent.cpp
    src/WarmDistortion.cpp
    src/PingPongDelay.cpp
    src/BitCrusher.cpp
    src/SynthortionChorus.cpp
)

target_include_directories(Synthortion
PRIVATE
    include
)

target_link_libraries(Synthortion
PRIVATE
    SynthortionResources
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
    gin
    gin_plugin      # For audio plugins
    gin_dsp         # For DSP components
    gin_gui         # For UI components
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(Synthortion
PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

option(JUCE_ENABLE_UNIT_TESTS "Enable JUCE unit tests" OFF)
if(JUCE_ENABLE_UNIT_TESTS)
    target_sources(Synthortion
    PRIVATE
        tests/AudioPluginTest.h
    )
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
    pkg_check_modules(GTK3 REQUIRED gtk+-x11-3.0)
    pkg_check_modules(CURL REQUIRED libcurl)

    target_include_directories(Synthortion PRIVATE ${WEBKIT2GTK_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
    target_link_libraries(Synthortion PRIVATE ${WEBKIT2GTK_LIBRARIES} ${GTK3_LIBRARIES} ${CURL_LIBRARIES})
endif()
