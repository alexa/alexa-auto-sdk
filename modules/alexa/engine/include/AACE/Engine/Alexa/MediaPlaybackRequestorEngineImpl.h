/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_MEDIAPLAYBACKREQUESTORENGINEIMPL_H_
#define AACE_ENGINE_ALEXA_MEDIAPLAYBACKREQUESTORENGINEIMPL_H_

#include <memory>

#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/MediaPlaybackRequestor.h"

#include "MediaPlaybackRequestor.h"

namespace aace {
namespace engine {
namespace alexa {

class MediaPlaybackRequestorEngineImpl
        : public aace::alexa::MediaPlaybackRequestorEngineInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<MediaPlaybackRequestorEngineImpl> {
private:
    MediaPlaybackRequestorEngineImpl(
        std::shared_ptr<aace::alexa::MediaPlaybackRequestor> mediaPlaybackRequestorPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    bool initialize(
        long long int mediaResumeThreshold,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar);

    using Status = alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status;
    using ChangedReason = alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason;

public:
    static std::shared_ptr<MediaPlaybackRequestorEngineImpl> create(
        std::shared_ptr<aace::alexa::MediaPlaybackRequestor> mediaPlaybackRequestorPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        long long int mediaResumeThreshold);

    // MediaPlaybackRequestorEngineInterface
    void onRequestMediaPlayback(InvocationReason invocationReason, long long int elapsedBootTime) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
    void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;
    void onConnectionStatusChanged(const Status status, const std::vector<EngineConnectionStatus>& engineStatuses)
        override;

protected:
    void doShutdown() override;

private:
    /// Auto SDK Alexa MediaPlaybackRequestor platform interface handler instance.
    std::shared_ptr<aace::alexa::MediaPlaybackRequestor> m_mediaPlaybackRequestorPlatformInterface;
    std::shared_ptr<aace::engine::alexa::MediaPlaybackRequestor> m_mediaPlaybackRequestor;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// Config values.
    long long int m_mediaResumeThreshold;

    Status m_connectionStatus;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_MEDIAPLAYBACKREQUESTORENGINEIMPL_H_
