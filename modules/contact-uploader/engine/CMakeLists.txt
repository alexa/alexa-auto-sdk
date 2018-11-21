# AACE ContactUploader Engine

# Find AVSCommon headers & library
# Reason: ContactUploader engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/ContactUploader/ContactUploaderEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/ContactUploader/ContactUploaderEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/ContactUploader/ContactUploaderRESTAgent.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEContactUploaderEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactUploader/ContactUploaderEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactUploader/ContactUploaderEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactUploader/ContactUploaderRESTAgent.cpp
)

target_include_directories(AACEContactUploaderEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
)

target_link_libraries(AACEContactUploaderEngine
    PUBLIC
        AACEContactUploaderPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${CURL_LIBRARIES}
)

install(
    TARGETS AACEContactUploaderEngine
    DESTINATION lib
    EXPORT AACEContactUploader
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
