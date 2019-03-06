# AACE CBL Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CBL/CBL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CBL/CBLConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CBL/CBLEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECBLPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBL.cpp
)

target_include_directories(AACECBLPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACECBLPlatform
    AACECorePlatform
)

install(
    TARGETS AACECBLPlatform
    DESTINATION lib
    EXPORT AACECBL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)