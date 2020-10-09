# AASB Messaging Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Messaging/AASBMessaging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Messaging/AASBMessagingEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_MESSAGING_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/ConnectionState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/ConversationsReportMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/ErrorCode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/PermissionState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/SendMessageFailedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/SendMessageMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/SendMessageSucceededMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/UpdateMessagesStatusFailedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/UpdateMessagesStatusMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/UpdateMessagesStatusSucceededMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/UpdateMessagingEndpointStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Messaging/Messaging/UploadConversationsMessage.h
)

source_group("Messaging Message Headers" FILES ${AASB_MESSAGING_MESSAGES})

add_library(AASBMessagingEngine SHARED
    ${HEADERS}
    ${AASB_MESSAGING_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBMessaging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBMessagingEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBMessagingEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBMessagingEngine
    PUBLIC
        AACEAASBEngine
        AACEMessagingPlatform
)

install(
    TARGETS AASBMessagingEngine
    DESTINATION lib
    EXPORT AASBMessaging
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
