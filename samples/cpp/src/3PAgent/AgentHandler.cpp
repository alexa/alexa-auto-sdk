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

#include <nlohmann/json.hpp>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Wakeword/WakewordManager.h>
#include <AASB/Message/Arbitrator/Arbitrator/AgentState.h>
#include <AASB/Message/Arbitrator/Arbitrator/DeregisterAgentMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/OnAgentStateUpdatedMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/OnDialogTerminatedMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/RegisterAgentMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/SetDialogStateMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/StartDialogMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/StopDialogMessage.h>
#include <AASB/Message/wakeword/wakeword/OnWakewordDetectedMessage.h>
#include <AASB/Message/wakeword/wakeword/SetWakewordStatusMessage.h>

#include "SampleApp/3PAgent/AgentHandler.h"

// C++ Standard Library
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>


namespace sampleApp{

namespace arbitrator{
    
using json = nlohmann::json;

using namespace aasb::message::arbitrator::arbitrator;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AgentHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;

// AssistantId SIRI
static const std::string ASSISTANT_ID = "1234";

static const std::string NAME_OF_3P_AGENT = "SIRI_ASSISTANT";

static const std::string  THREE_P_WAKEWORD = "SIRI";

// To check Mic Audio bool
static bool micAudio = false;

// To check Initialization of Pryonlite Engine
static bool isVADInitialized = false;

static std::string stateString = "RECOGNIZING_SPEECH";

// Initilizing begin and end Indexes
static int beginIndexPos = 0;

static int endIndexPos = 0; 

static auto sampleRateHz = 16000;

// path to model file
static std::string pathToModelFile;

// To prevent from switching into wrong states of SIRI.  
static std::string assist_id = "UNKNOWN";

static std::string dial_id = "UNKNOWN";

// Pryonlite Class Shared Pointer
std::shared_ptr<PryonLiteVAD> pryonLiteVADInstance{};

// AgentHandler Instance
AgentHandler *agentInstance = nullptr;

AgentHandler::AgentHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    setupUI();
    subscribeToAASBMessages();
    agentInstance = this;
}

void AgentHandler::initialize() {
    pryonLiteVADInstance = PryonLiteVAD::create(m_loggerHandler, shared_from_this());
}

std::weak_ptr<Activity> AgentHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AgentHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AgentHandler::subscribeToAASBMessages(){
    log(logger::LoggerHandler::Level::INFO, "Subscribe to RegisterAgentReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleRegisterAgentReplyMessage(message); },
        RegisterAgentMessageReply::topic(),
        RegisterAgentMessageReply::action());

    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetWakewordStatusReplyMessage(message); },
        aasb::message::wakeword::wakeword::SetWakewordStatusMessageReply::topic(),
        aasb::message::wakeword::wakeword::SetWakewordStatusMessageReply::action());


    log(logger::LoggerHandler::Level::INFO, "Subscribe to DeregisterAgentReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleDeregisterAgentReplyMessage(message); },
        DeregisterAgentMessageReply::topic(),
        DeregisterAgentMessageReply::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to OnAgentStateUpdated Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleOnAgentStateUpdatedMessage(message); },
        OnAgentStateUpdatedMessage::topic(),
        OnAgentStateUpdatedMessage::action());
    
    log(logger::LoggerHandler::Level::INFO, "Subscribe to StartDialogReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStartDialogReplyMessage(message); },
        StartDialogMessageReply::topic(),
        StartDialogMessageReply::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to OnDialogTerminated Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleOnDialogTerminatedMessage(message); },
        OnDialogTerminatedMessage::topic(),
        OnDialogTerminatedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to OnWakewordDetected Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleOnWakewordDetectedMessage(message); },
        aasb::message::wakeword::wakeword::OnWakewordDetectedMessage::topic(),
        aasb::message::wakeword::wakeword::OnWakewordDetectedMessage::action());
}

void AgentHandler::handleRegisterAgentReplyMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received handleRegisterAgentReplyMessage");
    RegisterAgentMessageReply replyMsg = json::parse(message);

    auto promise = getReplyMessagePromise(replyMsg.header.messageDescription.replyToId);
    if (promise){
        promise->set_value(replyMsg.payload.success);
        if(replyMsg.payload.success){
            std::string wakeword = THREE_P_WAKEWORD;
            showMessage("Agent registered successfully");
        }
    }else{
        log(logger::LoggerHandler::Level::ERROR, "RegisterAgentMessageReply promise is null");
    }
}

void AgentHandler::handleSetWakewordStatusReplyMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received handleSetWakewordStatusReplyMessage");
    aasb::message::wakeword::wakeword::SetWakewordStatusMessageReply replyMsg = json::parse(message);

    auto promise = getReplyMessagePromise(replyMsg.header.messageDescription.replyToId);
    if (promise){
        promise->set_value(replyMsg.payload.success);
        if(replyMsg.payload.success){
            showMessage("3P Wakeword state changed successfully");
        }
    }else{
        log(logger::LoggerHandler::Level::ERROR, "3P Wakeword state change failed");
    }
}

void AgentHandler::handleDeregisterAgentReplyMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received handleDeregisterAgentReplyMessage");
    DeregisterAgentMessageReply replyMsg = json::parse(message);

    auto promise = getReplyMessagePromise(replyMsg.header.messageDescription.replyToId);
    if(promise){
        promise->set_value(replyMsg.payload.success);
        if(replyMsg.payload.success){
            showMessage("Agent Deregisterd successfully");
        }
    }else{
        log(logger::LoggerHandler::Level::ERROR, "DeregisterAgentMessageReply promise is null");
    }
}

void AgentHandler::handleStartDialogReplyMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received handleStartDialogReplyMessage");
    StartDialogMessageReply replyMsg = json::parse(message);

    auto promise = getReplyMessagePromise(replyMsg.header.messageDescription.replyToId);
    if(promise){
        promise->set_value(replyMsg.payload.success);
        if(replyMsg.payload.success){
            showMessage("Recieved reply for the ID: " + replyMsg.payload.dialogId);
            auto timestamp = calculateStartOfKeywordTimeStamp(beginIndexPos, endIndexPos);
            std::string message = "Hey"  + THREE_P_WAKEWORD + "! was said ... at TimeStamp: ";
            requestSiri(timestamp, message);
            std::string state = "RECOGNIZING_SPEECH";
            assist_id = replyMsg.payload.assistantId;
            dial_id =  replyMsg.payload.dialogId;
            stateString = "SPEAKING";
            setDialogState(replyMsg.payload.assistantId, replyMsg.payload.dialogId, state);
        }
        else{
            log(logger::LoggerHandler::Level::ERROR, "StartDialog denied with reason: " + replyMsg.payload.reason);
        }
    }else{
        log(logger::LoggerHandler::Level::ERROR, "StartDialogMessageReply promise is null");
    }
}

void AgentHandler::handleOnAgentStateUpdatedMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received OnAgentStateUpdatedMessage");
    OnAgentStateUpdatedMessage msg = json::parse(message);
    onAgentStateUpdated(msg.payload.assistantId, msg.payload.name, msg.payload.state);
}

void AgentHandler::handleOnDialogTerminatedMessage(const std::string& message){
    log(logger::LoggerHandler::Level::INFO, "Received OnDialogTerminatedMessage");
    OnDialogTerminatedMessage msg = json::parse(message);
    onDialogTerminated(msg.payload.assistantId, msg.payload.dialogId, msg.payload.reason);
}

void AgentHandler::handleOnWakewordDetectedMessage(const std::string& message){
    auto activity = m_activity.lock();
        if (!activity) {
            return;
        }
    activity->runOnUIThread([=]() {
        log(logger::LoggerHandler::Level::INFO, "Received OnWakewordDetectedMessage");
        aasb::message::wakeword::wakeword::OnWakewordDetectedMessage msg = json::parse(message);
        onWakewordDetected(msg.payload.wakeword, msg.payload.beginIndex, msg.payload.endIndex);
    });
}

