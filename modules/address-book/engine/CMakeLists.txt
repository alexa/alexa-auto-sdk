# AACE AddressBook Engine

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

if (${USE_OBIGO_BUILD_SYSTEM})
find_path(CURL_INCLUDE_DIRS NAMES curl/curl.h PATHS "${VPA_SYSROOT_PATH}/usr/include")
find_library(CURL_LIBRARIES NAMES curl libcurl PATHS "${VPA_SYSROOT_PATH}/usr/lib")
endif ()

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBook/AddressBookCloudUploader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBook/AddressBookCloudUploaderRESTAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBook/AddressBookEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBook/AddressBookEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAddressBookEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookCloudUploader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookCloudUploaderRESTAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookEngineService.cpp
)

target_include_directories(AACEAddressBookEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
        ${AVS_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
)

target_link_libraries(AACEAddressBookEngine
    PUBLIC
        AACEAddressBookPlatform
        AACECoreEngine
    PRIVATE
        AACEAlexaEngine
        ${AVS_AVS_COMMON_LIBRARY}
        ${CURL_LIBRARIES}
)

install(
    TARGETS AACEAddressBookEngine
    DESTINATION lib
    EXPORT AACEAddressBook
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
