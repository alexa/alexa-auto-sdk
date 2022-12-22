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


#include <typeinfo>

#include <AACE/Engine/Arbitrator/ArbitratorEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace arbitrator {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.arbitrator.ArbitratorEngineService");

// register the service
REGISTER_SERVICE(ArbitratorEngineService)

ArbitratorEngineService::ArbitratorEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool ArbitratorEngineService::configure(std::shared_ptr<std::istream> configuration) {
    AACE_INFO(LX(TAG));
    try {
        json jconfiguration;
        jconfiguration = json::parse(*configuration);

        m_arbitratorConfig = jconfiguration.dump();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ArbitratorEngineService::shutdown() {
    if (m_arbitratorEngineImpl != nullptr) {
        m_arbitratorEngineImpl->doShutDown();
        m_arbitratorEngineImpl.reset();
    }
    return true;
}

bool ArbitratorEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::arbitrator::Arbitrator>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool ArbitratorEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::arbitrator::Arbitrator> arbitrator) {
    AACE_INFO(LX(TAG).m("Registering Arbitrator platform interface"));
    try {
        ThrowIfNotNull(m_arbitratorEngineImpl, "platformInterfaceAlreadyRegistered");
        m_arbitratorEngineImpl = ArbitratorEngineImpl::create(arbitrator, m_arbitratorConfig);

        ThrowIfNot(
            registerServiceInterface<ArbitratorServiceInterface>(shared_from_this()),
            "registerArbitratorServiceInterfaceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Arbitrator>").d("reason", ex.what()));
        return false;
    }
}

bool ArbitratorEngineService::registerAgent(
    const std::string& assistantId,
    const std::string& name,
    std::map<std::string, bool> dialogStateRules) {
    std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule> agentDialogStateRules;
    for (auto dialogRule : dialogStateRules) {
        auto state = dialogRule.first;
        bool wakewordInterruptionAllowed = dialogRule.second;
        aace::arbitrator::ArbitratorEngineInterface::DialogStateRule dialogStateRule =
            aace::arbitrator::ArbitratorEngineInterface::DialogStateRule(state, wakewordInterruptionAllowed);
        agentDialogStateRules.push_back(dialogStateRule);
    }
    if (m_arbitratorEngineImpl) {
        return m_arbitratorEngineImpl->onRegisterAgent(assistantId, name, agentDialogStateRules);
    } else {
        return false;
    }
}

bool ArbitratorEngineService::deregisterAgent(const std::string& assistantId) {
    if (m_arbitratorEngineImpl) {
        return m_arbitratorEngineImpl->onDeregisterAgent(assistantId);
    } else {
        return false;
    }
}

bool ArbitratorEngineService::startDialog(
    const std::string& assistantId,
    const std::string& mode,
    std::string& dialogId,
    std::string& denyReason) {
    if (m_arbitratorEngineImpl) {
        auto startDialogResult = m_arbitratorEngineImpl->onStartDialog(assistantId, mode);
        dialogId = startDialogResult.first;
        denyReason = startDialogResult.second;
        if (!dialogId.empty()) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void ArbitratorEngineService::stopDialog(const std::string& assistantId, const std::string& dialogId) {
    if (m_arbitratorEngineImpl) {
        m_arbitratorEngineImpl->onStopDialog(assistantId, dialogId);
    }
}

void ArbitratorEngineService::setDialogState(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& state) {
    if (m_arbitratorEngineImpl) {
        m_arbitratorEngineImpl->onSetDialogState(assistantId, dialogId, state);
    }
}

void ArbitratorEngineService::addObserver(
    std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) {
    if (m_arbitratorEngineImpl) {
        m_arbitratorEngineImpl->addObserver(arbitratorObserver);
    }
}

void ArbitratorEngineService::removeObserver(
    std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) {
    if (m_arbitratorEngineImpl) {
        m_arbitratorEngineImpl->removeObserver(arbitratorObserver);
    }
}

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace
