message("#### APPLYING AAC_CORE_MODULE CMAKE SETTINGS!")

if(AAC_CORE_ENGINE_DIRECTORY)
    configure_file (
        "${AAC_CORE_ENGINE_DIRECTORY}/include/AACE/Engine/Core/EngineVersion.h.in"
        "${AAC_CORE_ENGINE_DIRECTORY}/include/AACE/Engine/Core/EngineVersion.h"
        @ONLY)
endif()

# Enable/disable the default engine logger (On|Off).
#
#   -DAAC_DEFAULT_LOGGER_ENABLED=On
#   -DAAC_DEFAULT_LOGGER_ENABLED=Off
#
# Defaults to On. If enabled, there must be logger level and sink, either explicitly set or default.

if(AAC_DEFAULT_LOGGER_ENABLED)
    add_definitions(-DAAC_DEFAULT_LOGGER_ENABLED)

    # Set the logger level for the default engine logger (Verbose|Info|Metric|Warn|Error|Critical).
    #
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Verbose
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Info
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Metric
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Warn
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Error
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Critical
    #
    # Defaults to Info for release builds, and Verbose for debug builds.

    function(set_default_logger_level)
        string(TOUPPER "${CMAKE_BUILD_TYPE}" AAC_BUILD_TYPE_UPPER)
        if(AAC_BUILD_TYPE_UPPER STREQUAL "DEBUG")
            add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_VERBOSE)
        else()
            add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_INFO)
        endif()
    endfunction(set_default_logger_level)

    set(AAC_DEFAULT_LOGGER_LEVEL "Default" CACHE STRING "Set the default logger level")
    string(TOUPPER "${AAC_DEFAULT_LOGGER_LEVEL}" AAC_DEFAULT_LOGGER_LEVEL_UPPER)
    if(AAC_DEFAULT_LOGGER_LEVEL_UPPER MATCHES "^(DEFAULT)?$") # also matches an empty string
        set_default_logger_level()
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "VERBOSE")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_VERBOSE)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "INFO")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_INFO)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "METRIC")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_METRIC)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "WARN")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_WARN)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "ERROR")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_ERROR)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "CRITICAL")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_CRITICAL)
    elseif(NOT AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "NONE")
        message(FATAL_ERROR "Unknown default logger level: ${AAC_DEFAULT_LOGGER_LEVEL}")
    endif(AAC_DEFAULT_LOGGER_LEVEL_UPPER MATCHES "^(DEFAULT)?$")

    # Set the logger sink for the default engine logger (Console|Syslog).
    #
    #   -DAAC_DEFAULT_LOGGER_SINK=Console
    #   -DAAC_DEFAULT_LOGGER_SINK=Syslog
    #
    # Defaults to Syslog for Android build targets, and Console for all other build targets.

    function(set_default_logger_sink)
        string(TOUPPER "${CMAKE_SYSTEM_NAME}" AAC_SYSTEM_NAME_UPPER)
        if(AAC_SYSTEM_NAME_UPPER STREQUAL "ANDROID")
            add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_SYSLOG)
        else()
            add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_CONSOLE)
        endif()
    endfunction(set_default_logger_sink)

    set(AAC_DEFAULT_LOGGER_SINK "Default" CACHE STRING "Set the default logger sink")
    string(TOUPPER "${AAC_DEFAULT_LOGGER_SINK}" AAC_DEFAULT_LOGGER_SINK_UPPER)
    if(AAC_DEFAULT_LOGGER_SINK_UPPER MATCHES "^(DEFAULT)?$") # also matches an empty string
        set_default_logger_sink()
    elseif(AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "CONSOLE")
        add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_CONSOLE)
    elseif(AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "SYSLOG")
        add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_SYSLOG)
    elseif(NOT AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "NONE")
        message(FATAL_ERROR "Unknown default logger sink: ${AAC_DEFAULT_LOGGER_SINK}")
    endif(AAC_DEFAULT_LOGGER_SINK_UPPER MATCHES "^(DEFAULT)?$")

    # Log the logger compile definitions for testing and verification.
    # get_directory_property(LOGGER_COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
    # message(STATUS "Logger Compile Definitions: ${LOGGER_COMPILE_DEFINITIONS}")

endif(AAC_DEFAULT_LOGGER_ENABLED)

if(AAC_EMIT_COLOR_LOGS)
    add_definitions(-DAAC_EMIT_COLOR_LOGS)
endif()

if(AAC_EMIT_THREAD_MONIKER_LOGS)
    add_definitions(-DAAC_EMIT_THREAD_MONIKER_LOGS)
endif()