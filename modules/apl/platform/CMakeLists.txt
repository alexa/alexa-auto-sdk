# AACE APL Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/APL/APL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/APL/APLConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/APL/APLEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAPLPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/APL.cpp
)

target_include_directories(AACEAPLPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEAPLPlatform
    AACECorePlatform
)

install(
    TARGETS AACEAPLPlatform
    DESTINATION lib
    EXPORT AACEAPL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)