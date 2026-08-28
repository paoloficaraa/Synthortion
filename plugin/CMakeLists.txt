juce_add_plugin(Synthortion
    COMPANY_NAME "WavesCrate"
    BUNDLE_ID com.wavescrate.synthortion
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    NEEDS_WEBVIEW2 TRUE
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
    src/PingPongDelay.cpp
    src/BitCrusher.cpp
    src/SynthortionChorus.cpp
    src/SpectrumAnalyzer.cpp
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
    JUCE_USE_WIN_WEBVIEW2=1
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
    pkg_check_modules(GTK3 REQUIRED gtk+-x11-3.0)
    pkg_check_modules(CURL REQUIRED libcurl)

    target_include_directories(Synthortion PRIVATE ${WEBKIT2GTK_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
    target_link_libraries(Synthortion PRIVATE ${WEBKIT2GTK_LIBRARIES} ${GTK3_LIBRARIES} ${CURL_LIBRARIES})
endif()
add_custom_command(TARGET Synthortion_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        "${CMAKE_BINARY_DIR}/plugin/Synthortion_artefacts/$<CONFIG>/VST3/Synthortion.vst3/Contents/Resources/dist"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../ui/dist"
        "${CMAKE_BINARY_DIR}/plugin/Synthortion_artefacts/$<CONFIG>/VST3/Synthortion.vst3/Contents/Resources/dist"
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        "C:/Program Files/Common Files/VST3/Synthortion.vst3/Contents/Resources/dist"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../ui/dist"
        "C:/Program Files/Common Files/VST3/Synthortion.vst3/Contents/Resources/dist"
    COMMENT "Copying UI dist to VST3 bundle resources (clean sync)"
)

juce_add_console_app(SynthortionTests
    PRODUCT_NAME "Synthortion Tests"
    NEEDS_WEBVIEW2 TRUE
)

juce_generate_juce_header(SynthortionTests)

target_sources(SynthortionTests
PRIVATE
    tests/Main.cpp
    tests/AudioPluginTest.h
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/WarmDistortion.cpp
    src/PingPongDelay.cpp
    src/BitCrusher.cpp
    src/SynthortionChorus.cpp
    src/SpectrumAnalyzer.cpp
)

target_include_directories(SynthortionTests
PRIVATE
    include
)

target_compile_definitions(SynthortionTests
PRIVATE
    JUCE_USE_WIN_WEBVIEW2=1
    JUCE_USE_CURL=0
    JUCE_UNIT_TESTS=1
    JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(SynthortionTests
PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

add_test(NAME SynthortionTests COMMAND SynthortionTests)
