/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#ifndef AASB_IAASBCONTROLLER_H
#define AASB_IAASBCONTROLLER_H

#include <memory>

#include <aasb/interfaces/IConfigurationProvider.h>
#include <aasb/interfaces/IAlexaCapabilityDirectiveListener.h>
#include <AACE/Core/Engine.h>

namespace aasb {
namespace bridge {
/**
 * This class is an entry point for Alexa Auto Services Bridge clients.
 * Alexa Auto Services Bridge clients are typically services on each platform such
 * as Android, AGL, QNX etc.
 * Alexa Auto SDK is a client side component(s) which is responsible for maintaining
 * connection with Alexa Cloud and exchanging events and directives with Alexa cloud.
 * Alexa Auto Services Bridge provides convinient apis to interact with Alexa Auto
 * SDK.
 */
class IAASBController {
public:
    /**
     * Initializes the Alexa Auto Core Engine.
     *
     * @param configProvider An object to provide the necessary configuration for AASB.
     */
    virtual bool init(std::shared_ptr<IConfigurationProvider> configProvider) = 0;

    /**
     * Call @c start after @c init is successfully completed. This method will register the
     * capabilities and start the Alexa Auto Engine.
     *
     */
    virtual bool start() = 0;

    /**
     * Client should register an instance of @c IAlexaCapabilityDirectiveListener to receive
     * callbacks from AASB for processing incoming directive sent by Alexa.
     *
     * @param listener Listener which will receive callbacks on all the incoming
     *      directives that client of this library has to process.
     * @return true on success, false otherwise.
     */
    virtual bool registerCapabilityDirectiveListener(std::shared_ptr<IAlexaCapabilityDirectiveListener> listener) = 0;

    /**
     * Start listening to user speech. Should be invoked in response to user's action such
     * as pressing "tap to talk" UI element (or physical button).
     *
     * @param mode Mode in which to start listening. Refer to LISTENING_MODE_XYZ macros for
     *      List of possible listening modes.
     */
    virtual bool startListening(std::string mode) = 0;

    /**
     * Client should call this method when they want to send an event to Auto SDK.
     * Following are couple of examples:
     * 1) topic: TOPIC_AUTH_PROVIDER, action: ACTION_SET_AUTH_TOKEN, payload: "refresh-token"
     * 2) topic: TOPIC_AUDIO_PLAYER, action: ACTION_MEDIA_STATE_CHANGED, payload: "PLAYING"
     *
     * @param topic Which handler should handle the event.
     * @param action What is the event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    virtual void onReceivedEvent(const std::string& topic, const std::string& action, const std::string& payload) = 0;

    /**
     * Sends the audio samples for Alexa processing. Client should continuously call this
     * method for the duration when they wish the user speech to be processed by Alexa.
     *
     * Client doesn't need to call this method if they have supplied the custom audio input
     * output provider by calling start with a valid @c IAudioInputOutputProvider
     *
     * @param data Buffer where audio samples are stored.
     * @param size Number of samples in the buffer.
     * @return Number of samples sent to Alexa for processing. It can return -ve number
     *      in case of error.
     */
    virtual ssize_t writeAudioSamples(const int16_t* data, const size_t size) = 0;

    /**
     * This is called by client to enable or disable WakeWord.
     *
     * @param wakeWordEnable -  true to enable or false to disable
     */
    virtual bool setWakeWordEnable(bool wakeWordEnable) = 0;

    /**
     * Optional method for client to set a different locale.
     *
     * @param locale New locale to set.
     */
    virtual void setLocale(const std::string& locale) = 0;

    /**
     * Optional method for client to set the firmware version.
     *
     * @param firmwareVersion Firmware version to set.
     */
    virtual void setFirmwareVersion(const std::string& firmwareVersion) = 0;

private:
    /**
     * set mock engine instance for testing (must be used only by test).
     * @param engine instance
     */
    virtual void setMockEngine(std::shared_ptr<aace::core::Engine> engine) = 0;
    /**
     * Friend for testing
     */
    friend class AASBControllerTest;
};

}  // namespace bridge
}  // namespace aasb

#endif  // AASB_IAASBCONTROLLER_H
