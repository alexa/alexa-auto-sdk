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


#ifndef AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_H
#define AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_H

#include <nlohmann/json.hpp>
#include <string>

#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

#include "ArbitratorObserverInterface.h"
#include "Assistant.h"
#include "AssistantManagerListenerInterface.h"

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace arbitrator {

/**
 * Enum for system level interruption rule for wake word and gesture.
 */
enum class SystemInterruptionRule {
    /// Active agent interruption.
    ACTIVE_AGENT_ONLY,

    /// Any agent interruption.
    ANY_AGENT
};

/**
 * Responsible for managing registration/deregistration of assistants/agents and their dialog states
 */
class AssistantManager {
public:
    /**
     * Create an @c AssistantManager object with the system level rules specified in JSON configuration.
     * The provided JSON is expected to follow the following format.
     * @code
     * "aace.arbitrator": {
     *  "Arbitrator": {
     *      "WakewordInterruption": "ACTIVE_AGENT_ONLY"  #Possible values are "ACTIVE_AGENT_ONLY", "ANY_AGENT"
     *      "GestureInterruption": "ANY_AGENT"  #Possible values are "ACTIVE_AGENT_ONLY", "ANY_AGENT"
     *   }
     *  }
     * @endcode
     *
     * @param arbitratorConfig The system-level interruption rules configured for "aace.arbitrator" to be used
     *          across all agents.
     * @return A pointer to a new @c AssistantManager if arguments are valid, otherwise @c nullptr.
     */
    static std::shared_ptr<AssistantManager> create(const std::string& arbitratorConfig);

    /**
     * AssistantManager destructor
     */
    virtual ~AssistantManager() = default;

    /**
     * Set the listener reference in @c AssistantManager implementation.
     * 
     * @param listener The reference to the @c AssistantManagerListenerInterface
     */
    void setListener(std::shared_ptr<AssistantManagerListenerInterface> listener);

    /**
     * Remove the listener reference in @c AssistantManager implementation.
     * 
     * @param listener The reference to the @c AssistantManagerListenerInterface
     */

    void removeListener(std::shared_ptr<AssistantManagerListenerInterface> listener);

    /**
     * Register an agent
     *
     * @param assistantId The id of agent to register
     * @param name The name of agent to register
     * @param dialogStateRules The barge in rules for the agent
     * @return @c true on success, and @c false on failure.
     */
    bool registerAgent(
        const std::string& assistantId,
        const std::string& name,
        const std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule>& dialogStateRules);

    /**
     * Deregister an agent
     *
     * @param assistantId The id of agent to deregister
     * @return @c true on success, and @c false on failure.
     */
    bool deregisterAgent(const std::string& assistantId);



    /**
     * Check if an agent is registered
     *
     * @param assistantId The id of agent to deregister
     * @return @c true on if registered, and @c false on not registered.

     */
    bool isAgentRegistered(const std::string& assistantId);

    /**
     * Start dialog for an agent.
     *
     * @param assistantId The id of agent for whom the dialog is started
     * @param mode The mode of trigger for dialog - one of Wakeword, Gesture (for PTT/TTT)
     * @param token The identifier for this call to correlate with the reply
     * @return pair of strings, first is the dialogId (if dialog is started, empty otherwise)
     *         second is reason string (if dialog is denied, empty otherwise)  
     */
	std::pair<std::string, std::string> startDialog(
        const std::string& assistantId,
        aace::arbitrator::ArbitratorEngineInterface::Mode mode);

    /**
     * Stop the dialog
     *
     * @param assistantId The id of agent whose dialog should be stopped
     * @param dialogId The id of the dialog that should be stopped
     */
    void stopDialog(const std::string& assistantId, const std::string& dialogId);

    /**
     * Set the dialog state
     *
     * @param assistantId The id of agent whose dialog state should be set
     * @param dialogId The id of the dialog whose state should be set
     * @param dialogState The new state of the dialog
     */
    void setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& dialogState);

private:
    /**
     * AssistantManager constructor
     *
     * @param endpointId The configured ID for this endpoint
     * @param assetIds A complete list of asset IDs used to identify this endpoint
     */
    AssistantManager(
        const SystemInterruptionRule wakewordInterruption,
        const SystemInterruptionRule gestureInterruption);

    /**
     * Method to get active assistant
     *
     * @return active assistant from the list of registered agents
     */
    Assistant getActiveAssistant();

    /// Helper methods to handle interruptions
    std::pair<std::string, std::string> handleActiveAgentOnlyInterruption(
        const std::string& assistantId,
        bool isWakewordInterruption);

    std::pair<std::string, std::string> handleAnyAgentInterruption(
        const std::string& assistantId,
        bool isWakewordInterruption);

    /// The system setting for wakeword interruption
    SystemInterruptionRule m_wakewordInterruption;

    /// The system setting for gesture interruption
    SystemInterruptionRule m_gestureInterruption;

    /// A map of registered assistants managed by the assistant manager, keyed by assistant id
    std::unordered_map<std::string, std::shared_ptr<Assistant>> m_registeredAgents;

    /// To serialize the access to @c m_registeredAgents
    std::mutex m_registeredAgentsMutex;

    /// Reference to the @c AssistantManagerListenerInterface for callbacks.
    std::shared_ptr<AssistantManagerListenerInterface> m_listener;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_H
