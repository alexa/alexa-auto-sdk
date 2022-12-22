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

#ifndef AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_LISTENER_INTERFACE_H
#define AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_LISTENER_INTERFACE_H

#include <string>

#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

#include "ArbitratorObserverInterface.h"

namespace aace {
namespace engine {
namespace arbitrator {

class AssistantManagerListenerInterface {
public:
    /**
     * Notifies to terminate the dialog
     *
     * @param assistantId The id of agent whose dialog is active
     * @param dialogId The id of the dialog that is active
     * @param reason The reason for stopping the dialog
     */
    virtual void terminateDialog(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& reason) = 0;

    /**
     * Notifies to update the agent state
     *
     * @param assistantId The id of agent whose state is updated
     * @param name The name of agent whose state is updated
     * @param state The new state of the agent
     */
    virtual void updateAgentState(
        const std::string& assistantId,
        const std::string& name,
        aace::arbitrator::ArbitratorEngineInterface::AgentState state) = 0;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ASSISTANT_MANAGER_LISTENER_INTERFACE_H