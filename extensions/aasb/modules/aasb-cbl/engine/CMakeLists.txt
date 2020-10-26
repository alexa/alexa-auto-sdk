# AASB CBL Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/CBL/AASBCBLEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/CBL/AASBCBL.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_CBL_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/CancelMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/CBLState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/CBLStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/CBLStateChangedReason.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/ClearRefreshTokenMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/GetRefreshTokenMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/GetRefreshTokenMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/ResetMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/SetRefreshTokenMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/SetUserProfileMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Cbl/CBL/StartMessage.h
)

source_group("CBL Message Headers" FILES ${AASB_CBL_MESSAGES})

add_library(AASBCBLEngine SHARED
    ${HEADERS}
    ${AASB_CBL_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBCBLEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBCBL.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBCBLEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBCBLEngine
    PUBLIC
        AACEAASBEngine
        AACECBLPlatform
)

install(
    TARGETS AASBCBLEngine
    DESTINATION lib
    EXPORT AASBCBL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
