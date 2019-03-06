/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <ACL/AVSConnectionManager.h>

#include "AACE/Alexa/AlexaClient.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaClientEngineImpl :
public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface,
public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface,
public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface {
private:
    AlexaClientEngineImpl( std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface );

public:
    static std::shared_ptr<AlexaClientEngineImpl> create( std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface );

    // AuthObserverInterface
    void onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error ) override;

    // ConnectionStatusObserverInterface
    void onConnectionStatusChanged( const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status, const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason )
        override;

    // DialogUXStateObserverInterface
    void onDialogUXStateChanged( alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState state ) override;

private:
    std::shared_ptr<aace::alexa::AlexaClient> m_alexaClientPlatformInterface = nullptr;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H
