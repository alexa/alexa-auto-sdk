# AACE CBL Engine

# Find AVSCommon headers & library
# Reason: CBL engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_REGISTRATION_MANAGER_LIBRARY RegistrationManager)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CBL/CBLConfigurationInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CBL/CBLConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CBL/CBLAuthorizationProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CBL/CBLEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CBL/CBLEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECBLEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBLConfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBLAuthorizationProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBLConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBLEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBLEngineService.cpp
)

target_include_directories(AACECBLEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
)

target_link_libraries(AACECBLEngine
    PUBLIC
        AACECBLPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_REGISTRATION_MANAGER_LIBRARY}
)

install(
    TARGETS AACECBLEngine
    DESTINATION lib
    EXPORT AACECBL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()