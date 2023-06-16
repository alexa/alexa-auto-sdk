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
#ifndef SAMPLEAPP_ARBITRATOR_AGENTHANDLER_H
#define SAMPLEAPP_ARBITRATOR_AGENTHANDLER_H

#include <AACE/Core/MessageBroker.h>
#include <AACE/Arbitrator/ArbitratorEngineInterface.h>
#include <AASB/Message/Arbitrator/Arbitrator/DialogStateRule.h>
#include <AASB/Message/Arbitrator/Arbitrator/Mode.h>
#include <AASB/Message/Arbitrator/Arbitrator/AgentState.h>
#include "PryonliteVAD.h"
#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"


#include <fstream>
#include <thread>
#include <stdio.h>
#include <chrono>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp{

namespace arbitrator {
 
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AgentHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AgentHandler
            : public AgentPryonListenerInterface
            , public std::enable_shared_from_this<AgentHandler> {

public:
    /// The enum State describes the state of CarPlay Mode.
    enum class CarPlayMode {
        // Voice Activity Detection Mode.
        VAD,
        // Keyword Detection Mode.
        KWD,
        // Deactivation Mode.
        DEACTIVATED,
    };

    /// The enum of type of Audio Input
    enum class AudioType{
        // Mic Audio
        MIC_AUDIO,
        // Audio File
        AUDIO_FILE,
    };

private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AgentHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AgentHandler> {
        auto var = std::shared_ptr<AgentHandler>(new AgentHandler(args...));
        var->initialize();
        return var;
        }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    auto subscribeToAASBMessages() -> void;
    /**
     * Initializes the object. 
     */
    auto initialize() -> void;
    /**
     * Handles the StartDialogReply messages received from the Engine.
     * 
     * @param [in] message The message received from the Engine
     */
    void handleStartDialogReplyMessage(const std::string& message); 
    /**
     * Handles the RegisterAgentReply messages received from the Engine.
     * 
     * @param [in] message The message received from the Engine
     */
    void handleRegisterAgentReplyMessage(const std::string& message);   

    /**
     * Handles the SetWakewordStatusReply messages received from the Engine.
     * 
     * @param [in] message The message received from the Engine
     */

    void handleSetWakewordStatusReplyMessage(const std::string& message);

    /**
     * Handles the DeregisterAgentReply messages received from the Engine.
     * 
     * @param [in] message The message received from the Engine
     */
    void handleDeregisterAgentReplyMessage(const std::string& message); 
    /**
     * Handles the AgentStateUpdated message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleOnAgentStateUpdatedMessage(const std::string& message);
    /**
     * Handles the DialogTerminated message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleOnDialogTerminatedMessage(const std::string& message);
    /**
     * Handles the WakewordDetected message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleOnWakewordDetectedMessage(const std::string& message);
    /**
     * AgentPryonListenerInterface override
     * 
     * Notifies the AgentHandler with VAD time stamp from PryonliteVAD
     */
    void setStartofSpeechTimeStamp(std::chrono::time_point<std::chrono::system_clock>& startofSpeechTimeStamp) override;

private:
    using AgentHandlerPromise = std::promise<bool>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<AgentHandlerPromise>> m_promiseMap;

    bool waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<AgentHandlerPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<AgentHandlerPromise> getReplyMessagePromise(const std::string& messageId);

private:
    /**
     * @brief Sets the Agent invocation to the respective mode. 
     * 
     * @param mode Invocation Mode: WAKEWORD or GESTURE (PTT/ TTT)
     */
    void immediateAgentInvocation(std::string& assistantId ,aasb::message::arbitrator::arbitrator::Mode mode);
    /**
     * @brief Set the Car Play Mode object
     * 
     * @param mode VAD, KWD or Deactivated
     */
    void setCarPlayMode(CarPlayMode mode);
    // Methods to publish messages to Engine (Arbitrator)
    /**
     * Register an agent with the Engine
     */
    bool registerAgent(const std::string& assistantId,const std::string& name, std::vector<aasb::message::arbitrator::arbitrator::DialogStateRule> dialogStateRules);
    /**
     * Deregister an agent with the Engine
     */
    bool deregisterAgent(const std::string& assistantId);
    /**
     * Notifies Engine to set the dialog state
     */
    void setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state);  
    /**
     * Notifies Engine to start the dialog
     */    
    bool startDialog(const std::string& assistantId, aasb::message::arbitrator::arbitrator::Mode& mode);
    /**
     * Notifies Engine to stop the dialog
     */
    void stopDialog(std::string& assistantId, const std::string& dialogId);    
    /**
     * Notifies Engine to enable 3P wakeword
     */
    bool enable3PWakeword(const std::string& wakeword, bool state);

private:
    /**
     *  Provides the implementation for Dialog Termination.
     */
    void onDialogTerminated(std::string& assistantId,std::string& dialogId, std::string& reason);
    /**
     *  Provides the implementation for Agent State Updated.
     */
    void onAgentStateUpdated(std::string& assistantId, std::string& name, aasb::message::arbitrator::arbitrator::AgentState state);
    /**
     *  Provides the implementation for On Wakeword Detection.
     */
    void onWakewordDetected(std::string& wakeword, int beginIndex, int endIndex);
    /**
     *  Provides the implementation to calculate Start of Speech timestamp.
     */
    std::chrono::time_point<std::chrono::system_clock> calculateStartOfKeywordTimeStamp(int& beginIndex, int& endIndex);
    
public:
    /**
     *  Method for handshake between SIRI and SampleApp for Timestamp in NTP.
     * @param Timestamp and corresponding Message 
     */
    void requestSiri(std::chrono::time_point<std::chrono::system_clock>& timeStamp, std::string& message);
    /**
     *  get the Path to sampleApp folder. 
     */
    static bool getPath(const std::vector<json>& jsons);

private:
    /**
     * Log entry.
     */
   auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    /**as
     * Setup all the event listeners.
     */
    auto setupUI() -> void;
    /**
     * Show Message to Console
     * 
     */
    void showMessage(const std::string& message);

private:
    /**
     * @brief Pryonlite is initialized and Audio is Fed of Type: Mic Audio or Audio File.
     * 
     */
    void VADFeedAudio(AgentHandler::AudioType type);
    
    // Console
    std::weak_ptr<View> m_console{};
};

}  // namespace arbitrator

} // namespace sampleApp
 
#endif  // SAMPLEAPP_ARBITRATOR_AGENTHANDLER_H
