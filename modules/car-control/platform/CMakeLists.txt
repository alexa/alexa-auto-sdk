# AACE CarControl Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CarControl/CarControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CarControl/CarControlAssets.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CarControl/CarControlConfiguration.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECarControlPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl.cpp
)

target_include_directories(AACECarControlPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACECarControlPlatform
    AACECorePlatform
)

install(
    TARGETS AACECarControlPlatform
    DESTINATION lib
    EXPORT AACECarControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
