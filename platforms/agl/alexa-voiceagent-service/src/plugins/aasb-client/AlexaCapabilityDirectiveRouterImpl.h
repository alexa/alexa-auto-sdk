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

#ifndef AGL_ALEXA_SVC_ALEXA_CAPABILITY_DIRECTIVE_ROUTER_IMPL_H_
#define AGL_ALEXA_SVC_ALEXA_CAPABILITY_DIRECTIVE_ROUTER_IMPL_H_

#include <aasb/interfaces/IAlexaCapabilityDirectiveListener.h>

#include <condition_variable>
#include <memory>
#include <string>
#include <unordered_map>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/capability/ICapabilityMessageDispatcher.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace alexa {

/**
 * Implements the @c aasb::bridge::IAlexaCapabilityDirectiveListener to receive directives
 * coming from Alexa services.
 */
class AlexaCapabilityDirectiveRouterImpl : public aasb::bridge::IAlexaCapabilityDirectiveListener {
public:
    /**
     * Creates a new instance of @c AlexaCapabilityDirectiveRouterImpl
     *
     * @param logger An instance of logger.
     * @param api AFB API instance to notify other AGL services of incoming directives.
     */
    static std::shared_ptr<AlexaCapabilityDirectiveRouterImpl> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    /**
     * Sets the id of voice agent. This id will be used in the payload for auth, dialog and
     * connection state change events.
     *
     * @param voiceAgentId Id of the current (Alexa) voice agent as determined by AGL voice
     *      framework.
     */
    void setVoiceAgentId(std::string voiceAgentId);

    /// @name IAlexaCapabilityDirectiveListener Functions
    /// @{
    void onReceivedDirective(const std::string& topic, const std::string& action, const std::string& jsonPayload)
        override;
    /// @}

    /**
     * Register the directive listener which will be responsible for processing directives
     * for the given topic.
     *
     * @param topic Topic for which directive would be sent to the @c listener.
     * @param listener Listener of the specific topic directives.
     */
    void registerCapabilityDispatcher(
        const std::string& topic,
        std::shared_ptr<agl::capability::interfaces::ICapabilityMessageDispatcher> listener);

    /**
     * Subscribe to auth state events. Subscriber is notified anytime auth state is changed.
     *
     * @param subscriber Subscriber (residing in a separate AGL binding) interested in auth
     *      events.
     */
    bool subscribeToAuthStateEvents(agl::common::interfaces::IAFBRequest& subscriber);

    /**
     * Subscribe to dialog state events. Subscriber is notified anytime dialog state is changed.
     *
     * @param subscriber Subscriber (residing in a separate AGL binding) interested in dialog
     *      events.
     */
    bool subscribeToDialogStateEvents(agl::common::interfaces::IAFBRequest& subscriber);

    /**
     * Subscribe to connection state events. Subscriber is notified anytime connection state is changed.
     *
     * @param subscriber Subscriber (residing in a separate AGL binding) interested in connection
     *      events.
     */
    bool subscribeToConnectionStateEvents(agl::common::interfaces::IAFBRequest& subscriber);

    /**
     * Subscribe to CBL events. Subscriber is notified anytime CBL notifications need to be raised.
     *
     * @param subscriber Subscriber (residing in a separate AGL binding) interested in CBL
     *      events.
     */
    bool subscribeToCBLEvents(agl::common::interfaces::IAFBRequest& subscriber);

