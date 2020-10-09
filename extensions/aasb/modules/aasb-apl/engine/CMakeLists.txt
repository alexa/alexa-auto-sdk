# AASB APL Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/APL/AASBAPL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/APL/AASBAPLEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_APL_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ActivityEvent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ClearAllExecuteCommandsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ClearCardMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ClearDocumentMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ExecuteCommandsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ExecuteCommandsResultMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/GetVisualContextMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/GetVisualContextMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/InterruptCommandSequenceMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/ProcessActivityEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/RenderDocumentMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/RenderDocumentResultMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/SendUserEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/SetAPLMaxVersionMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Apl/APL/SetDocumentIdleTimeoutMessage.h
)

source_group("APL Message Headers" FILES ${AASB_APL_MESSAGES})

add_library(AASBAPLEngine SHARED
    ${HEADERS}
    ${AASB_APL_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAPL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAPLEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBAPLEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBAPLEngine
    PUBLIC
        AACEAASBEngine
        AACEAPLPlatform
)

install(
    TARGETS AASBAPLEngine
    DESTINATION lib
    EXPORT AASBAPL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
