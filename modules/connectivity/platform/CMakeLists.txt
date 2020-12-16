# AACE Connectivity Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Connectivity/AlexaConnectivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Connectivity/AlexaConnectivityEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEConnectivityPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaConnectivity.cpp
)

target_include_directories(AACEConnectivityPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEConnectivityPlatform
    PUBLIC
        AACEAlexaEngine
        AACEAlexaPlatform
        AACECoreEngine
        AACECorePlatform
)

install(
    TARGETS AACEConnectivityPlatform
    DESTINATION lib
    EXPORT AACEConnectivity
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
