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
#include <AACE/Engine/Arbitrator/Assistant.h>
#include <AACE/Engine/Utils/UUID/UUID.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.arbitrator.Assistant");

/// String constant for dialog state
static const std::string INITIAL_DIALOG_STATE = "INIT";

namespace aace {
namespace engine {
namespace arbitrator {

Assistant::Assistant(
    const std::string& assistantId,
    const std::string& assistantName,
    std::unordered_map<std::string, bool> bargeInRules) :
        m_assistantId(assistantId),
        m_assistantName(assistantName),
        m_bargeInRules(bargeInRules),
        m_assistantState(aace::arbitrator::ArbitratorEngineInterface::AgentState::INACTIVE),
        m_dialog({}, {}) {
}

std::shared_ptr<Assistant> Assistant::create(
    const std::string& assistantId,
    const std::string& assistantName,
    const std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule>& dialogStateRules) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIf(assistantId.empty(), "emptyAssistantId");
        ThrowIf(assistantName.empty(), "emptyAssistantName");
        ThrowIf(dialogStateRules.empty(), "emptyDialogStateRules");

        // process dialog state rules
        std::unordered_map<std::string, bool> bargeInRules;
        for (auto rule : dialogStateRules) {
            bargeInRules.insert(std::pair<std::string, bool>(rule.state, rule.wakewordInterruptionAllowed));
        }

        auto assistant = std::shared_ptr<Assistant>(new Assistant(assistantId, assistantName, bargeInRules));

        return assistant;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool Assistant::isActiveAssistant() {
    return (m_assistantState == aace::arbitrator::ArbitratorEngineInterface::AgentState::ACTIVE);
}

bool Assistant::isBargeInAllowed() {
    try {
        // barge in allowed if assistant is active AND
        // (active assistant dialog state is in wakewordInterruptionAllowed state)
        ThrowIf(m_dialog.m_dialogId.empty(), "emptyDialogIdForAssistant");
        ThrowIf(m_dialog.m_dialogState.empty(), "emptyDialogStateForAssistant");
        ThrowIf(m_bargeInRules.find(m_dialog.m_dialogState) == m_bargeInRules.end(), "noBargeinRuleForDialogState");
        auto it = m_bargeInRules.find(m_dialog.m_dialogState);
        auto bargeInAllowed = (it->second == true);

        return isActiveAssistant() && bargeInAllowed;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string Assistant::startDialog() {
    AACE_DEBUG(LX(TAG));
    // generate the dialogId and state to INITIAL_DIALOG_STATE
    auto dialogId = aace::engine::utils::uuid::generateUUID();
    m_dialog.m_dialogId = dialogId;
    m_dialog.m_dialogState = INITIAL_DIALOG_STATE;

    // set assistant to active
    m_assistantState = aace::arbitrator::ArbitratorEngineInterface::AgentState::ACTIVE;

    return dialogId;
}

void Assistant::stopDialog(const std::string& dialogId) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(dialogId.empty(), "emptyDialogIdForAssistant");
        ThrowIf((m_dialog.m_dialogId.compare(dialogId) != 0), "invalidDialogIdForAssistant");
        // stop dialog by re-setting the dialogId & dialogstate
        m_dialog.m_dialogId = {};
        m_dialog.m_dialogState = {};

        // set assistant to inactive
        m_assistantState = aace::arbitrator::ArbitratorEngineInterface::AgentState::INACTIVE;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void Assistant::setDialogState(const std::string& dialogId, const std::string& dialogState) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(dialogId.empty(), "emptyDialogIdForAssistant");
        ThrowIf((m_dialog.m_dialogId.compare(dialogId) != 0), "invalidDialogIdForAssistant");
        // set dialogstate
        m_dialog.m_dialogState = dialogState;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string Assistant::getAssistantName() {
    return m_assistantName;
}

aace::arbitrator::ArbitratorEngineInterface::AgentState Assistant::getAssistantState() {
    return m_assistantState;
}

std::string Assistant::getDialogId() {
    return m_dialog.m_dialogId;
}

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace