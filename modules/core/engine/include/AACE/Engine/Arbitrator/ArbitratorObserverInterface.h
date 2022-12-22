/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ARBITRATOR_ARBITRATOR_OBSERVER_INTERFACE_H
#define AACE_ENGINE_ARBITRATOR_ARBITRATOR_OBSERVER_INTERFACE_H

#include <string>
#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

namespace aace {
namespace engine {
namespace arbitrator {

/**
 * This class provides APIs for internal (Alexa) agent to implement and to be notified
 * about dialog termination and state update.
 */
class ArbitratorObserverInterface {
public:
    virtual ~ArbitratorObserverInterface() = default;

    /**
     * Notifies to terminate the dialog. This is called by the 
     * Arbitrator to stop the active dialog of the current agent
     * and to allow another agent to interrupt,
     *
     * @param assistantId The id of agent whose dialog is active
     * @param dialogId The id of the dialog that is active
     * @param reason The reason for stopping the dialog
     */
    virtual void onDialogTerminated(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& reason) = 0;

    /**
     * Notifies of update to the agent state. This is called by the
     * Arbitrator to inform all registered agents that one of the agent
     * has changed state (from ACTIVE to INACTIVE or vice versa)
     *
     * @param assistantId The id of agent whose state is updated
     * @param name The name of agent whose state is updated
     * @param state The new state of the agent
     */
    virtual void onAgentStateUpdated(
        const std::string& assistantId,
        const std::string& name,
        aace::arbitrator::ArbitratorEngineInterface::AgentState state) = 0;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ARBITRATOR_OBSERVER_INTERFACE_H