# AACE AASB Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/AASBEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/AASBEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/MessageBrokerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/MessageBroker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/Message.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/PublishMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/StreamManagerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/StreamManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/AASBHandlerEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AASB/AASBServiceInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAASBEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MessageBroker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PublishMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StreamManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBHandlerEngineService.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBServiceInterface.cpp 
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACEAASBEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AACEAASBEngine
    PUBLIC
        AACEAASBPlatform
        AACECoreEngine
)

install(
    TARGETS AACEAASBEngine
    DESTINATION lib
    EXPORT AACEAASB
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
