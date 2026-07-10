add_executable(AIPoweredDAWTests
    ModelTests.cpp
    ../src/core/Project.cpp
    ../src/core/ProjectSerializer.cpp)

target_include_directories(AIPoweredDAWTests
    PRIVATE
        ../src)

target_link_libraries(AIPoweredDAWTests
    PRIVATE
        juce::juce_core
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

add_test(NAME AIPoweredDAWTests COMMAND AIPoweredDAWTests)
