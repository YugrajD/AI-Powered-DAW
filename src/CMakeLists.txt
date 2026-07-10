juce_add_gui_app(AIPoweredDAW
    PRODUCT_NAME "AI Powered DAW"
    COMPANY_NAME "AI Powered DAW"
    BUNDLE_ID "com.aipowereddaw.app"
    VERSION "${PROJECT_VERSION}")

target_sources(AIPoweredDAW
    PRIVATE
        Main.cpp)

target_compile_definitions(AIPoweredDAW
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(AIPoweredDAW
    PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
