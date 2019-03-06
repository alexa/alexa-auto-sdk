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
#ifndef AASB_PHONECONTROL_PHONECALLCONTROLLERHANDLER_H
#define AASB_PHONECONTROL_PHONECALLCONTROLLERHANDLER_H

#include <memory>

#include <AACE/PhoneCallController/PhoneCallController.h>

#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace phoneCallController {

/**
 * Platform implementation for @c aace::phoneCallController::PhoneCallController.
 *
 * AASB clients will be notified, through @c IAlexaCapabilityDirectiveListener, about the
 * directives to dial, redial, answer etc. on the devices connected to the target device.
 *
 * AASB client should let AASB know about the state changes on the phone connected to the
 * target device (by calling @c RequestHandler::onReceivedEvent). Example of such state
 * changes are, phone connected/disconnected to device, phone is ringing with inbound call,
 * phone is engaged in conversation etc.
 */
class PhoneCallControllerHandler : public aace::phoneCallController::PhoneCallController
{
public:
    /**
     * Creates an instance of @c PhoneCallControllerHandler.
     *
     * @param logger An instance of logger.
     * @param directiveDispatcher An object through which the directives for phone call control
     *      received from alexa cloud will be dispatched to AASB clients.
     */
    static std::shared_ptr<PhoneCallControllerHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::phoneCallController::PhoneCallController
    /// @{
    bool dial(const std::string &payload) override;
    bool redial(const std::string &payload) override;
    void answer(const std::string &payload) override;
    void stop(const std::string &payload) override;
    void sendDTMF(const std::string &payload) override;
    /// @}

    /**
     * Process incoming events from AASB client meant for topic @c TOPIC_PHONECALL_CONTROLLER
     *
     * @param action Type of event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    /**
     * Constructor for @c PhoneCallControllerHandler.
     */
    PhoneCallControllerHandler(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /**
     * Notifies the Engine of a change in connection to a calling device
     * 
     * @param payload Payload containing data.
     */
    void connectionStateChanged(const std::string& payload);

    /**
     * Notifies the Engine of a change in the state of an ongoing call
     * 
     * @param payload Payload containing data.
     */
    void callStateChanged(const std::string& payload);

    /**
     * Notifies the Engine of an error related to a call
     * 
     * @param payload Payload containing data.
     */
    void callFailed(const std::string& payload);

    /**
     * Notifies the Engine that a caller id was received for an inbound call
     * 
     * @param payload Payload containing data.
     */
    void callerIdReceived(const std::string& payload);

    /** 
     * Notifies the Engine that sending the DTMF signal succeeded.
     * 
     * @param payload Payload containing data.
     */
    void sendDTMFSucceeded(const std::string& payload);

    /** 
     * Notifies the Engine that the DTMF signal could not be delivered to the remote party
     * 
     * @param payload Payload containing data.
     */
    void sendDTMFFailed(const std::string& payload);

    /**
     * Notifies the Engine of the calling feature configuration of the connected calling device.
     * 
     * @param payload Payload containing data.
     */
    void deviceConfigurationUpdated(const std::string& payload);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // To send directive to service
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;
};

}  // phoneCallController
}  // aasb

#endif  // AASB_PHONECONTROL_PHONECALLCONTROLLERHANDLER_H