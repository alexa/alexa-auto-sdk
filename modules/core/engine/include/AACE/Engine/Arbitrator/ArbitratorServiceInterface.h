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

#ifndef AACE_ENGINE_ARBITRATOR_ARBITRATOR_SERVICE_INTERFACE_H
#define AACE_ENGINE_ARBITRATOR_ARBITRATOR_SERVICE_INTERFACE_H

#include <string>
#include <map>
#include <memory>
#include <mutex>

#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

#include "ArbitratorObserverInterface.h"

namespace aace {
namespace engine {
namespace arbitrator {

class ArbitratorServiceInterface {
public:
    virtual ~ArbitratorServiceInterface() = default;

    /**
     * Register an agent
     *
     * @param assistantId The unique id of agent to register
     * @param name The name of agent to register
     * @param dialogStateRules The barge in rules for the agent
     * @return @c true on success, and @c false on failure.
     */
    virtual bool registerAgent(
        const std::string& assistantId,
        const std::string& name,
        std::map<std::string, bool> dialogStateRules) = 0;

    /**
     * Deregister an agent
     *
     * @param assistantId The unique id of agent to deregister
     * @return @c true on success, and @c false on failure.
     */
    virtual bool deregisterAgent(const std::string& assistantId) = 0;

    /**
     * Start dialog for an agent
     *
     * @param [in] assistantId The id of agent for whom the dialog is started
     * @param [in] mode The mode of trigger for dialog - one of Wakeword, Gesture (for PTT/TTT)
     * @param [out] dialogId The dialog Id if the requested dialog is started. 
     * @param [out] denyReason The reason if the requested dialog is denied. 
     * @return @c true on success, and @c false on failure.
     */
    virtual bool startDialog(
        const std::string& assistantId,
        const std::string& mode,
        std::string& dialogId,
        std::string& denyReason) = 0;

    /**
     * Stop the dialog for an agent
     *
     * @param assistantId The id of agent whose dialog should be stopped
     * @param dialogId The id of the dialog that should be stopped
     */
    virtual void stopDialog(const std::string& assistantId, const std::string& dialogId) = 0;

    /**
     * Set the dialog state for an agent
     *
     * @param assistantId The id of agent whose dialog state should be set
     * @param dialogId The id of the dialog whose state should be set
     * @param state The new state of the dialog
     */
    virtual void setDialogState(
        const std::string& assistantId,
        const std::string& dialogId,
        const std::string& state) = 0;

    /**
     * Add observer for internal assistant
     * 
     * @param arbitratorObserver The observer for the internal assistant to add
     */
    virtual void addObserver(
        std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) = 0;

    /**
     * Remove observer for internal assistant
     * 
     * @param arbitratorObserver The observer for the internal assistant to remove
     */
    virtual void removeObserver(
        std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) = 0;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ARBITRATOR_SERVICE_INTERFACE_H