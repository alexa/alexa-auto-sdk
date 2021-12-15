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

#include <AASB/Engine/CustomDomain/AASBCustomDomain.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/CustomDomain/CustomDomain/CancelDirectiveMessage.h>
#include <AASB/Message/CustomDomain/CustomDomain/GetContextMessage.h>
#include <AASB/Message/CustomDomain/CustomDomain/HandleDirectiveMessage.h>
#include <AASB/Message/CustomDomain/CustomDomain/ReportDirectiveHandlingResultMessage.h>
#include <AASB/Message/CustomDomain/CustomDomain/ResultType.h>
#include <AASB/Message/CustomDomain/CustomDomain/SendEventMessage.h>

namespace aasb {
namespace engine {
namespace customDomain {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.customDomain.AASBCustomDomain");

// Aliases
using Message = aace::engine::messageBroker::Message;
using ResultType = aace::customDomain::CustomDomain::ResultType;

bool AASBCustomDomain::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // Create a weak pointer reference
        std::weak_ptr<AASBCustomDomain> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::customDomain::customDomain::ReportDirectiveHandlingResultMessage::topic(),
            aasb::message::customDomain::customDomain::ReportDirectiveHandlingResultMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::customDomain::customDomain::ReportDirectiveHandlingResultMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->reportDirectiveHandlingResult(
                        payload.directiveNamespace, payload.messageId, static_cast<ResultType>(payload.result));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::customDomain::customDomain::SendEventMessage::topic(),
            aasb::message::customDomain::customDomain::SendEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::customDomain::customDomain::SendEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->sendEvent(
                        payload.eventNamespace,
                        payload.eventName,
                        payload.eventPayload,
                        payload.requiresContext,
                        payload.correlationToken,
                        payload.customContext);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AASBCustomDomain> AASBCustomDomain::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBCustomDomain>(new AASBCustomDomain());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBCustomDomainFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

//
// aace::customDomain::CustomDomain
//

void AASBCustomDomain::handleDirective(
    const std::string& directiveNamespace,
    const std::string& directiveName,
    const std::string& directivePayload,
    const std::string& correlationToken,
    const std::string& messageId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::customDomain::customDomain::HandleDirectiveMessage message;
        message.payload.directiveNamespace = directiveNamespace;
        message.payload.directiveName = directiveName;
        message.payload.directivePayload = directivePayload;
        message.payload.correlationToken = correlationToken;
        message.payload.messageId = messageId;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBCustomDomain::cancelDirective(
    const std::string& directiveNamespace,
    const std::string& directiveName,
    const std::string& correlationToken,
    const std::string& messageId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::customDomain::customDomain::CancelDirectiveMessage message;
        message.payload.directiveNamespace = directiveNamespace;
        message.payload.directiveName = directiveName;
        message.payload.correlationToken = correlationToken;
        message.payload.messageId = messageId;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AASBCustomDomain::getContext(const std::string& contextNamespace) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::customDomain::customDomain::GetContextMessage message;
        message.payload.contextNamespace = contextNamespace;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetContextTimeout");
        aasb::message::customDomain::customDomain::GetContextMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.customContext;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

}  // namespace customDomain
}  // namespace engine
}  // namespace aasb