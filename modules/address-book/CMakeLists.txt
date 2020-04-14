# AACE AddressBook

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACEAddressBook LANGUAGES CXX)

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
    message(STATUS "Enabling Address Sanitizer for AACEAddressBook")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling coverage for AACEAddressBook.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

if (AAC_EMIT_SENSITIVE_LOGS)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
    if (BUILD_TYPE_UPPER STREQUAL DEBUG)
        message("WARNING: Logging of sensitive information enabled!")
        add_definitions(-DAAC_EMIT_SENSITIVE_LOGS)
    else()
        message(FATAL_ERROR "FATAL_ERROR: AAC_EMIT_SENSITIVE_LOGS=ON in non-DEBUG build.")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
endif()

add_subdirectory(platform)
add_subdirectory(engine)

install(
    EXPORT AACEAddressBook
    DESTINATION share/cmake
)
