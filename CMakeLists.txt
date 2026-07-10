cmake_minimum_required(VERSION 3.24)

project(AIPoweredDAW
    VERSION 0.1.0
    DESCRIPTION "A compact AI-assisted DAW built in modern C++"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AIDAW_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(JUCE)

add_subdirectory(src)

if(AIDAW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
