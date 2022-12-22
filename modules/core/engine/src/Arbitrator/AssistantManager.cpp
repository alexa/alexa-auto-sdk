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

#include <nlohmann/json.hpp>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Arbitrator/AssistantManager.h>

using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.arbitrator.AssistantManager");

/// String constants in configuration
static const std::string ARBITRATOR = "Arbitrator";
static const std::string WAKEWORD_INTERRUPTION = "WakewordInterruption";
static const std::string GESTURE_INTERRUPTION = "GestureInterruption";
static const std::string ACTIVE_AGENT_ONLY = "ACTIVE_AGENT_ONLY";
static const std::string ANY_AGENT = "ANY_AGENT";

namespace aace {
namespace engine {
namespace arbitrator {

AssistantManager::AssistantManager(
    const SystemInterruptionRule wakewordInterruption,
    const SystemInterruptionRule gestureInterruption) :
        m_wakewordInterruption(wakewordInterruption), m_gestureInterruption(gestureInterruption) {
}

std::shared_ptr<AssistantManager> AssistantManager::create(const std::string& arbitratorConfig) {
    AACE_INFO(LX(TAG));
    try {

        //if empty config default to most restrictive rule system wide 
        SystemInterruptionRule wakewordInterruptionRule;
        SystemInterruptionRule gestureInterruptionRule;
	 
        if(!arbitratorConfig.empty()){
            // Parse aace.arbitrator config
            auto arbitratorConfigJson = json::parse(arbitratorConfig);
            ThrowIfNot(arbitratorConfigJson.contains(ARBITRATOR), "configForArbitratorNotFound");
            auto& arbitrator = arbitratorConfigJson.at(ARBITRATOR);
            
            // Get WakewordInterruption setting
            ThrowIfNot(
                arbitrator.contains(WAKEWORD_INTERRUPTION) && arbitrator[WAKEWORD_INTERRUPTION].is_string(),
                "invalidWakewordInterruption");
            auto wakewordInterruption = arbitrator[WAKEWORD_INTERRUPTION];
            
            if (wakewordInterruption == ACTIVE_AGENT_ONLY) {
                wakewordInterruptionRule = SystemInterruptionRule::ACTIVE_AGENT_ONLY;
            } else if (wakewordInterruption == ANY_AGENT) {
                wakewordInterruptionRule = SystemInterruptionRule::ANY_AGENT;
            } else {
                Throw("invalidWakeWordInterruptionRule");
            }
            
            // Get GestureInterruption setting
            ThrowIfNot(
                arbitrator.contains(GESTURE_INTERRUPTION) && arbitrator[GESTURE_INTERRUPTION].is_string(),
                "invalidGestureInterruption");
            auto gestureInterruption = arbitrator[GESTURE_INTERRUPTION];
            
            if (gestureInterruption == ACTIVE_AGENT_ONLY) {
                gestureInterruptionRule = SystemInterruptionRule::ACTIVE_AGENT_ONLY;
            } else if (gestureInterruption == ANY_AGENT) {
                gestureInterruptionRule = SystemInterruptionRule::ANY_AGENT;
            } else {
                Throw("invalidGestureInterruptionRule");
            }
        } else {
            AACE_WARN(LX(TAG).m("emptyArbitratorConfig use default rules"));
            wakewordInterruptionRule = SystemInterruptionRule::ACTIVE_AGENT_ONLY;
            gestureInterruptionRule = SystemInterruptionRule::ACTIVE_AGENT_ONLY;
        }
        auto assistantManager =
            std::shared_ptr<AssistantManager>(new AssistantManager(wakewordInterruptionRule, gestureInterruptionRule));

        ThrowIfNull(assistantManager, "invalidAssistantManager");

        return assistantManager;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AssistantManager::setListener(std::shared_ptr<AssistantManagerListenerInterface> listener) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(m_listener, "listenerAlreadySet");
        m_listener = listener;
    } catch (std::exception& ex) {
        AACE_ERROR(LX("aace.engine.arbitrator.AssistantManager").d("reason", ex.what()));
    }
}


void AssistantManager::removeListener(std::shared_ptr<AssistantManagerListenerInterface> listener) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(listener, "nullListener");
        ThrowIfNot(m_listener == listener, "invalidListenerReceived");
        m_listener.reset();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }

}
bool AssistantManager::registerAgent(
    const std::string& assistantId,
    const std::string& name,
    const std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule>& dialogStateRules) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(assistantId.empty(), "invalidAssistantId");
        ThrowIf(name.empty(), "invalidName");
        std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
        ThrowIf(m_registeredAgents.find(assistantId) != m_registeredAgents.end(), "assistantAlreadyRegistered");
        auto assisant = Assistant::create(assistantId, name, dialogStateRules);
        m_registeredAgents[assistantId] = assisant;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AssistantManager::deregisterAgent(const std::string& assistantId) {
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
        auto it = m_registeredAgents.find(assistantId);
        ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
        auto assistant = it->second;
        ThrowIf(assistant->isActiveAssistant(), "assistantIsActive");
        m_registeredAgents.erase(it);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AssistantManager::isAgentRegistered(const std::string& assistantId){
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
        auto it = m_registeredAgents.find(assistantId);
	
        if(it == m_registeredAgents.end())
            return false;
	else
	    return true;   
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}
	
std::pair<std::string, std::string> AssistantManager::startDialog(
    const std::string& assistantId,
    aace::arbitrator::ArbitratorEngineInterface::Mode mode) {
    AACE_DEBUG(LX(TAG));
    try {
        // check if any active assistant
        bool foundActiveAssistant = false;
        std::unordered_map<std::string, std::shared_ptr<Assistant>>::iterator it;
        std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
        for (it = m_registeredAgents.begin(); it != m_registeredAgents.end(); it++) {
            auto assistant = it->second;
            if (assistant->isActiveAssistant()) {
                foundActiveAssistant = true;
                break;
            }
        }
        if (!foundActiveAssistant) {
            // no active assistant, start dialog for requesting assistant
            auto it = m_registeredAgents.find(assistantId);

	     //check for agent registered or not
           ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
            auto assistant = it->second;
            auto dialogId = assistant->startDialog();
            auto assistantName = assistant->getAssistantName();
            auto assistantState = assistant->getAssistantState();
            
            // update agent state
            m_listener->updateAgentState(assistantId, assistantName, assistantState);
            return std::make_pair(dialogId, std::string());
        } else {
            switch (mode) {
                case aace::arbitrator::ArbitratorEngineInterface::Mode::WAKEWORD:
                    if (m_wakewordInterruption == SystemInterruptionRule::ACTIVE_AGENT_ONLY) {
                        return handleActiveAgentOnlyInterruption(assistantId, true);
                    } else if (m_wakewordInterruption == SystemInterruptionRule::ANY_AGENT) {
                        return handleAnyAgentInterruption(assistantId, true);
                    }
                    break;
                case aace::arbitrator::ArbitratorEngineInterface::Mode::GESTURE:
                    if (m_gestureInterruption == SystemInterruptionRule::ACTIVE_AGENT_ONLY) {
                        return handleActiveAgentOnlyInterruption(assistantId, false);
                    } else if (m_gestureInterruption == SystemInterruptionRule::ANY_AGENT) {
                        return handleAnyAgentInterruption(assistantId, false);
                    }
                    break;
                default:
                    return std::make_pair(std::string(), std::string());
                    break;
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(std::string(), std::string());
    }
    return std::make_pair(std::string(), std::string());
}

void AssistantManager::stopDialog(const std::string& assistantId, const std::string& dialogId) {
    std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
    auto it = m_registeredAgents.find(assistantId);
    ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
    auto assistant = it->second;
    assistant->stopDialog(dialogId);
    // update agent state
    m_listener->updateAgentState(assistantId, assistant->getAssistantName(), assistant->getAssistantState());
}

void AssistantManager::setDialogState(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& dialogState) {
    std::lock_guard<std::mutex> lock(m_registeredAgentsMutex);
    auto it = m_registeredAgents.find(assistantId);
    ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
    auto assistant = it->second;
    assistant->setDialogState(dialogId, dialogState);
}

std::pair<std::string, std::string> AssistantManager::handleActiveAgentOnlyInterruption(
    const std::string& assistantId,
    bool isWakewordInterruption) {
    auto it = m_registeredAgents.find(assistantId);
    ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
    auto assistant = it->second;
    auto interruptionConditionMet = isWakewordInterruption
                                        ? assistant->isActiveAssistant() && assistant->isBargeInAllowed()
                                        : assistant->isActiveAssistant();
    if (interruptionConditionMet) {
        // terminate old dialog
        auto oldDialogId = assistant->getDialogId();
        assistant->stopDialog(oldDialogId);
        // inform dialog terminated
        m_listener->terminateDialog(assistantId, oldDialogId, {});

        // start new dialog
        auto newDialogId = assistant->startDialog();
        auto assistantName = assistant->getAssistantName();
        auto assistantState = assistant->getAssistantState();
        // update agent state
        m_listener->updateAgentState(assistantId, assistantName, assistantState);
        return std::make_pair(newDialogId, std::string());
    } else {
        // deny dialog
        std::string reason = "Barge in not allowed";
        // m_listener->dialogDenied(assistantId, reason, token);
        return std::make_pair(std::string(), reason);
    }
}

std::pair<std::string, std::string> AssistantManager::handleAnyAgentInterruption(
    const std::string& assistantId,
    bool isWakewordInterruption) {
    // terminate active agent dialog
    std::string activeAssistantId;
    std::shared_ptr<Assistant> activeAssistant;
    std::unordered_map<std::string, std::shared_ptr<Assistant>>::iterator itr;
    for (itr = m_registeredAgents.begin(); itr != m_registeredAgents.end(); itr++) {
        auto assistant = itr->second;
        if (assistant->isActiveAssistant()) {
            activeAssistantId = itr->first;
            activeAssistant = itr->second;
            break;
        }
    }

    auto interruptionConditionMet =
        isWakewordInterruption ? activeAssistant && activeAssistant->isBargeInAllowed() : true;
    if (interruptionConditionMet) {
        if (activeAssistant) {
            auto activeDialogId = activeAssistant->getDialogId();
            activeAssistant->stopDialog(activeDialogId);
            auto activeAssistantName = activeAssistant->getAssistantName();
            auto activeAssistantState = activeAssistant->getAssistantState();
            // inform dialog terminated
            m_listener->terminateDialog(activeAssistantId, activeDialogId, {});
            // update agent state
            m_listener->updateAgentState(activeAssistantId, activeAssistantName, activeAssistantState);
        }

        // start requesting agent dialog
        auto it = m_registeredAgents.find(assistantId);
        ThrowIf(it == m_registeredAgents.end(), "assistantNotFound");
        auto assistant = it->second;
        auto newDialogId = assistant->startDialog();
        auto assistantName = assistant->getAssistantName();
        auto assistantState = assistant->getAssistantState();
        // update agent state
        m_listener->updateAgentState(assistantId, assistantName, assistantState);
        return std::make_pair(newDialogId, std::string());
    } else {
        // deny dialog
        std::string reason = "Barge in not allowed";
        // m_listener->dialogDenied(assistantId, reason, token);
        return std::make_pair(std::string(), reason);
    }
}

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace
