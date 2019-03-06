###########################################################################
# Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

PROJECT_TARGET_ADD(alexa-voiceagent-api)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(ALEXA_VOICEAGENT_LIB_SRC
        # API
        ${CMAKE_CURRENT_SOURCE_DIR}/AlexaVoiceagentApi.cpp

        # Consts
        ${CMAKE_CURRENT_SOURCE_DIR}/AlexaConsts.cpp

        # AFB
        ${CMAKE_CURRENT_SOURCE_DIR}/afb/src/AFBEventImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/afb/AFBApiImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/afb/AFBRequestImpl.cpp

        # AASB Client
        ${CMAKE_CURRENT_SOURCE_DIR}/aasb-client/AlexaCapabilityDirectiveRouterImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aasb-client/config/AASBConfigProviderImpl.cpp

        # Dispatchers
        ${CMAKE_CURRENT_SOURCE_DIR}/dispatchers/navigation/NavigationDispatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dispatchers/phonecall/PhoneCallDispatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dispatchers/playback/PlaybackDispatcher.cpp

        # Utilities
        ${CMAKE_CURRENT_SOURCE_DIR}/utilities/logging/Logger.cpp
    )

    # Define targets
    ADD_LIBRARY(${TARGET_NAME} MODULE
        ${ALEXA_VOICEAGENT_LIB_SRC}
    )

    # Alexa VoiceAgent plugin properties
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "PLUGIN"
        PREFIX ""
        SUFFIX ".ctlso"
        OUTPUT_NAME ${TARGET_NAME}
    )

    # Define target includes
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME}
        PUBLIC ${GLIB_PKG_INCLUDE_DIRS}
        PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}"
        PRIVATE "${CMAKE_SOURCE_DIR}/app-controller/ctl-lib"
    )

    # Library dependencies (include updates automatically)
    TARGET_LINK_LIBRARIES(${TARGET_NAME}
        afb-helpers
        AlexaAutoServicesBridgeLib
        ${GLIB_PKG_LIBRARIES}
        ${link_libraries}
    )

    option(ENABLE_UNIT_TESTS "Build unit tests or not" OFF)
    if (ENABLE_UNIT_TESTS)
        execute_process(
            COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/agreement.sh"
            RESULT_VARIABLE AGREEMENT_RESULT
        )
        message(STATUS "Agreement Result: ${AGREEMENT_RESULT}")
        if (${AGREEMENT_RESULT} MATCHES "1")
            message(FATAL_ERROR "User agreement not accepted. Quitting")
        endif()

        include(cmake/gtest.cmake)

        set(ALEXA_VOICEAGENT_TEST_SRC ${ALEXA_VOICEAGENT_LIB_SRC})
        list(APPEND ALEXA_VOICEAGENT_TEST_SRC
            # Main
            ${CMAKE_CURRENT_SOURCE_DIR}/TestMain.cpp
        )

        ADD_EXECUTABLE(${TARGET_NAME}_Test
            ${ALEXA_VOICEAGENT_TEST_SRC}
        )

        TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME}_Test
            PUBLIC ${GLIB_PKG_INCLUDE_DIRS}
            PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}"
            PRIVATE "${CMAKE_SOURCE_DIR}/app-controller/ctl-lib"
        )

        TARGET_LINK_LIBRARIES(${TARGET_NAME}_Test
            afb-helpers
            libgtest
            libgmock
            ${GLIB_PKG_LIBRARIES}
            ${link_libraries}
        )

        ENABLE_TESTING()
        ADD_TEST(AlexaVoiceAgentTest ${TARGET_NAME}_Test)
    endif()

    add_subdirectory("aac-services")

    # Static data such as configurations
    add_subdirectory("data")

    # make wgt packager discover and pack the AlexaAutoServicesBridgeLib
    PROJECT_TARGET_ADD(AlexaAutoServicesBridgeLib)
    SET_TARGET_PROPERTIES(AlexaAutoServicesBridgeLib
    PROPERTIES
        LABELS LIBRARY
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/src/plugins/aac-services/libAlexaAutoServicesBridgeLib.so
    )
