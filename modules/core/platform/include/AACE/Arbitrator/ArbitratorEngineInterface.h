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


#ifndef AACE_ARBITRATOR_ARBITRATOR_ENGINE_INTERFACE_H
#define AACE_ARBITRATOR_ARBITRATOR_ENGINE_INTERFACE_H

/** @file */

#include <iostream>
#include <vector>

namespace aace {
namespace arbitrator {

/**
 * The class to interface with the Arbitrator Engine.
 */
class ArbitratorEngineInterface {
public:
    virtual ~ArbitratorEngineInterface() = default;

    /// Describes the agent states.
    enum class AgentState {
        /// Agent is active with an on-going dialog
        ACTIVE,

        /// Agent is inactive
        INACTIVE,
    };

    /// Describes the dialog request modes.
    enum class Mode {
        /// Wakeword
        WAKEWORD,

        /// Gesture - e.g., PTT/TTT
        GESTURE,
    };

    /// Describes the rules for the dialog state
    struct DialogStateRule {
        DialogStateRule(std::string& state, bool wakewordInterruptionAllowed) :
                state{state}, wakewordInterruptionAllowed{wakewordInterruptionAllowed} {
        }

	 std::string state;
        bool wakewordInterruptionAllowed;
    };

    /**
     * Register an agent with the Engine
     *
     * @param assistantId The id of agent to register
     * @param name The name of agent to register
     * @param dialogStateRules The barge in rules for the agent
     * @return true On success
     * @return false On failure
     */
    virtual bool onRegisterAgent(
        const std::string& assistantId,
        const std::string& name,
        const std::vector<DialogStateRule>& dialogStateRules) = 0;

    /**
     * Deregister an agent with the Engine
     *
     * @param assistantId The id of agent to deregister
     * @return true On success
     * @return false On failure
     */
    virtual bool onDeregisterAgent(const std::string& assistantId) = 0;

    /**
     * Notifies Engine to start dialog for an agent.
     *
     * @param assistantId The id of agent for whom the dialog is started
     * @param mode The mode of trigger for dialog - one of Wakeword, Gesture (for PTT/TTT)
     * @param token An unique identifier to correlate @c startDialogReply to @c startDialog. This will be
     * sent back in @c startDialogReply.
     */
    virtual void onStartDialog(const std::string& assistantId, Mode mode, const std::string& token) = 0;

    /**
     * Notifies Engine to stop the dialog
     *
     * @param assistantId The id of agent whose dialog should be stopped
     * @param dialogId The id of the dialog that should be stopped
     */
    virtual void onStopDialog(const std::string& assistantId, const std::string& dialogId) = 0;

    /**
     * Notifies Engine to set the dialog state
     *
     * @param assistantId The id of agent whose dialog state should be set
     * @param dialogId The id of the dialog whose state should be set
     * @param state The new state of the dialog
     */
    virtual void onSetDialogState(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& state) = 0;
};

}  // namespace arbitrator
}  // namespace aace

#endif  // AACE_ARBITRATOR_ARBITRATOR_ENGINE_INTERFACE_H
