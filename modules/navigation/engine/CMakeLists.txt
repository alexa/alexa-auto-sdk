# AACE Navigation Engine

# Find AVSCommon headers & library
# Reason: Navigation engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationObserverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationEngineImpl.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACENavigationEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationEngineImpl.cpp
)

target_include_directories(AACENavigationEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACENavigationEngine
    PUBLIC
        AACENavigationPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACENavigationEngine
    DESTINATION lib
    EXPORT AACENavigation
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()