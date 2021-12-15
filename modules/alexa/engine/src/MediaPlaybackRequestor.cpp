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

#include "AACE/Engine/Alexa/MediaPlaybackRequestor.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"
#include <AVSCommon/Utils/JSON/JSONGenerator.h>
#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/AVS/MessageRequest.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.MediaPlaybackRequestor");

/// MediaPlaybackRequestor capability constants
static const std::string PLAYBACK_REQUESTOR_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// MediaResume Interface name.
static const std::string PLAYBACK_REQUESTOR_INTERFACE_NAME = "Alexa.Media.PlaybackRequestor";
/// MediaResume interface version.
static const std::string PLAYBACK_REQUESTOR_INTERFACE_VERSION = "1.0";

//Event Details
static const std::string PLAYBACK_EVENT_NAME = "RequestMediaPlayback";
static const char PLAYBACK_REQUESTOR_INVOCATION_REASON_KEY[] = "invocationReason";
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "MediaPlaybackRequestor";
static const std::string METRIC_TIME_TO_REQUEST_MEDIA_RESUME = "MediaPlaybackRequestor.Latency";

MediaPlaybackRequestor::MediaPlaybackRequestor(
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{PLAYBACK_REQUESTOR_INTERFACE_NAME, exceptionSender},
        m_messageSender(messageSender),
        m_exceptionSender(exceptionSender) {
}

std::shared_ptr<MediaPlaybackRequestor> MediaPlaybackRequestor::createMediaPlaybackRequestor(
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender) {
    try {
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(exceptionSender, "nullExceptionSender");

        auto mediaPlaybackRequestor =
            std::shared_ptr<MediaPlaybackRequestor>(new MediaPlaybackRequestor(messageSender, exceptionSender));

        ThrowIfNull(mediaPlaybackRequestor, "nullMediaPlaybackRequestorCapabilityAgent");

        return mediaPlaybackRequestor;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::unordered_set<std::shared_ptr<CapabilityConfiguration>> MediaPlaybackRequestor::getCapabilityConfigurations() {
    AACE_INFO(LX(TAG));
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, PLAYBACK_REQUESTOR_CAPABILITY_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, PLAYBACK_REQUESTOR_INTERFACE_NAME});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, PLAYBACK_REQUESTOR_INTERFACE_VERSION});

    return {std::make_shared<CapabilityConfiguration>(configMap)};
}

void MediaPlaybackRequestor::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::onDeregistered() {
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration MediaPlaybackRequestor::getConfiguration() const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    return configuration;
}

std::future<bool> MediaPlaybackRequestor::requestMediaPlayback(
    std::string const invocationReason,
    long long elapsedBootTime) {
    AACE_INFO(LX(TAG));
    std::promise<bool> sendMessagePromise;
    std::future<bool> sendMessageFuture = sendMessagePromise.get_future();

    try {
        alexaClientSDK::avsCommon::utils::json::JsonGenerator json;
        json.addMember(PLAYBACK_REQUESTOR_INVOCATION_REASON_KEY, invocationReason);
        std::string payload = json.toString();

        if (payload.empty()) {
            AACE_ERROR(LX(__func__).d("reason", "emptyPayload"));
            sendMessagePromise.set_value(false);
            return sendMessageFuture;
        } else {
            auto msgIdAndJsonEvent = alexaClientSDK::avsCommon::avs::buildJsonEventString(
                PLAYBACK_REQUESTOR_INTERFACE_NAME, PLAYBACK_EVENT_NAME, "", payload, "");

            auto messageRequest =
                std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);
            messageRequest->addObserver(shared_from_this());
            m_sendMessagePromise = std::move(sendMessagePromise);
            m_messageSender->sendMessage(messageRequest);
        }

        emitTimerMetrics(
            METRIC_PROGRAM_NAME_SUFFIX, "mediaResumeRequest", METRIC_TIME_TO_REQUEST_MEDIA_RESUME, elapsedBootTime);

    } catch (std::exception& ex) {
        AACE_ERROR(LX(__func__).d("reason", ex.what()));
        sendMessagePromise.set_value(false);
    }
    return sendMessageFuture;
}

void MediaPlaybackRequestor::onResponseStatusReceived(MessageRequestObserverInterface::Status status) {
    AACE_DEBUG(LX(TAG).d("status", status));
}

void MediaPlaybackRequestor::onSendCompleted(MessageRequestObserverInterface::Status status) {
    AACE_DEBUG(LX(TAG).d("status", status));
    m_sendMessagePromise.set_value(true);
}

void MediaPlaybackRequestor::onExceptionReceived(const std::string& exceptionMessage) {
    AACE_ERROR(LX(TAG).d("exceptionMessage", exceptionMessage));
    m_sendMessagePromise.set_value(false);
}

void MediaPlaybackRequestor::doShutdown() {
    AACE_INFO(LX(TAG));
    m_messageSender.reset();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
