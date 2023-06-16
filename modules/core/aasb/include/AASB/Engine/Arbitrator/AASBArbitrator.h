/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_ARBITRATOR_AASB_ARBITRATOR_H
#define AASB_ENGINE_ARBITRATOR_AASB_ARBITRATOR_H

#include <AACE/Arbitrator/Arbitrator.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace arbitrator {

class AASBArbitrator
        : public aace::arbitrator::Arbitrator
        , public std::enable_shared_from_this<AASBArbitrator> {
private:
    AASBArbitrator() = default;

    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBArbitrator> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    // aace::arbitrator::Arbitrator
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
    void startDialogReply(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& reason,
        const std::string& token) override;

    /**
     * Notifies platform of update to the agent state
     *
     * @param assistantId The id of agent whose state is updated
     * @param name The name of agent whose state is updated
     * @param state The new state of the agent
     */
    void onAgentStateUpdated(const std::string& assistantId, const std::string& name, AgentState state) override;

    /**
     * Notifies platform to terminate the dialog
     *
     * @param assistantId The id of agent whose dialog is active
     * @param dialogId The id of the dialog that is active
     * @param reason The reason for stopping the dialog
     */
    virtual void onDialogTerminated(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& reason) override;

private:
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_ARBITRATOR_AASB_ARBITRATOR_H
