# AACE PhoneCallController Engine

# Find AVSCommon headers & library
# Reason: PhoneCallController engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/PhoneCallController/PhoneCallControllerCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/PhoneCallController/PhoneCallControllerRESTAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/PhoneCallController/PhoneCallControllerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/PhoneCallController/PhoneCallControllerEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/PhoneCallController/PhoneCallControllerInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEPhoneControlEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneCallControllerCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneCallControllerRESTAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneCallControllerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneCallControllerEngineService.cpp
)

target_include_directories(AACEPhoneControlEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
)

target_link_libraries(AACEPhoneControlEngine
    PUBLIC
        AACEPhoneControlPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
)

install(
    TARGETS AACEPhoneControlEngine
    DESTINATION lib
    EXPORT AACEPhoneControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
