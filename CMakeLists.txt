cmake_minimum_required(VERSION 3.22)
project(RepartoVocalSuite VERSION 1.1.0)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(RV_FORMATS VST3 AU Standalone)
    set(RV_LITE_FORMATS VST3 AU)
else()
    set(RV_FORMATS VST3 Standalone)
    set(RV_LITE_FORMATS VST3)
endif()

juce_add_plugin(RepartoVocalSuite
    COMPANY_NAME "Ireko"
    PLUGIN_MANUFACTURER_CODE Rpau
    PLUGIN_CODE Rvs1
    FORMATS ${RV_FORMATS}
    AU_MAIN_TYPE kAudioUnitType_Effect
    PRODUCT_NAME "Reparto Chain by Ireko"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(RepartoVocalSuite PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(RepartoVocalSuite PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RepartoVocalSuite PRIVATE
    juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ---- Version LITE: interfaz estatica con 2 perillas por modulo y motor ligero ----
juce_add_plugin(RepartoChainLite
    COMPANY_NAME "Ireko"
    PLUGIN_MANUFACTURER_CODE Rpau
    PLUGIN_CODE Rvl1
    FORMATS ${RV_LITE_FORMATS}
    AU_MAIN_TYPE kAudioUnitType_Effect
    PRODUCT_NAME "Reparto Chain Lite by Ireko"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(RepartoChainLite PRIVATE Source/PluginProcessor.cpp Source/LiteEditor.cpp)
target_compile_definitions(RepartoChainLite PUBLIC RV_LITE=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RepartoChainLite PRIVATE
    juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
