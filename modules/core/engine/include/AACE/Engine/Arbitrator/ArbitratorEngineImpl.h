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

#ifndef AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_IMPL_H
#define AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_IMPL_H

#include <memory>

#include <AACE/Arbitrator/Arbitrator.h>
#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

#include "ArbitratorObserverInterface.h"
#include "ArbitratorServiceInterface.h"
#include "AssistantManager.h"
#include "AssistantManagerListenerInterface.h"

namespace aace {
namespace engine {
namespace arbitrator {

class ArbitratorEngineImpl
        : public aace::arbitrator::ArbitratorEngineInterface
        , public AssistantManagerListenerInterface
        , public std::enable_shared_from_this<ArbitratorEngineImpl> {
private:
    ArbitratorEngineImpl(std::shared_ptr<aace::arbitrator::Arbitrator> arbitratorPlatformInterface);

public:
    static std::shared_ptr<ArbitratorEngineImpl> create(
        std::shared_ptr<aace::arbitrator::Arbitrator> arbitratorPlatformInterface,
        const std::string& arbitratorConfig);

    /// @name ArbitratorEngineInterface
    /// @{
    bool onRegisterAgent(
        const std::string& assistantId,
        const std::string& name,
        const std::vector<DialogStateRule>& dialogStateRules) override;
    bool onDeregisterAgent(const std::string& assistantId) override;
    void onStartDialog(const std::string& assistantId, Mode mode, const std::string& token) override;
    void onStopDialog(const std::string& assistantId, const std::string& dialogId) override;
    void onSetDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state)
        override;
    /// @}

    /// @name AssistantManagerListenerInterface
    /// @{
    void terminateDialog(const std::string& assistantId, const std::string& dialogId, const std::string& reason)
        override;
    void updateAgentState(
        const std::string& assistantId,
        const std::string& name,
        aace::arbitrator::ArbitratorEngineInterface::AgentState state) override;
    /// @}

    void doShutDown();

    void addObserver(std::shared_ptr<ArbitratorObserverInterface> arbitratorObserver);
    void removeObserver(std::shared_ptr<ArbitratorObserverInterface> arbitratorObserver);

    // API for internal assistant through service interface
    std::pair<std::string, std::string> onStartDialog(const std::string& assistantId, const std::string& mode);

private:
    void initializeAssistantManager(const std::string& arbitratorConfig);

    /// Arbitrator platform interface handler reference
    std::shared_ptr<aace::arbitrator::Arbitrator> m_arbitratorPlatformInterface;

    /// The list of registered arbitrator observers
    std::vector<std::weak_ptr<ArbitratorObserverInterface>> m_observers;

    /// The reference to the assistant manager
    std::shared_ptr<AssistantManager> m_assistantManager;

    static inline std::string convertModeToString(Mode mode) {
        switch (mode) {
            case Mode::WAKEWORD:
                return "WAKEWORD";
                break;
            case Mode::GESTURE:
                return "GESTURE";
                break;
            default:
                throw("Unknown Mode.");
        }
    }

    static inline Mode convertStringToMode(std::string modeStr) {
        if (modeStr == "WAKEWORD") {
            return Mode::WAKEWORD;
        } else if (modeStr == "GESTURE") {
            return Mode::GESTURE;
        } else {
            throw("Unknown Mode string");
        }
    }
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_IMPL_H