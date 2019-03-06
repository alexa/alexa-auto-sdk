# Alexa Auto SDK C++ Sample App

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(SampleApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("No build type specified, defaulting to DEBUG.")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Wall -Wno-covered-switch-default -Wno-missing-braces -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -Wall -Wno-covered-switch-default -Wno-missing-braces -g")

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_GENERATOR STREQUAL "Unix Makefiles" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
endif()

if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
    include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
    include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)
    include(${AAC_HOME}/share/cmake/AACEAudio.cmake)
    include(${AAC_HOME}/share/cmake/AACECBL.cmake)
    if(AMAZONLITE)
        include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
    endif()
endif()

# Find cURL library for the sample app.
find_package(CURL REQUIRED)

# Find SQLite3 package for the sample app.
find_package(SQLite3 REQUIRED)

file(GLOB CXX_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/*/*.h
)

file(GLOB CXX_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
)

source_group("Header Files" FILES ${CXX_HEADER_FILES})

add_executable(SampleApp
    ${CXX_HEADER_FILES}
    ${CXX_SOURCE_FILES}
)

target_include_directories(SampleApp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include> # <prefix>/include
    ${CURL_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(SampleApp
    AACECorePlatform
    AACECoreEngine
    AACEAlexaPlatform
    AACEAlexaEngine
    AACENavigationPlatform
    AACENavigationEngine
    AACEPhoneControlPlatform
    AACEPhoneControlEngine
    AACEGStreamerAudio
    AACECBLPlatform
    AACECBLEngine
    ${CURL_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

if(AMAZONLITE)
    target_link_libraries(SampleApp
        AACEAmazonLiteEngine
    )
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "(Android|QNX)")
    target_link_libraries(SampleApp pthread)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    if(PORTAUDIO)
        target_link_libraries(SampleApp
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreServices"
            "-framework Carbon"
        )
    endif()
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
    # AACE modules must be explicitly linked
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

install(
    TARGETS SampleApp
    DESTINATION bin
)
