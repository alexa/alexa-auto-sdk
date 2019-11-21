# AACE AmazonLite Platform

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAmazonLitePlatform INTERFACE)

target_include_directories(AACEAmazonLitePlatform
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS AACEAmazonLitePlatform
    EXPORT AACEAmazonLite
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)