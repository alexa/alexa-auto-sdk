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

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/Alexa/MediaPlaybackRequestorEngineImpl.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.MediaPlaybackRequestorEngineImpl");

static const std::string METRIC_PROGRAM_NAME_SUFFIX = "MediaPlaybackRequestor";
static const std::string METRIC_TIME_TO_IGNORE_MEDIA_RESUME = "MediaPlaybackRequestor.RoadSafetyGuardrail";

MediaPlaybackRequestorEngineImpl::MediaPlaybackRequestorEngineImpl(
    std::shared_ptr<aace::alexa::MediaPlaybackRequestor> mediaPlaybackRequestorPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_mediaPlaybackRequestorPlatformInterface(mediaPlaybackRequestorPlatformInterface),
        m_connectionStatus(Status::DISCONNECTED) {
    m_mediaPlaybackRequestor =
        aace::engine::alexa::MediaPlaybackRequestor::createMediaPlaybackRequestor(messageSender, exceptionSender);
}

bool MediaPlaybackRequestorEngineImpl::initialize(
    long long int mediaResumeThreshold,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar) {
    AACE_INFO(LX(TAG));
    capabilitiesRegistrar->withCapability(m_mediaPlaybackRequestor, m_mediaPlaybackRequestor);
    try {
        m_mediaResumeThreshold = mediaResumeThreshold;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<MediaPlaybackRequestorEngineImpl> MediaPlaybackRequestorEngineImpl::create(
    std::shared_ptr<aace::alexa::MediaPlaybackRequestor> mediaPlaybackRequestorPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    long long int mediaResumeThreshold) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIfNull(mediaPlaybackRequestorPlatformInterface, "invalidPlatformInterface");

        auto mediaPlaybackRequestorEngineImpl =
            std::shared_ptr<MediaPlaybackRequestorEngineImpl>(new MediaPlaybackRequestorEngineImpl(
                mediaPlaybackRequestorPlatformInterface, messageSender, exceptionSender));

        ThrowIfNot(
            mediaPlaybackRequestorEngineImpl->initialize(mediaResumeThreshold, capabilitiesRegistrar),
            "initializeMediaPlaybackRequestorEngineImplFailed");

        // Set the platform engine interface reference.
        mediaPlaybackRequestorPlatformInterface->setEngineInterface(mediaPlaybackRequestorEngineImpl);

        return mediaPlaybackRequestorEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void MediaPlaybackRequestorEngineImpl::onRequestMediaPlayback(
    InvocationReason invocationReason,
    long long int elapsedBootTime) {
    AACE_DEBUG(LX(TAG).d("elapsedBootTime", elapsedBootTime).d("mediaResumeThreshold", m_mediaResumeThreshold));

    auto mediaResumeThreshold = m_mediaResumeThreshold;
    auto mediaPlaybackRequestorPlatformInterface = m_mediaPlaybackRequestorPlatformInterface;
    auto mediaPlaybackRequestor = m_mediaPlaybackRequestor;
    auto reason = invocationReasonToString(invocationReason);
    auto connectionStatus = m_connectionStatus;
    m_executor.submit([reason,
                       elapsedBootTime,
                       mediaResumeThreshold,
                       mediaPlaybackRequestorPlatformInterface,
                       mediaPlaybackRequestor,
                       connectionStatus] {
        try {
            if (elapsedBootTime > mediaResumeThreshold) {
                AACE_WARN(LX(TAG)
                              .d("reason", "TimeoutForMediaResume")
                              .d("elapsedBootTime", elapsedBootTime)
                              .d("mediaResumeThreshold", mediaResumeThreshold));
                mediaPlaybackRequestorPlatformInterface->mediaPlaybackResponse(
                    MediaPlaybackRequestStatus::FAILED_TIMEOUT);
                emitTimerMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX,
                    "mediaResumeRequest",
                    METRIC_TIME_TO_IGNORE_MEDIA_RESUME,
                    elapsedBootTime);
            } else if (connectionStatus != Status::CONNECTED) {
                AACE_WARN(LX(TAG).d("connectionStatus", connectionStatus));
                mediaPlaybackRequestorPlatformInterface->mediaPlaybackResponse(
                    MediaPlaybackRequestStatus::FAILED_CAN_RETRY);
            } else {
                auto resultFuture =
                    mediaPlaybackRequestor->requestMediaPlayback(static_cast<std::string>(reason), elapsedBootTime);
                if ((resultFuture.wait_for(std::chrono::milliseconds(2000)) == std::future_status::ready)) {
                    auto result = resultFuture.get();
                    mediaPlaybackRequestorPlatformInterface->mediaPlaybackResponse(
                        result ? MediaPlaybackRequestStatus::SUCCESS : MediaPlaybackRequestStatus::ERROR);
                }
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    });
}

void MediaPlaybackRequestorEngineImpl::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    m_connectionStatus = status;
}

void MediaPlaybackRequestorEngineImpl::onConnectionStatusChanged(
    const Status status,
    const std::vector<EngineConnectionStatus>& engineStatuses) {
    m_connectionStatus = status;
}

void MediaPlaybackRequestorEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));
    m_mediaPlaybackRequestor->doShutdown();
    if (m_mediaPlaybackRequestorPlatformInterface != nullptr) {
        m_mediaPlaybackRequestorPlatformInterface->setEngineInterface(nullptr);
        m_mediaPlaybackRequestorPlatformInterface.reset();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
