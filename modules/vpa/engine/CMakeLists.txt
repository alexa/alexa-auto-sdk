# AACE VPA Engine

# Find AVSCommon headers & library
# Reason: VPA engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/VPA/VPAEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/VPA/VPACapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/VPA/VPAObserverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/VPA/VPAEngineImpl.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEVPAEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VPAEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VPACapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VPAEngineImpl.cpp
)

target_include_directories(AACEVPAEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACEVPAEngine
    PUBLIC
        AACEVPAPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACEVPAEngine
    DESTINATION lib
    EXPORT AACEVPA
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
