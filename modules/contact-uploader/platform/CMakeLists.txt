# AACE ContactUploader Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/ContactUploader/ContactUploader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/ContactUploader/ContactUploaderEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEContactUploaderPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactUploader/ContactUploader.cpp
)

target_include_directories(AACEContactUploaderPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEContactUploaderPlatform
    AACECorePlatform
)

install(
    TARGETS AACEContactUploaderPlatform
    DESTINATION lib
    EXPORT AACEContactUploader
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    enable_testing()
endif()