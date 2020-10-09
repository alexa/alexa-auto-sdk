# AACE Navigation Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Navigation/Navigation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Navigation/NavigationConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Navigation/NavigationEngineInterfaces.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACENavigationPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Navigation.cpp
)

target_include_directories(AACENavigationPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACENavigationPlatform
    AACECorePlatform
)

install(
    TARGETS AACENavigationPlatform
    DESTINATION lib
    EXPORT AACENavigation
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)