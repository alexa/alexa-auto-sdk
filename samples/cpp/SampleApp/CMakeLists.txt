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

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_GENERATOR STREQUAL "Unix Makefiles" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
endif()

if(AAC_ENABLE_ADDRESS_SANITIZER)
    message(STATUS "Enabling Address Sanitizer for SampleApp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

if(AAC_ENABLE_COVERAGE)
    message(STATUS "Enabling Code Coverage for SampleApp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
    include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
    include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)
    include(${AAC_HOME}/share/cmake/AACECBL.cmake)
    include(${AAC_HOME}/share/cmake/AACEAddressBook.cmake)
    if(AMAZONLITE)
        include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
        add_definitions(-DAMAZONLITE)
    endif()
    if(LOCALVOICECONTROL)
        include(${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)
        include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
        include(${AAC_HOME}/share/cmake/AACECarControl.cmake)
        include(${AAC_HOME}/share/cmake/AACEAddressBookLocalService.cmake)
        add_definitions(-DLOCALVOICECONTROL)
    endif()
    if(ALEXACOMMS)
        include(${AAC_HOME}/share/cmake/AACECommunication.cmake)
        add_definitions(-DALEXACOMMS)
    endif()
    if(GSTREAMER)
        include(${AAC_HOME}/share/cmake/AACEGStreamer.cmake)
        add_definitions(-DGSTREAMER)
    endif()
    if(DCM)
        include(${AAC_HOME}/share/cmake/AACEDcmNativeMetrics.cmake)
        add_definitions(-DDCM)
    endif()
    if(LOOPBACK_DETECTOR)
        include(${AAC_HOME}/share/cmake/AACELoopbackDetector.cmake)
    endif()
else()
    if(AMAZONLITE)
        add_definitions(-DAMAZONLITE)
    endif()
    if(LOCALVOICECONTROL)
        add_definitions(-DLOCALVOICECONTROL)
    endif()
    if(ALEXACOMMS)
        add_definitions(-DALEXACOMMS)
    endif()
    if(GSTREAMER)
        add_definitions(-DGSTREAMER)
    endif()
    if(DCM)
        add_definitions(-DDCM)
    endif()
endif()

# Find cURL library for the sample app.
find_package(CURL REQUIRED)

# Find SQLite3 package for the sample app.
find_package(SQLite3 REQUIRED)

set(CXX_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Activity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/AddressBook/AddressBookHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AlertsHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AlexaClientHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AudioPlayerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/EqualizerControllerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/LocalMediaSourceHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/NotificationsHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/PlaybackControllerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/SpeechRecognizerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/SpeechSynthesizerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/TemplateRuntimeHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/AlexaSpeakerHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Alexa/GlobalPresetHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Audio/AudioInputProviderHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Audio/AudioOutputProviderHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/ApplicationContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Args.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CBL/CBLHandler.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/CarControlHandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/CarControlDataProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/BoolController.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/RangeController.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/CarControl/ModeController.h
    )
endif()

if(ALEXACOMMS)
    set(CXX_HEADER_FILES ${CXX_HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SampleApp/Communication/CommunicationHandler.h
    )
endif()

set(CXX_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Activity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBook/AddressBookHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlertsHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlexaClientHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AudioPlayerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/EqualizerControllerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/LocalMediaSourceHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/NotificationsHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/PlaybackControllerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechRecognizerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechSynthesizerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/TemplateRuntimeHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlexaSpeakerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/GlobalPresetHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioInputProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioOutputProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ApplicationContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CBL/CBLHandler.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl/CarControlHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl/CarControlDataProvider.cpp
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
    AACEAddressBookPlatform
    AACEAddressBookEngine
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
        AACEGStreamerEngine
    )
endif()

if(DCM)
    target_link_libraries(SampleApp
        AACEDCMEngine
        AACEDCMPlatform
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
        AACEAddressBookLocalServiceEngine
    )
endif()

if(ALEXACOMMS)
    target_link_libraries(SampleApp
        AACECommunicationPlatform
        AACECommunicationEngine
    )
endif()

if(LOOPBACK_DETECTOR)
    target_link_libraries(SampleApp AACELoopbackDetectorEngine)
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

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_definitions(-DMONITORAIRPLANEMODEEVENTS)
endif()

install(
    TARGETS SampleApp
    DESTINATION bin
)

# Install files for SampleApp execution

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../assets/config.json.in ${CMAKE_CURRENT_BINARY_DIR}/configure/config.json)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/configure/config.json
          ${CMAKE_CURRENT_SOURCE_DIR}/../assets/menu.json
    DESTINATION etc
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../assets/certs
              ${CMAKE_CURRENT_SOURCE_DIR}/../assets/inputs
              ${CMAKE_CURRENT_SOURCE_DIR}/../assets/sampledata
    DESTINATION etc
)