bool AgentHandler::enable3PWakeword(const std::string& wakeword, bool state){
        aasb::message::wakeword::wakeword::SetWakewordStatusMessage msg;
        msg.payload.name = wakeword;
        msg.payload.value = state;

        m_messageBroker->publish(msg.toString());
	 return waitForAsyncReply(msg.header.id);

}

bool AgentHandler::registerAgent(const std::string& assistantId, const std::string& name, std::vector<aasb::message::arbitrator::arbitrator::DialogStateRule> dialogStateRules){
	
    RegisterAgentMessage msg;
    msg.payload.assistantId = assistantId;
    msg.payload.name = name;
    msg.payload.dialogStateRules = dialogStateRules;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

bool AgentHandler::deregisterAgent(const std::string& assistantId){
    DeregisterAgentMessage msg;
    msg.payload.assistantId = ASSISTANT_ID;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

void AgentHandler::setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state){
    SetDialogStateMessage msg;
    msg.payload.assistantId = assistantId;
    msg.payload.dialogId = dialogId;
    msg.payload.state = state;
    showMessage("Mode changed to: " + state);
    m_messageBroker->publish(msg.toString());
}

bool AgentHandler::startDialog(const std::string& assistantId, aasb::message::arbitrator::arbitrator::Mode& mode){
    StartDialogMessage msg;
    msg.payload.assistantId = assistantId;
    msg.payload.mode = mode;
    m_messageBroker->publish(msg.toString());    
    return waitForAsyncReply(msg.header.id);
}    

void AgentHandler::stopDialog(std::string& assistantId, const std::string& dialogId){
    StopDialogMessage msg;
    msg.payload.assistantId = assistantId;
    msg.payload.dialogId = dialogId;

    m_messageBroker->publish(msg.toString());
}

void AgentHandler::onAgentStateUpdated(std::string& assistantId, std::string& name, aasb::message::arbitrator::arbitrator::AgentState state){
    OnAgentStateUpdatedMessage msg;
    msg.payload.state = state;
    msg.payload.name = name;
    switch (state) {
        case aasb::message::arbitrator::arbitrator::AgentState::ACTIVE:
            showMessage(name + " is active");
            break;
        case aasb::message::arbitrator::arbitrator::AgentState::INACTIVE:
            showMessage(name + " is inactive");
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "Invalid AgentState value");
            return;
    };
}

void AgentHandler::onDialogTerminated(std::string& assistantId,std::string& dialogId, std::string& reason){
    showMessage("Dialog terminated with reason: " + reason);
}

void AgentHandler::onWakewordDetected(std::string& wakeword, int beginIndex, int endIndex){
    aasb::message::wakeword::wakeword::OnWakewordDetectedMessage msg;
    beginIndexPos = beginIndex;
    endIndexPos = endIndex;
    showMessage("WakeWord detected: " + wakeword);
    auto mode = aasb::message::arbitrator::arbitrator::Mode::WAKEWORD;
    std::string assistantId = ASSISTANT_ID;
    startDialog(assistantId, mode);
}

std::chrono::time_point<std::chrono::system_clock> AgentHandler::calculateStartOfKeywordTimeStamp(int& beginIndex, int& endIndex){
    // Start with the current time in the startOfSpeechTimestamp corresponding with the currentIndex from the reader.
    std::chrono::time_point<std::chrono::system_clock> startOfSpeechTimestamp = std::chrono::system_clock::now();

    // Translate the currentIndex position to a time duration elapsed since the end of wakeword.
    const auto sampleRatePerMillisec = sampleRateHz / 1000;

    const auto timeSinceStartOfWW = std::chrono::milliseconds((endIndex - beginIndex) / sampleRatePerMillisec);

    // Adjust the start of speech timestamp to be the start of the WW.
    startOfSpeechTimestamp -= timeSinceStartOfWW;
    
    return startOfSpeechTimestamp;
}

