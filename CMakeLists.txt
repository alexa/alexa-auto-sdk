cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Detect the version number
execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/builder/scripts/gen-version.sh" -b
    OUTPUT_VARIABLE SDK_BASE_VERSION
)

project(AAC VERSION ${SDK_BASE_VERSION})

message(STATUS "Alexa Auto SDK - ${SDK_VERSION}")

enable_testing()
set(AAC_ENABLE_TESTS ON)

if(AAC_ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

if(MODULES)
    string(REPLACE "," " " MODULES ${MODULES})
    separate_arguments(MODULES)
    foreach(MODULE ${MODULES})
        add_subdirectory(modules/${MODULE})
    endforeach()
else()
    add_subdirectory(modules/alexa)
    add_subdirectory(modules/core)
    add_subdirectory(modules/navigation)
    add_subdirectory(modules/phone-control)
    add_subdirectory(modules/contact-uploader)
    add_subdirectory(modules/cbl)
endif()

if(EXTRA_MODULES)
   string(REPLACE "," " " EXTRA_MODULES ${EXTRA_MODULES})
   separate_arguments(EXTRA_MODULES)
   foreach(EXTRA_MODULE ${EXTRA_MODULES})
       add_subdirectory(${EXTRA_MODULE})
   endforeach()
endif()
