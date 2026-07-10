juce_add_gui_app(AIPoweredDAW
    PRODUCT_NAME "AI Powered DAW"
    COMPANY_NAME "AI Powered DAW"
    BUNDLE_ID "com.aipowereddaw.app"
    VERSION "${PROJECT_VERSION}")

target_sources(AIPoweredDAW
    PRIVATE
        audio/AudioEngine.cpp
        audio/AudioEngine.h
        audio/TrackProcessingGraph.cpp
        audio/TrackProcessingGraph.h
        core/DiagnosticLog.cpp
        core/DiagnosticLog.h
        core/Project.cpp
        core/Project.h
        core/ProjectSerializer.cpp
        core/ProjectSerializer.h
        Main.cpp
        ui/ArrangementView.cpp
        ui/ArrangementView.h
        ui/InspectorPanel.cpp
        ui/InspectorPanel.h
        ui/MainComponent.cpp
        ui/MainComponent.h
        ui/PianoRollView.cpp
        ui/PianoRollView.h
        ui/TransportBar.cpp
        ui/TransportBar.h)

target_compile_definitions(AIPoweredDAW
    PRIVATE
        AIDAW_VERSION_STRING="${PROJECT_VERSION}"
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
