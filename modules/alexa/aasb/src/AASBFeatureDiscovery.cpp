/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Alexa/AASBFeatureDiscovery.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/FeatureDiscovery/GetFeaturesMessage.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBFeatureDiscovery");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBFeatureDiscovery> AASBFeatureDiscovery::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto featureDiscovery = std::shared_ptr<AASBFeatureDiscovery>(new AASBFeatureDiscovery());
        ThrowIfNot(featureDiscovery->initialize(messageBroker), "initializeFailed");

        return featureDiscovery;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBFeatureDiscovery::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    m_messageBroker = messageBroker;

    // create a wp reference
    std::weak_ptr<AASBFeatureDiscovery> wp = shared_from_this();

    messageBroker->subscribe(
        aasb::message::alexa::featureDiscovery::GetFeaturesMessage::topic(),
        aasb::message::alexa::featureDiscovery::GetFeaturesMessage::action(),
        [wp](const Message& message) {
            AACE_VERBOSE(LX(TAG));
            try {
                auto sp = wp.lock();
                ThrowIfNull(sp, "invalidWeakPtrReference");
                aasb::message::alexa::featureDiscovery::GetFeaturesMessage::Payload payload =
                    nlohmann::json::parse(message.payload());
                sp->getFeatures(message.messageId(), payload.discoveryRequests);
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
            }
        });

    return true;
}

void AASBFeatureDiscovery::featuresReceived(const std::string& requestId, const std::string& discoveredFeatures) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::featureDiscovery::GetFeaturesMessageReply message;
        message.header.messageDescription.replyToId = requestId;
        message.payload.discoveryResponses = discoveredFeatures;
        messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
