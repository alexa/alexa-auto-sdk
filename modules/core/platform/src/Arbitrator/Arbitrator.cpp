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

#include <AACE/Arbitrator/Arbitrator.h>

namespace aace {
namespace arbitrator {

Arbitrator::~Arbitrator() = default;  // key function

class DialogStateRule;

bool Arbitrator::registerAgent(
    const std::string& assistantId,
    const std::string& name,
    const std::vector<DialogStateRule>& dialogStateRules) {
    if (auto arbitratorEngineInterface_lock = m_arbitratorEngineInterface.lock()) {
        return arbitratorEngineInterface_lock->onRegisterAgent(assistantId, name, dialogStateRules);
    } else {
        return false;
    }
}

bool Arbitrator::deregisterAgent(const std::string& assistantId) {
    if (auto arbitratorEngineInterface_lock = m_arbitratorEngineInterface.lock()) {
        return arbitratorEngineInterface_lock->onDeregisterAgent(assistantId);
    } else {
        return false;
    }
}

void Arbitrator::startDialog(const std::string& assistantId, Mode mode, const std::string& token) {
    if (auto arbitratorEngineInterface_lock = m_arbitratorEngineInterface.lock()) {
        arbitratorEngineInterface_lock->onStartDialog(assistantId, mode, token);
    }
}

void Arbitrator::stopDialog(const std::string& assistantId, const std::string& dialogId) {
    if (auto arbitratorEngineInterface_lock = m_arbitratorEngineInterface.lock()) {
        arbitratorEngineInterface_lock->onStopDialog(assistantId, dialogId);
    }
}

void Arbitrator::setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state) {
    if (auto arbitratorEngineInterface_lock = m_arbitratorEngineInterface.lock()) {
        arbitratorEngineInterface_lock->onSetDialogState(assistantId, dialogId, state);
    }
}

void Arbitrator::setEngineInterface(std::shared_ptr<ArbitratorEngineInterface> arbitratorEngineInterface) {
    m_arbitratorEngineInterface = arbitratorEngineInterface;
}

}  // namespace arbitrator
}  // namespace aace
