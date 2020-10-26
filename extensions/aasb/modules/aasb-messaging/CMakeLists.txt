# AASB Messaging

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AASBMessaging LANGUAGES CXX)

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

if(AAC_ENABLE_ADDRESS_SANITIZER)
    message(STATUS "Enabling Address Sanitizer for AASBMessaging")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling coverage for AASBMessaging.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

# AASBMessaging module depends on AACEAASB and AACEMessaging module
if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACEAASB.cmake)
    include(${AAC_HOME}/share/cmake/AACEMessaging.cmake)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
endif()

add_subdirectory(engine)

install(
    EXPORT AASBMessaging
    DESTINATION share/cmake
)
