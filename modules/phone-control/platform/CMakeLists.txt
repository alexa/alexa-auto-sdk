# AACE PhoneCallController Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/PhoneCallController/PhoneCallController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/PhoneCallController/PhoneCallControllerEngineInterfaces.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEPhoneControlPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneCallController.cpp
)

target_include_directories(AACEPhoneControlPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEPhoneControlPlatform
    AACECorePlatform
)

install(
    TARGETS AACEPhoneControlPlatform
    DESTINATION lib
    EXPORT AACEPhoneControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    enable_testing()
endif()