# AACE Alexa

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACEAlexa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("No build type specified, defaulting to DEBUG.")
endif()

# Select flags.
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -DAACE_DEBUG_LOG_ENABLED -Wall -g")

if (AAC_EMIT_LATENCY_LOGS)
add_definitions(-DACSDK_LATENCY_LOG_ENABLED)
add_definitions(-DACSDK_LOG_MODULE=avsCommon)
add_definitions(-DAAC_LATENCY_LOGS_ENABLED)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Alexa module is depends on Core module
if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
endif()

add_subdirectory(platform)
add_subdirectory(engine)

install(
    EXPORT AACEAlexa
    DESTINATION share/cmake
)
