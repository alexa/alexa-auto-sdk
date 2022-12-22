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

#ifndef AACE_ARBITRATOR_ARBITRATOR_H
#define AACE_ARBITRATOR_ARBITRATOR_H

#include <iostream>
#include <string>

#include <AACE/Core/PlatformInterface.h>

#include "ArbitratorEngineInterface.h"
/** @file */

namespace aace {
namespace arbitrator {

/**
 * Arbitrator should be extended to handle agent arbitration directives from the Engine.
 * 
 * @deprecated This platform interface is deprecated. 
 *             Use the Alexa Auto Services Bridge (AASB) message broker 
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 *
 */
class Arbitrator : public aace::core::PlatformInterface {
protected:
    Arbitrator() = default;

public:
    /// Describes the agent states.
    using AgentState = aace::arbitrator::ArbitratorEngineInterface::AgentState;
    /// Describes the dialog request modes.
    using Mode = aace::arbitrator::ArbitratorEngineInterface::Mode;
    /// Describes the rules for dialog states.
    using DialogStateRule = aace::arbitrator::ArbitratorEngineInterface::DialogStateRule;

    /**
     * Register an agent with the Engine
     *
     * @param assistantId The id of agent to register
     * @param name The name of agent to register
     * @param dialogStateRules The barge in rules for the agent
     * @return true On success
     * @return false On failure
     */
    virtual bool registerAgent(
        const std::string& assistantId,
        const std::string& name,
        const std::vector<DialogStateRule>& dialogStateRules);

    /**
     * Deregister an agent with the Engine
     *
     * @param assistantId The id of agent to deregister
     * @return true On success
     * @return false On failure
     */
    virtual bool deregisterAgent(const std::string& assistantId);

    /**
     * Start dialog for an agent.
     *
     * @param assistantId The id of agent for whom the dialog is started
     * @param mode The mode of trigger for dialog - one of Wakeword, Gesture (for PTT/TTT)
     * @param token An unique identifier to correlate @c startDialogReply to @c startDialog. This will be
     * sent back in @c startDialogReply. 
     */
    virtual void startDialog(const std::string& assistantId, Mode mode, const std::string& token);

    /**
     * Notifies platform if request to start a dialog is granted and the requested dialog is started 
     * or if the request is denied and the reason for denying.
     *
     * @param assistantId The id of agent for whom the dialog is requested
     * @param dialogId The unique identifier generated for the started dialog, populated if the dialog was started, empty otherwise
     * @param reason The reason for the dialog not started, populated if request was denied, empty otherwise. 
     * This field is informational and is used only for logging purpose.
     * @param token The identifier that was provided in @c startDialog call.
     */
    virtual void startDialogReply(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& reason,
        const std::string& token) = 0;

    /**
     * Notifies Engine to stop the dialog
     *
     * @param assistantId The id of agent whose dialog should be stopped
     * @param dialogId The id of the dialog that should be stopped
     */
    virtual void stopDialog(const std::string& assistantId, const std::string& dialogId);

    /**
     * Notifies Engine to set the dialog state
     *
     * @param assistantId The id of agent whose dialog state should be set
     * @param dialogId The id of the dialog whose state should be set
     * @param state The new state of the dialog
     */
    virtual void setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state);

    /**
     * Notifies platform that to terminate the dialog
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
     * Notifies platform of update to the agent state
     *
     * @param assistantId The id of agent whose state is updated
     * @param name The name of agent whose state is updated
     * @param state The new state of the agent
     */
    virtual void onAgentStateUpdated(const std::string& assistantId, const std::string& name, AgentState state) = 0;

    /**
     * @internal
     * Sets the Engine interface delegate.
     * 
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<ArbitratorEngineInterface> arbitratorEngineInterface);

    /**
     * Destructor
     */
    virtual ~Arbitrator();

private:
    std::weak_ptr<ArbitratorEngineInterface> m_arbitratorEngineInterface;
};

}  // namespace arbitrator
}  // namespace aace

#endif  // AACE_ARBITRATOR_ARBITRATOR_H