void AgentHandler::setStartofSpeechTimeStamp(std::chrono::time_point<std::chrono::system_clock>& startofSpeechTimeStamp){
    std::string message = "Voice Activity Begin Timestamp: ";
    requestSiri(startofSpeechTimeStamp, message);
}

void AgentHandler::requestSiri(std::chrono::time_point<std::chrono::system_clock>& timeStamp, std::string& message){
    std::string voiceActivation;
    // Converting system_clock to NTP. requestSIRI gives timestamp in NTP format.
    const long long secs = 2208988800;  // Total number of seconds from 1 Jan 1900 to 31 Dec 1970 is = 60*60*24*25567 = 2208988800;
    long long beginTimestamp = std::chrono::duration_cast<std::chrono::seconds>(timeStamp.time_since_epoch()).count()  // total seconds form 1 Jan 1971 to now
                                    + (std::chrono::seconds(secs)).count();
    showMessage(message + std::to_string(beginTimestamp));
}

void AgentHandler::VADFeedAudio(AgentHandler::AudioType type){
    if(!isVADInitialized){
        std::ifstream file(pathToModelFile);
        if (!file) {
            log(logger::LoggerHandler::Level::ERROR, "Invalid path to Models file");
            return;
        }
        pryonLiteVADInstance->getWavFilePath(pathToModelFile);
        std::string pathToFingerprintsFile = std::string();
        std::string pathToWatermarkCfgFile = std::string();
        isVADInitialized = pryonLiteVADInstance->init(pathToModelFile, pathToFingerprintsFile, pathToWatermarkCfgFile);
        if(isVADInitialized){
            showMessage("VAD Mode is set successfully");
        }
    }
    if(type == AgentHandler::AudioType::AUDIO_FILE){   
        if(micAudio){
            pryonLiteVADInstance->stopMicAudio();
            micAudio = false;
        }
        pryonLiteVADInstance->feedAudio();
    }
    if(type == AgentHandler::AudioType::MIC_AUDIO){
        if(!micAudio){
            pryonLiteVADInstance->startMicAudio();
            micAudio = true;
        } else{
            log(logger::LoggerHandler::Level::ERROR, "Mic Audio already enabled");
        }
    }
}

bool AgentHandler::getPath(const std::vector<json>& jsons){
    for (auto const& j : jsons) {
        try {
           if (j.find("pryonLiteVAD") != j.end()) {
                pathToModelFile = std::string(j["pryonLiteVAD"]["rootPath"]) + "/" + std::string(j["pryonLiteVAD"]["models"]);
                return true;
            }
        } catch (json::exception& e){
        }
    }
    return false;
}

void AgentHandler::setCarPlayMode(AgentHandler::CarPlayMode mode){
    std::string wakeword = THREE_P_WAKEWORD;
    if (AgentHandler::CarPlayMode::KWD == mode){
        showMessage("KWD Mode is set");
        pryonLiteVADInstance->destroy();
        micAudio = false;
        isVADInitialized = false;
        enable3PWakeword(wakeword, true);

    } else if(AgentHandler::CarPlayMode::DEACTIVATED == mode) {
        showMessage("Deactivated Mode is set");
        pryonLiteVADInstance->destroy();
        micAudio = false;
        isVADInitialized = false;
        enable3PWakeword(wakeword, false);
    }
}

void AgentHandler::immediateAgentInvocation(std::string& assistantId ,aasb::message::arbitrator::arbitrator::Mode mode){
    if(aasb::message::arbitrator::arbitrator::Mode::GESTURE == mode){
        showMessage("Gesture Mode is set");
    }
    else if (aasb::message::arbitrator::arbitrator::Mode::WAKEWORD == mode){
        showMessage("Wakeword Mode is set");
    }
    startDialog(assistantId, mode);
}
 
