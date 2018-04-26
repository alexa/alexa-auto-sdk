# AACE Navigation

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACENavigation VERSION 0.99.0 LANGUAGES CXX)

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

# Navigation module depends on Alexa module
if(NOT TARGET AACEAlexaPlatform)
    if(NOT AAC_HOME)
        message(FATAL_ERROR "AAC_HOME is not set!")
    endif()
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
endif()

add_subdirectory(platform)
add_subdirectory(engine)

install(
    EXPORT AACENavigation
    DESTINATION share/cmake
)
