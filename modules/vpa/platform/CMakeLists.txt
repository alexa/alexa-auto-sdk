# AACE VPA Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/VPA/DirectiveHandler.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEVPAPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DirectiveHandler.cpp
)

target_include_directories(AACEVPAPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEVPAPlatform
    AACECorePlatform
)

install(
    TARGETS AACEVPAPlatform
    DESTINATION lib
    EXPORT AACEVPA
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)