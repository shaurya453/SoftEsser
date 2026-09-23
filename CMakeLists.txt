cmake_minimum_required(VERSION 3.23.1)

project(SoftEsser VERSION 0.0.1)

# General setup
set(PROJECT_NAME "SoftEsser")
set(CURRENT_VERSION "0.0.1")
set(PRODUCT_NAME "SoftEsser")
set(COMPANY_NAME "TheMeloMix")
set(BUNDLE_ID "com.themelomix.SoftEsser")
set(FORMATS VST3)

# JUCE
add_subdirectory(modules/JUCE)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows runtime library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif()

# Define the plugin target
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE ADSP
    PLUGIN_CODE SmGP
    FORMATS ${FORMATS}
    PRODUCT_NAME "${PRODUCT_NAME}"
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
)

# Source files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)

# Add sources directly to plugin target
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# Optional assets
file(GLOB_RECURSE Assets CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/assets/*.*"
)

# Only create binary data if assets exist
if(Assets)
    juce_add_binary_data(AudioPluginData
        SOURCES ${Assets}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE AudioPluginData)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_BINARY_DIR}/AudioPluginData"
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

if(TARGET juce_vst3_helper)
    target_include_directories(juce_vst3_helper PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/JUCE/modules/juce_audio_processors/format_types/VST3_SDK
        ${CMAKE_SOURCE_DIR}/modules/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK
    )
    
    target_compile_definitions(juce_vst3_helper PRIVATE
        JucePlugin_Name="${PRODUCT_NAME}"
        JucePlugin_Desc="${PRODUCT_NAME}"
        JucePlugin_Manufacturer="${COMPANY_NAME}"
        JucePlugin_ManufacturerWebsite=""
        JucePlugin_ManufacturerEmail=""
        JucePlugin_ManufacturerCode=0x41445350
        JucePlugin_PluginCode=0x536d4750
        JucePlugin_VSTUniqueID=0x536d4750
        JucePlugin_VersionString="${CURRENT_VERSION}"
        JucePlugin_VersionCode=0x1
        JucePlugin_VSTCategory=kPlugCategEffect
        "JucePlugin_Vst3Category=\"Fx\""
    )
endif()

if(TARGET SoftEsser_VST3)
    target_include_directories(SoftEsser_VST3 PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK
    )
endif()

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_ENABLE_REPAINT_DEBUGGING=0
)

# Link JUCE modules
target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_product_unlocking
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)