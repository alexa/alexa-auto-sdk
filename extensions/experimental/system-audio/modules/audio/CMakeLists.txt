# AACE SystemAudio

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACESystemAudio LANGUAGES CXX)

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

add_definitions(-DTHROTTLE_AUDIO)

if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|Linux)")
    set(ENABLE_GSTREAMER ON)
    set(ENABLE_OMXAL OFF)
    set(ENABLE_QSA OFF)
    set(AUDIO_CONFIG_FILE "config.json.linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "QNX")
    set(ENABLE_GSTREAMER OFF)
    set(ENABLE_OMXAL ON)
    set(ENABLE_QSA ON)
    set(AUDIO_CONFIG_FILE "config.json.qnx")
else()
    message(FATAL_ERROR "Unsupported system")
endif()

# Include AAL
add_subdirectory(lib/aal)

# Engine depends on Alexa module
if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
endif()

#add_subdirectory(platform)
add_subdirectory(engine)

install(
    EXPORT AACESystemAudio
    DESTINATION share/cmake
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/${AUDIO_CONFIG_FILE}
    RENAME "config-system-audio.json"
    DESTINATION etc
)