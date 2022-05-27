set(CXX_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Activity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ApplicationContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Executor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Extension.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Views.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

# create the cmake defines from conan defines
foreach(opt ${CONAN_DEFINES})
    string(SUBSTRING ${opt} 2 -1 sub_opt)
    set(${sub_opt} ON)
endforeach()

if(NOT AAC_CORE)
    message( FATAL_ERROR "AAC_CORE must be defined!" )
endif()

set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioInputProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioOutputProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/AuthorizationHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/AuthProviderAuthorizationHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/LocationProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkInfoProviderHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PropertyManager/PropertyManagerHandler.cpp
)

if(AAC_ALEXA)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlertsHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlexaClientHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AudioPlayerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/DeviceSetupHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/DoNotDisturbHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/EqualizerControllerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/LocalMediaSourceHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/MediaPlaybackRequestorHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/NotificationsHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/FeatureDiscoveryHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/PlaybackControllerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechRecognizerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/SpeechSynthesizerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/TemplateRuntimeHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Alexa/AlexaSpeakerHandler.cpp
    )
endif()

if(AAC_ADDRESS_BOOK)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBook/AddressBookHandler.cpp
    )
endif()

if(AAC_NAVIGATION)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Navigation/NavigationHandler.cpp
    )
endif()

if(AAC_MESSAGING)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Messaging/MessagingHandler.cpp
    )
endif()

if(AAC_PHONE_CONTROL)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PhoneControl/PhoneControlHandler.cpp
    )
endif()

if(AAC_CAR_CONTROL)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl/CarControlDataProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControl/CarControlHandler.cpp
    )
endif()

if(AAC_TEXT_TO_SPEECH)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeech/TextToSpeechHandler.cpp
    )
endif()

if(AAC_ALEXACOMMS)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Communication/AlexaCommsHandler.cpp
    )
endif()

if(AAC_CONNECTIVITY)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Connectivity/AlexaConnectivityHandler.cpp
    )
endif()

if(AAC_DCM)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/DCM/DCMHandler.cpp
    )
endif()

if(AAC_LOCAL_VOICE_CONTROL)
    set(CXX_SOURCE_FILES ${CXX_SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalNavigation/LocalSearchProviderHandler.cpp
    )
endif()
