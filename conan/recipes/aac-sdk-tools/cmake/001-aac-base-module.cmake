message("#### APPLYING AAC_BASE_MODULE CMAKE SETTINGS!")

if(AAC_ENABLE_ADDRESS_SANITIZER)
    message(STATUS "Enabling Address Sanitizer for AACECore")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling coverage for Core module.")
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

if (AAC_EMIT_LATENCY_LOGS)
    add_definitions(-DAAC_LATENCY_LOGS_ENABLED)
endif()

# NOTE: we should remove this when we get rid of the rapidjson dependencies
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)
    add_definitions(-DAACE_DEBUG_LOG_ENABLED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -Werror")
else()
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")
endif()