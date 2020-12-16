cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Detect the version number
execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/builder/scripts/gen-version.sh" -b
    OUTPUT_VARIABLE SDK_BASE_VERSION
)

project(AAC VERSION ${SDK_BASE_VERSION})

message(STATUS "Alexa Auto SDK - ${SDK_VERSION}")

option(AAC_ENABLE_TESTS "Enable building test packages for AAC modules" ON)
if(AAC_ENABLE_TESTS)
    enable_testing()
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling coverage for all modules.")
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

if(MODULES)
    set(CORE_MODULES ${MODULES})
    string(REPLACE "," " " CORE_MODULES ${CORE_MODULES})
    separate_arguments(CORE_MODULES)
    foreach(MODULE ${CORE_MODULES})
        add_subdirectory(modules/${MODULE})
    endforeach()
else()
    add_subdirectory(modules/address-book)
    add_subdirectory(modules/alexa)
    add_subdirectory(modules/apl)
    add_subdirectory(modules/car-control)
    add_subdirectory(modules/cbl)
    add_subdirectory(modules/connectivity)
    add_subdirectory(modules/core)
    add_subdirectory(modules/messaging)
    add_subdirectory(modules/navigation)
    add_subdirectory(modules/phone-control)
    add_subdirectory(modules/text-to-speech)
    add_subdirectory(modules/text-to-speech-provider)
endif()

if(EXTRA_MODULES)
   string(REPLACE "," " " EXTRA_MODULES ${EXTRA_MODULES})
   separate_arguments(EXTRA_MODULES)
   foreach(EXTRA_MODULE ${EXTRA_MODULES})
       add_subdirectory(${EXTRA_MODULE})
   endforeach()
endif()