    /**
     * This method is called in response to AASB CBL's get refresh token directive.
     * When this call arrives we can safely assume that the CBL implementation of
     * AASB has received the latest refresh token. This method is used to unblock
     * the thread that handled ACTION_CBL_GET_REFRESH_TOKEN directive.
     */
    void didReceiveGetRefreshTokenResponse();

private:
    AlexaCapabilityDirectiveRouterImpl(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    /**
     * Initialize the @c AlexaCapabilityDirectiveRouterImpl object. The object is
     * considered invalid until it is initialized with success.
     *
     * @reutrn true when initialize succeeded, false otherwise.
     */
    bool initialize();

    /**
     * Publish the current auth state to all subscribers who have subscribed using
     * @c subscribeToAuthStateEvents call.
     */
    void publishCurrentAuthState();

    /**
     * Publish the current dialog state to all subscribers who have subscribed using
     * @c subscribeToDialogStateEvents call.
     */
    void publishCurrentDialogState();

    /**
     * Publish the current connection state to all subscribers who have subscribed using
     * @c subscribeToConnectionStateEvents call.
     */
    void publishCurrentConnectionState();

    /**
     * Creates an Json object of following format:
     * {"va-id": m_alexaVoiceAgentId, "state": state}
     *
     * @param state Value of the state going into json object.
     */
    json_object* createStateEventJsonObject(const std::string& state);

    /**
     * Process the action coming in for topic @c aasb::bridge::TOPIC_ALEXA_CLIENT
     *
     * @c action Action/Event to process.
     * @c payload Parameters necessary for taking an action.
     */
    void processAlexaClientAction(const std::string& action, const std::string& payload);

    /**
     * Process the action coming in for topic @c aasb::bridge::TOPIC_TEMPLATE_RUNTIME
     *
     * @c action Action/Event to process.
     * @c payload Parameters necessary for taking an action.
     */
    void processTemplateRuntimeAction(const std::string& action, const std::string& payload);

    /**
     * Process the action coming in for topic @c aasb::bridge::TOPIC_CBL
     *
     * @c action Action/Event to process.
     * @c payload Parameters necessary for taking an action.
     */
    void processCBLAction(const std::string& action, const std::string& payload);

    /// Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

    /// AFB API object for events pub/sub, and for calling other AGL services.
    std::shared_ptr<agl::common::interfaces::IAFBApi> m_api;

    /// Id of the current (Alexa) voice agent as determined by AGL voice framework.
    std::string m_alexaVoiceAgentId;

    /// AFB Event for publishing alexa service auth state changes.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_authStateEvent;

    /// Current auth state of alexa voice agent.
    std::string m_currentAuthState;

    /// AFB Event for publishing alexa service dialog state changes.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_dialogStateEvent;

    /// Current dialog state of alexa voice agent.
    std::string m_currentDialogState;

    /// AFB Event for publishing alexa service connection state changes.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_connectionStateEvent;

    /// AFB Event for publishing code pair received event.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_cblCodePairReceivedEvent;

    /// AFB Event for publishing code pair expired event.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_cblCodePairExpiredEvent;

    /// AFB Event for publishing CBL set refresh token event.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_cblSetRefreshTokenEvent;

    /// AFB Event for publishing CBL clear refresh token event.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_cblClearRefreshTokenEvent;

    /// AFB Event for publishing CBL get refresh token event.
    std::shared_ptr<agl::common::interfaces::IAFBApi::IAFBEvent> m_cblGetRefreshTokenEvent;

    /// All external directive listeners who are asked to process directives if directive
    /// belongs to their registered topic (registered through @c registerDirectiveListener)
    std::unordered_map<std::string, std::shared_ptr<agl::capability::interfaces::ICapabilityMessageDispatcher>>
        m_externalDirectiveListeners;

    /// Current connection state of alexa voice agent.
    std::string m_currentConnectionState;

    // Conditional variable and mutex for fetching refresh token synchronously
    // AGL binding/app responsible for storing refreshing tokens securely.
    std::condition_variable m_getRefreshTokenCv;
    std::mutex m_getRefreshTokenMutex;

    // The value is updated as soon the response for ACTION_GET_REFRESH_TOKEN
    // is received.
    bool m_didReceiveGetRefreshTokenResponse;
};
}  // namespace alexa
}  // namespace agl

#endif  // AGL_ALEXA_SVC_ALEXA_CAPABILITY_DIRECTIVE_ROUTER_IMPL_H_
