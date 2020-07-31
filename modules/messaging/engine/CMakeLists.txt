# AACE Messaging Engine

# Find AVSCommon headers & library
# Reason: Messaging engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)
find_library(AVS_MESSAGING_LIBRARY Messaging)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Messaging/MessagingEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Messaging/MessagingEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEMessagingEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MessagingEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MessagingEngineImpl.cpp
)

target_include_directories(AACEMessagingEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AACEMessagingEngine
    PUBLIC
        AACEMessagingPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
        ${AVS_MESSAGING_LIBRARY}
)

install(
    TARGETS AACEMessagingEngine
    DESTINATION lib
    EXPORT AACEMessaging
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
