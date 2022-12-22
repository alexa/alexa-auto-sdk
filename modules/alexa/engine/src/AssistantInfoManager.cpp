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

#include <iostream>
#include <utility>

#include "AACE/Engine/Alexa/AssistantInfoManager.h"
#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Utils/String/StringUtils.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AssistantInfoManager");

AssistantInfoManager::AssistantInfoManager() {
    // Insert AGENT_ID_NONE (0) and AGENT_ID_ALL (1) into the map to prevent undefined behavior
    m_idToAssistants.insert(std::make_pair(
        alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_NONE,
        Assistant{alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_NONE, "", "", false}));
    m_idToAssistants.insert(std::make_pair(
        alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL,
        Assistant{alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, "", "", false}));

    // Add Alexa by default
    auto assistant = Assistant{.id = ALEXA_ASSISTANT_ID, .uuid = "", .name = ALEXA_ASSISTANT_NAME, .isLocal = false};

    std::lock_guard<std::mutex> lock(m_mutex);
    m_idToAssistants.insert(std::make_pair(ALEXA_ASSISTANT_ID, assistant));

    // Set default policy
    m_policy = AssistantPolicy::ALEXA_ONLY;
}

alexaClientSDK::avsCommon::utils::Optional<Assistant> AssistantInfoManager::getAssistantInfoById(AssistantIdType id) {
    AACE_DEBUG(LX(TAG).d("id", id));
    alexaClientSDK::avsCommon::utils::Optional<Assistant> assistant;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_idToAssistants.find(id) == m_idToAssistants.end()) {
        AACE_ERROR(LX(TAG).d("unknownAssistantId", id));
        return assistant;
    }

    auto assistantInfo = m_idToAssistants.find(id)->second;
    assistant.set(assistantInfo);
    AACE_DEBUG(LX(TAG).m("foundMatchingAssistant").d("id", assistantInfo.id));
    return assistant;
}

alexaClientSDK::avsCommon::utils::Optional<Assistant> AssistantInfoManager::getAssistantInfoByName(
    const std::string& name) {
    AACE_DEBUG(LX(TAG).d("name", name));
    alexaClientSDK::avsCommon::utils::Optional<Assistant> assistant;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto iter = m_idToAssistants.begin(); iter != m_idToAssistants.end(); iter++) {
        if (aace::engine::utils::string::equal(iter->second.name, name, false)) {
            assistant.set(iter->second);
            return assistant;
        }
    }

    AACE_ERROR(LX(TAG).m("unknownAssistantName"));
    return assistant;
}

void AssistantInfoManager::addAssistantInfo(Assistant assistantInfo) {
    AACE_DEBUG(LX(TAG).d("id", assistantInfo.id));
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_idToAssistants.find(assistantInfo.id) != m_idToAssistants.end()) {
        // Update an existing assistant
        AACE_INFO(LX(TAG).m("updatingExistingAssistant"));
        m_idToAssistants.erase(assistantInfo.id);
    }
    m_idToAssistants.insert(std::make_pair(assistantInfo.id, assistantInfo));
}

AssistantPolicy AssistantInfoManager::getAssistantPolicy() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_policy;
}

std::unordered_map<AssistantIdType, Assistant> AssistantInfoManager::getAllAssistantInfo() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_idToAssistants;
}

void AssistantInfoManager::setAssistantPolicy(AssistantPolicy policy) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_policy = policy;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace