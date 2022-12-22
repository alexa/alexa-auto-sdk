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


#ifndef AACE_ENGINE_ARBITRATOR_ASSISTANT_H
#define AACE_ENGINE_ARBITRATOR_ASSISTANT_H

#include <nlohmann/json.hpp>
#include <string>

#include <AACE/Arbitrator/ArbitratorEngineInterface.h>

#include "ArbitratorObserverInterface.h"

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace arbitrator {

/**
 * Represents an assistant
 */
class Assistant {
public:
    /// struct that represents the dialog for an active assistant
    struct Dialog {
        /// The dialog id
        std::string m_dialogId;

        /// The dialog state
        std::string m_dialogState;

        Dialog(std::string dialogId, std::string dialogState) : m_dialogId(dialogId), m_dialogState(dialogState) {
        }
    };
    /**
     * Create an @c Assistant object 
     *
     * @param assistantId the assistant Id.
     * @param assistantName the assistant name.
     * @param dialogStateRules the dialog state rules.
     * @return A pointer to a new @c Assistant if arguments are valid, otherwise @c nullptr.
     */
    static std::shared_ptr<Assistant> create(
        const std::string& assistantId,
        const std::string& assistantName,
        const std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule>& dialogStateRules);

    /**
     * Assistant destructor
     */
    virtual ~Assistant() = default;

    /**
     * Check if assistant is active
     *
     * @return true if assistant is active, false otherwise
     */
    bool isActiveAssistant();

    /**
     * Check if barge in is allowed
     *
     * @return true if barge in is allowed, false otherwise
     */
    bool isBargeInAllowed();

    /**
     * Start dialog.
     *
     * @return dialogId The unique id of the dialog that is started
     */
    std::string startDialog();

    /**
     * Stop the dialog
     *
     * @param dialogId The id of the dialog, that was returned by startDialog(), that should be stopped.
     */
    void stopDialog(const std::string& dialogId);

    /**
     * Set the dialog state
     *
     * @param dialogId The id of the dialog whose state should be set
     * @param dialogState The new state of the dialog
     */
    void setDialogState(const std::string& dialogId, const std::string& dialogState);

    /** 
     * Get assisant name
     * 
     * @return assistant name
     */
    std::string getAssistantName();

    /** 
     * Get assisant state
     * 
     * @return assistant state
     */
    aace::arbitrator::ArbitratorEngineInterface::AgentState getAssistantState();

    /** 
     * Get assisant's dialog id
     * 
     * @return dialog id
     */
    std::string getDialogId();

private:
    /**
     * Assistant constructor
     */
    Assistant(
        const std::string& assistantId,
        const std::string& assistantName,
        std::unordered_map<std::string, bool> bargeInRules);

    /// The supplied assistant ID for this assistant
    std::string m_assistantId;

    /// The supplied assistant name for this assistant
    std::string m_assistantName;

    /// A map of dialog states and if barge in is allowed for the state
    std::unordered_map<std::string, bool> m_bargeInRules;

    /// The assistant state - one of ACTIVE or INACTIVE
    aace::arbitrator::ArbitratorEngineInterface::AgentState m_assistantState;

    /// The dialog for an active assistant
    Dialog m_dialog;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ASSISTANT_H
