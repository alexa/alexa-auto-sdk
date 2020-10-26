# AACE Navigation Engine

# Find AVSCommon headers & library
# Reason: Navigation engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/DisplayManagerCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationAssistanceCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Navigation/NavigationHandlerInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACENavigationEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DisplayManagerCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationAssistanceCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NavigationEngineService.cpp
)

target_include_directories(AACENavigationEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
)

target_link_libraries(AACENavigationEngine
    PUBLIC
        AACENavigationPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
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
