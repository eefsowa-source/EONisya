cmake_minimum_required(VERSION 3.22)

# ── Project ─────────────────────────────────────────────────────────────
project(EONisya VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ── JUCE ────────────────────────────────────────────────────────────────
# Prefer a local JUCE checkout; fall back to fetching 8.0.14 from GitHub.
if(DEFINED ENV{JUCE_DIR})
    set(JUCE_ROOT "$ENV{JUCE_DIR}")
elseif(EXISTS "/Users/sungha/JUCE")
    set(JUCE_ROOT "/Users/sungha/JUCE")
endif()

if(JUCE_ROOT)
    add_subdirectory("${JUCE_ROOT}" juce)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.14
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ── Plugin target ───────────────────────────────────────────────────────
juce_add_plugin(EONisya
    # ── Identification ──
    PLUGIN_MANUFACTURER_CODE  Eons
    PLUGIN_CODE               Eon6

    # ── Plugin formats (macOS & Windows) ──
    FORMATS                   VST3 AU AUv3 Standalone

    # ── Product metadata ──
    PRODUCT_NAME              "EON MOTIF6"
    DESCRIPTION               "EON MOTIF6 music production synthesizer (VSTi)"

    # ── Company info ──
    COMPANY_NAME              "EON Audio Systems"
    COMPANY_WEBSITE           "https://eonaudio.systems"
    COMPANY_EMAIL             "dev@eonaudio.systems"

    # ── Bundle identifiers ──
    BUNDLE_ID                 com.eonaudio.eonismotif6
    PLUGIN_MANUFACTURER       "EON Audio Systems"
    RESPONSE_LIMIT            1024

    # ── AU compat ──
    AU_MAIN_TYPE              kAudioUnitType_MusicDevice
    AU_SANDBOX_SAFE           YES
)

# ── Sources ──────────────────────────────────────────────────────────────
target_sources(EONisya PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(EONisya PRIVATE
    Source
)

# No released VST2 counterpart exists; silence the VST2/VST3 automation
# migration guard in juce_audio_plugin_client_VST3.cpp.
target_compile_definitions(EONisya PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ── Enable JUCE modules ──────────────────────────────────────────────
target_link_libraries(EONisya PUBLIC
    juce::juce_audio_basics
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_dsp
)

# ── OS-specific ──────────────────────────────────────────────────────────
if(APPLE)
    target_compile_options(EONisya PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-deprecated-declarations
    )
    set_target_properties(EONisya PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY          "Apple Development"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE             "Automatic"
    )
elseif(WIN32)
    target_compile_options(EONisya PRIVATE /W4)
endif()
