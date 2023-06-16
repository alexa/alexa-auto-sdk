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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Arbitrator/ArbitratorEngineImpl.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.arbitrator.ArbitratorEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "ArbitratorEngineImpl";

/// Counter metrics for Arbitrator Platform APIs
static const std::string METRIC_ARBITRATOR_REGISTER_AGENT = "RegisterAgent";
static const std::string METRIC_ARBITRATOR_DEREGISTER_AGENT = "DeregisterAgent";
static const std::string METRIC_ARBITRATOR_START_DIALOG = "StartDialog";

namespace aace {
namespace engine {
namespace arbitrator {

ArbitratorEngineImpl::ArbitratorEngineImpl(std::shared_ptr<aace::arbitrator::Arbitrator> arbitratorPlatformInterface) :
        m_arbitratorPlatformInterface(arbitratorPlatformInterface) {
}

std::shared_ptr<ArbitratorEngineImpl> ArbitratorEngineImpl::create(
    std::shared_ptr<aace::arbitrator::Arbitrator> arbitratorPlatformInterface,
    const std::string& arbitratorConfig) {
    try {
        ThrowIfNull(arbitratorPlatformInterface, "invalidArbitratorPlatformInterface");

        auto arbitratorEngineImpl =
            std::shared_ptr<ArbitratorEngineImpl>(new ArbitratorEngineImpl(arbitratorPlatformInterface));

        // set the platform engine interface reference
        arbitratorPlatformInterface->setEngineInterface(arbitratorEngineImpl);

        // initialize AssistantManager
        arbitratorEngineImpl->initializeAssistantManager(arbitratorConfig);

        return arbitratorEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void ArbitratorEngineImpl::initializeAssistantManager(const std::string& arbitratorConfig) {
    m_assistantManager = AssistantManager::create(arbitratorConfig);
    ThrowIfNull(m_assistantManager, "createAssistantManagerFailed");
    m_assistantManager->setListener(shared_from_this());
}

bool ArbitratorEngineImpl::onRegisterAgent(
    const std::string& assistantId,
    const std::string& name,
    const std::vector<DialogStateRule>& dialogStateRules) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId).d("name", name));
    try {
        ThrowIf(assistantId.empty(), "invalidAssistantId");
        ThrowIf(name.empty(), "invalidName");

        ThrowIfNot(m_assistantManager->registerAgent(assistantId, name, dialogStateRules), "registerAgentFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ArbitratorEngineImpl::onDeregisterAgent(const std::string& assistantId) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId));
    try {
        ThrowIf(assistantId.empty(), "invalidAssistantId");

        ThrowIfNot(m_assistantManager->deregisterAgent(assistantId), "deregisterAgentFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void ArbitratorEngineImpl::onStartDialog(const std::string& assistantId, Mode mode, const std::string& token) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId).d("mode", convertModeToString(mode)));
    try {
        ThrowIf(assistantId.empty(), "assistantId");
        std::string reason = "Agent Not Registered";
        std::string dialogId = "";

        if (m_assistantManager->isAgentRegistered(assistantId)) {
            auto startDialogResult = m_assistantManager->startDialog(assistantId, mode);
            dialogId = startDialogResult.first;
            reason = startDialogResult.second;
        }
        ThrowIfNull(m_arbitratorPlatformInterface, "nullArbitratorPlatformInterface");
        // send start dialog reply to platform
        m_arbitratorPlatformInterface->startDialogReply(assistantId, dialogId, reason, token);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ArbitratorEngineImpl::onStopDialog(const std::string& assistantId, const std::string& dialogId) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId).d("dialogId", dialogId));
    try {
        ThrowIf(assistantId.empty(), "assistantId");
        ThrowIf(dialogId.empty(), "dialogId");

        m_assistantManager->stopDialog(assistantId, dialogId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ArbitratorEngineImpl::onSetDialogState(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& state) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId).d("dialogId", dialogId).d("state", state));
    try {
        ThrowIf(assistantId.empty(), "assistantId");
        ThrowIf(dialogId.empty(), "dialogId");
        ThrowIf(state.empty(), "state");

        //check for valid dialogId and registered assistant
        ThrowIfNot(m_assistantManager->isAgentRegistered(assistantId), "Agent Not Registered");

        m_assistantManager->setDialogState(assistantId, dialogId, state);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ArbitratorEngineImpl::doShutDown() {
    AACE_INFO(LX(TAG));

    if (m_assistantManager != nullptr) {
        m_assistantManager->removeListener(shared_from_this());
        m_assistantManager.reset();
    }

    if (m_arbitratorPlatformInterface != nullptr) {
        m_arbitratorPlatformInterface->setEngineInterface(nullptr);
        m_arbitratorPlatformInterface.reset();
    }
}

void ArbitratorEngineImpl::addObserver(std::shared_ptr<ArbitratorObserverInterface> arbitratorObserver) {
    m_observers.push_back(arbitratorObserver);
}

void ArbitratorEngineImpl::removeObserver(std::shared_ptr<ArbitratorObserverInterface> arbitratorObserver) {
    for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
        if (it->lock() == arbitratorObserver) {
            m_observers.erase(it);
            return;
        }
    }
}

std::pair<std::string, std::string> ArbitratorEngineImpl::onStartDialog(
    const std::string& assistantId,
    const std::string& mode) {
    AACE_DEBUG(LX(TAG).d("assistantId", assistantId).d("mode", mode));
    try {
        ThrowIf(assistantId.empty(), "assistantId");
        if (m_assistantManager->isAgentRegistered(assistantId)) {
            return m_assistantManager->startDialog(assistantId, convertStringToMode(mode));
        } else {
            std::string reason = "Agent Not Registered";
            std::string dialogId = "";

            return std::make_pair(dialogId, reason);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(std::string(), std::string());
    }
}

void ArbitratorEngineImpl::terminateDialog(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& reason) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_arbitratorPlatformInterface, "nullArbitratorPlatformInterface");
        m_arbitratorPlatformInterface->onDialogTerminated(assistantId, dialogId, reason);

        // notify observers
        for (const auto& arbitratorObserver : m_observers) {
            if (auto observer = arbitratorObserver.lock()) {
                observer->onDialogTerminated(assistantId, dialogId, reason);
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ArbitratorEngineImpl::updateAgentState(const std::string& assistantId, const std::string& name, AgentState state) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_arbitratorPlatformInterface, "nullArbitratorPlatformInterface");
        m_arbitratorPlatformInterface->onAgentStateUpdated(assistantId, name, state);

        // notify observers
        for (const auto& arbitratorObserver : m_observers) {
            if (auto observer = arbitratorObserver.lock()) {
                observer->onAgentStateUpdated(assistantId, name, state);
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace
