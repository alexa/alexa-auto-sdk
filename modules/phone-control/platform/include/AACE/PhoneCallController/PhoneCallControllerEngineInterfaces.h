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

#ifndef AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_INTERFACE_H
#define AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_INTERFACE_H

namespace aace {
namespace phoneCallController {

/**
 * PhoneCallControllerEngineInterface
 */
class PhoneCallControllerEngineInterface {

public:

    /**
     * Specifies the state of connection to the platform calling device
     */
    enum class ConnectionState {
        /**
         * The platform is connected to a calling device
         */
        CONNECTED,

        /**
         * The platform is not connected to a calling device
         */
        DISCONNECTED
    };

    virtual void onConnectionStateChanged( ConnectionState state  ) = 0;
    virtual void onCallActivated( const std::string& callId ) = 0;
    virtual void onCallFailed( const std::string& callId, const std::string& error, const std::string& message ) = 0;
    virtual void onCallTerminated( const std::string& callId ) = 0;
};

} // aace::phoneCallController 
} // aace

#endif
