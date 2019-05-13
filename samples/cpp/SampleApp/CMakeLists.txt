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
        add_definitions(-DAMAZONLITE)
    endif()
    if(LOCALVOICECONTROL)
        include(${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)
        include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
        include(${AAC_HOME}/share/cmake/AACECarControl.cmake)
        add_definitions(-DLOCALVOICECONTROL)
    endif()
    if(ALEXACOMMS)
        include(${AAC_HOME}/share/cmake/AACECommunication.cmake)
        add_definitions(-DALEXACOMMS)
    endif()
endif()

# Find cURL library for the sample app.
find_package(CURL REQUIRED)

# Find SQLite3 package for the sample app.
find_package(SQLite3 REQUIRED)

set(CXX_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Activity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AlertsHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AlexaClientHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AudioChannelHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AudioPlayerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/EqualizerControllerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/LocalMediaSourceHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/NotificationsHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/PlaybackControllerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/SpeechRecognizerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/SpeechSynthesizerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/TemplateRuntimeHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/ApplicationContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Args.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/AudioFileReader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/AudioInputManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CBL/CBLHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/DefaultMediaPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Executor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Location/LocationProviderHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Logger/LoggerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Navigation/NavigationHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Network/NetworkInfoProviderHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/PhoneControl/PhoneControlHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Status.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Subject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/TaskQueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/TaskThread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/TTY.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Views.h
)

if(LOCALVOICECONTROL)
    set(CXX_HEADER_FILES ${CXX_HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/ClimateControlHandler.h
    )
endif()

if(ALEXACOMMS)
    set(CXX_HEADER_FILES ${CXX_HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Communication/CommunicationHandler.h
    )
endif()

set(CXX_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Activity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlertsHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlexaClientHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AudioChannelHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AudioPlayerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/EqualizerControllerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/LocalMediaSourceHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/NotificationsHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/PlaybackControllerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechRecognizerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechSynthesizerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/TemplateRuntimeHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ApplicationContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioFileReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioInputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBL/CBLHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DefaultMediaPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Executor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/LocationProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Navigation/NavigationHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkInfoProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneControl/PhoneControlHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Views.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

if(LOCALVOICECONTROL)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl/ClimateControlHandler.cpp
    )
endif()

if(ALEXACOMMS)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Communication/CommunicationHandler.cpp
    )
endif()

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
    AACECBLPlatform
    AACECBLEngine
    ${CURL_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

if(AMAZONLITE)
    target_link_libraries(SampleApp
        # AACEAmazonLitePlatform
        AACEAmazonLiteEngine
    )
endif()

if(FILEAUDIO)
    target_link_libraries(SampleApp
        AACEFileAudio
    )
endif()

if(GSTREAMER)
    target_link_libraries(SampleApp
        AACEGStreamerAudio
    )
endif()

if(LOCALVOICECONTROL)
    target_link_libraries(SampleApp
        AACELocalSkillServicePlatform
        AACELocalSkillServiceEngine
        AACECarControlPlatform
        AACECarControlEngine
        AACELocalVoiceControlPlatform
        AACELocalVoiceControlEngine
    )
endif()

if(ALEXACOMMS)
    target_link_libraries(SampleApp
        AACECommunicationPlatform
        AACECommunicationEngine
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
