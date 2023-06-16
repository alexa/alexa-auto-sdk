/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef AACE_ENGINE_ASSISTANT_INFO_MANAGER_H
#define AACE_ENGINE_ASSISTANT_INFO_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <AVSCommon/AVS/AgentId.h>
#include <AVSCommon/Utils/Optional.h>

namespace aace {
namespace engine {
namespace alexa {

static const std::string ALEXA_ASSISTANT_NAME = "Alexa";

using AssistantIdType = alexaClientSDK::avsCommon::avs::AgentId::IdType;

enum class AssistantPolicy {
    /**
     * Custom assistant (if exists) is powered by Alexa.
     */
    POWERED_BY_ALEXA,

    /**
     * Custom assistant (if exists) is local-only.
     */
    ON_DEVICE,

    /**
     * Alexa only.
     */
    ALEXA_ONLY
};

struct Assistant {
    /**
     * Agent Id of Assistant.
     */
    AssistantIdType id;

    /**
     * Public UUID of Assistant. 
     */
    std::string uuid;

    /**
     * Name of Assistant.
     */
    std::string name;

    /**
     * Whether Assistant is local-only.
     */
    bool isLocal;
};

/**
 * This class provides APIs to allow other modules to add assistant info and 
 * manages the info for all the registered assistants.
 */
class AssistantInfoManager {
public:
    AssistantInfoManager();

    alexaClientSDK::avsCommon::utils::Optional<Assistant> getAssistantInfoById(AssistantIdType id);
    alexaClientSDK::avsCommon::utils::Optional<Assistant> getAssistantInfoByName(const std::string& name);
    std::unordered_map<AssistantIdType, Assistant> getAllAssistantInfo();
    AssistantPolicy getAssistantPolicy();
    void addAssistantInfo(Assistant assistantInfo);
    void setAssistantPolicy(AssistantPolicy policy);

private:
    std::unordered_map<AssistantIdType, Assistant> m_idToAssistants;
    /**
     * Mutex to serialize the access to @c m_idToAssistants and @c m_policy and @c m_registeredAgent.
     */
    std::mutex m_mutex;
    AssistantPolicy m_policy;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_INFO_PROVIDER_H