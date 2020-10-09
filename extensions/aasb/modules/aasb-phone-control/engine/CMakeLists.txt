# AASB PhoneControl Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/PhoneCallController/AASBPhoneCallController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/PhoneCallController/AASBPhoneCallControllerEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_PHONECONTROL_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/AnswerMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallerIdReceivedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallError.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallFailedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallingDeviceConfigurationProperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CallStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/ConnectionState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/ConnectionStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CreateCallIdMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/CreateCallIdMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/DeviceConfigurationUpdatedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/DialMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/DTMFError.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/RedialMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/SendDTMFFailedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/SendDTMFMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/SendDTMFSucceededMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PhoneCallController/PhoneCallController/StopMessage.h
   
)

source_group("PhoneControl Message Headers" FILES ${AASB_PHONECONTROL_MESSAGES})

add_library(AASBPhoneControlEngine SHARED
    ${HEADERS}
    ${AASB_PHONECONTROL_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBPhoneCallController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBPhoneCallControllerEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBPhoneControlEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBPhoneControlEngine
    PUBLIC
        AACEAASBEngine
        AACEPhoneControlPlatform
)

install(
    TARGETS AASBPhoneControlEngine
    DESTINATION lib
    EXPORT AASBPhoneControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
