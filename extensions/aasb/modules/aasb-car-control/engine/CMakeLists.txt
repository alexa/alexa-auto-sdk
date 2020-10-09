# AASB Car Control Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/CarControl/AASBCarControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/CarControl/AASBCarControlEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_CARCONTROL_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/AdjustRangeControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/AdjustModeControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/AdjustControllerValueMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/SetPowerControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/SetModeControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/SetRangeControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/SetToggleControllerValueMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/CarControl/CarControl/SetControllerValueMessageReply.h
)

source_group("Car Control Message Headers" FILES ${AASB_CARCONTROL_MESSAGES})

add_library(AASBCarControlEngine SHARED
    ${HEADERS}
    ${AASB_CARCONTROL_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBCarControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBCarControlEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBCarControlEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBCarControlEngine
    PUBLIC
        AACEAASBEngine
        AACECarControlPlatform
)

install(
    TARGETS AASBCarControlEngine
    DESTINATION lib
    EXPORT AASBCarControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
