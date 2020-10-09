# AACE AASB Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/AASB/AASB.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/AASB/AASBStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/AASB/AASBEngineInterfaces.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAASBPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASB.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBStream.cpp
)

target_include_directories(AACEAASBPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEAASBPlatform
    AACECorePlatform
)

if(AAC_ENABLE_ADDRESS_SANITIZER)
    message(STATUS "Enabling Address Sanitizer for AACEAASB")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling coverage for AACEAASB.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

install(
    TARGETS AACEAASBPlatform
    DESTINATION lib
    EXPORT AACEAASB
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
