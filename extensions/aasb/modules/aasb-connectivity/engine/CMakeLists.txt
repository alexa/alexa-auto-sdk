# AASB Connectivity Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Connectivity/AASBAlexaConnectivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Connectivity/AASBConnectivityEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_CONNECTIVITY_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessageReply.h
)

source_group("Message Header Files" FILES ${AASB_CONNECTIVITY_MESSAGES})

add_library(AASBConnectivityEngine SHARED
    ${HEADERS}
    ${AASB_CONNECTIVITY_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAlexaConnectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBConnectivityEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBConnectivityEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBConnectivityEngine
    PUBLIC
        AACEAASBEngine
        AACEConnectivityPlatform
)

install(
    TARGETS AASBConnectivityEngine
    DESTINATION lib
    EXPORT AASBConnectivity
)

install(
    DIRECTORY include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
