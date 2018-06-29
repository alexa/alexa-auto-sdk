/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_H
#define AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_H

#include <string>
#include "AACE/Core/PlatformInterface.h"
#include "PhoneCallControllerEngineInterfaces.h"

/** @file */ 

namespace aace {
namespace phoneCallController {

/**
 * PhoneCallController should be extended to handle directives to initiate a phone call
 * on the platform calling device. It also provides interfaces for notifying the Engine 
 * of the state of a call session and the state of platform connection to the calling device.
 *
 * The platform implementation is responsible for managing the lifecycle and user experience of a call session
 * and connection to the calling device.
 */
class PhoneCallController : public aace::core::PlatformInterface {
protected:
    PhoneCallController() = default;

public: 

    /**
     * Specifies the state of connection to the platform calling device
     * @sa @c aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState
     */
    using ConnectionState = aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState;

    virtual ~PhoneCallController();

    /**
     * Notifies the platform implementation to initiate an outgoing phone call to a destination address
     * 
     * @param [in] payload Details of the outgoing call in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json})
     *  {
     *     "callId": "{{STRING}}",
     *     "callee": {
     *       "details": "{{STRING}}",
     *       "defaultAddress": {
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": "{{STRING}}"
     *       },
     *       "alternativeAddresses": [{
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": {{STRING}}
     *       }]
     *     }
     *   }
     * }
     * @endcode 
     * @li callId (required): A unique identifier for the call
     * @li callee (required): The destination of the outgoing call
     * @li callee.details (optional): Descriptive information about the callee
     * @li callee.defaultAddress (required): The default address to use for calling the callee
     * @li callee.alternativeAddresses (optional): An array of alternate addresses for the existing callee
     * @li address.protocol (required): The protocol for this address of the callee (e.g. PSTN, SIP, H323, etc.)
     * @li address.format (optional): The format for this address of the callee (e.g. E.164, E.163, E.123, DIN5008, etc.)
     * @li address.value (required): The address of the callee.
     *
     * @return @c true if the platform implementation will initiate the call, 
     * else @c false
     */ 
    virtual bool dial( const std::string& payload ) = 0;
    
    /**
     * Notifies the Engine of a change in connection state of the platform calling device
     */
    void connectionStateChanged( PhoneCallControllerEngineInterface::ConnectionState state );

    /**
     * Notifies the Engine that a phone call was activated on the platform calling device.
     *
     * @c callActivated() should be called in response to a @c dial() directive in which the platform implementation returned @c true.
     * @c callId must match the @c callId from the @c dial() payload.
     *
     * When the platform implementation calls @c callActivated() for a call initiated outside of the scope of Alexa,
     * it is responsible for creating a UUID for this call session.
     *
     * @param [in] callId The unique identifier for the call
     */
    void callActivated( const std::string& callId );

    /**
     * Notifies the Engine of an error in initiating or maintaining a call on the platform calling device
     * 
     * @param [in] callId The unique identifier for the call
     * @param [in] error An error status code:
     * @li 4xx range: Validation failure for the input from the @c dial() directive
     * @li 500: Internal error on the platform unrelated to the cellular network
     * @li 503: Error on the platform related to the cellular network
     * @param [in] message A description of the error
     */
    void callFailed( const std::string& callId, const std::string& error, const std::string& message );

    /**
     * Notifies the Engine that an active call was terminated or an ongoing phone call setup was cancelled
     * 
     * @param [in] callId The unique identifier for the call
     */
    void callTerminated( const std::string& callId );

    /**
     * @internal
     * Sets the Engine interface delegate
     * 
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface( std::shared_ptr<PhoneCallControllerEngineInterface> phoneCallControllerEngineInterface );

private:
    std::shared_ptr<PhoneCallControllerEngineInterface> m_phoneCallControllerEngineInterface;
};

} // aace::phoneCallController
} // aace

#endif