bool AgentHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the AgentHandler reply message to fulfill
    std::shared_ptr<AgentHandlerPromise> promise = std::make_shared<AgentHandlerPromise>();

    // create a future to receive the promised AgentHandler reply message when it is received
    std::shared_future<bool> future(promise->get_future());

    bool success = false;
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void AgentHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<AgentHandlerPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void AgentHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<AgentHandler::AgentHandlerPromise> AgentHandler::getReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

void AgentHandler::showMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, message);
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}

void AgentHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AgentHandler", message);
}

void AgentHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    // Registration of Agent
    activity->registerObserver(Event::registerSiriAgent, [=](const std::string&) {
        showMessage("Starting Registration Agent flow...");
        std::string assistantId = ASSISTANT_ID;
        std::string name = NAME_OF_3P_AGENT;
        std::vector<aasb::message::arbitrator::arbitrator::DialogStateRule> dialogStateRules;
        aasb::message::arbitrator::arbitrator::DialogStateRule dialog;
        std::vector<std::string> states = {"RECOGNIZING_SPEECH", "SPEAKING", "NONE"};
        for (auto state : states){
            if (state == "SPEAKING"){
                dialog.wakewordInterruptionAllowed = true;
            }else{
                dialog.wakewordInterruptionAllowed = false;
            }
        dialog.state = state;
        dialogStateRules.push_back(dialog);
        }
        registerAgent(assistantId, name, dialogStateRules);
        return true;
    });

    // Deregistration of Agent
    activity->registerObserver(Event::onDeregisterAgent, [=](const std::string&) {
        showMessage("Deregistration of Agent...");
        std::string assistantId = ASSISTANT_ID; 
        deregisterAgent(assistantId);
        return true;
    });

    // Audio File for VAD
    activity->registerObserver(Event::onAudioFile, [=](const std::string&) {
        VADFeedAudio(AgentHandler::AudioType::AUDIO_FILE);
        return true;
    });

    // Mic Audio for VAD
    activity->registerObserver(Event::onMicAudio, [=](const std::string&) {
        VADFeedAudio(AgentHandler::AudioType::MIC_AUDIO);
        return true;
    });

    // If Set to KeyWord Detection CarPlay Mode
    activity->registerObserver(Event::onKWD, [=](const std::string&) {
        showMessage("Car Play mode is set...");
        setCarPlayMode(AgentHandler::CarPlayMode::KWD);
        return true;
    });

    // If Set to Deactivated CarPlay Mode
    activity->registerObserver(Event::onDeactivated, [=](const std::string&) {
        showMessage("Car Play mode is set...");
        setCarPlayMode(AgentHandler::CarPlayMode::DEACTIVATED);
        return true;
    });

    activity->registerObserver(Event::onPTT, [=](const std::string&) {
        std::string assistantId = ASSISTANT_ID;
        immediateAgentInvocation(assistantId ,aasb::message::arbitrator::arbitrator::Mode::GESTURE);
        return true;
    });
    
    activity->registerObserver(Event::onSiriSPEAKING, [=](const std::string&) {
        // Comparing assist_id and dial_id in order to prevent wrong state switching
        if((stateString != "SPEAKING") || (assist_id == "UNKNOWN") || (dial_id == "UNKNOWN")){
            log(logger::LoggerHandler::Level::ERROR, "Can not switch to SPEAKING state");
            return false;
        }
        std::string mode = "SPEAKING";
        stateString = "NONE";
        setDialogState(assist_id, dial_id, mode);
        return true;
    });

    activity->registerObserver(Event::onSiriNONE, [=](const std::string&) {
        // Comparing assist_id and dial_id in order to prevent wrong state switching
        if((stateString != "NONE") || (assist_id == "UNKNOWN") || (dial_id == "UNKNOWN")){
            log(logger::LoggerHandler::Level::ERROR, "Can not switch to NONE state");
            return false; 
        }
        std::string mode = "NONE";
        stateString = "RECOGNIZING_SPEECH";
        setDialogState(assist_id, dial_id, mode);
        stopDialog(assist_id, dial_id);
        return true;
    });
}


} //namespace arbitrator

} // namespace sampleapp
