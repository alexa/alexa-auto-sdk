# AACE Messaging Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Messaging/Messaging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Messaging/MessagingEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEMessagingPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Messaging.cpp
)

target_include_directories(AACEMessagingPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEMessagingPlatform
    AACECorePlatform
)

install(
    TARGETS AACEMessagingPlatform
    DESTINATION lib
    EXPORT AACEMessaging
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    enable_testing()
endif()