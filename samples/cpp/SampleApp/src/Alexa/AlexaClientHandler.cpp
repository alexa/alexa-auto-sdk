/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/AlexaClientHandler.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

#ifdef OBIGO_AIDAEMON
#include "SampleApp/VPA/IPCHandler.h"
#include "SampleApp/VPA/AIDaemon-IPC.h"
#endif // OBIGO_AIDAEMON

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaClientHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AlexaClientHandler::AlexaClientHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AlexaClientHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> AlexaClientHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::alexa::AlexaClient interface

void AlexaClientHandler::dialogStateChanged(AlexaClient::DialogState state) {
    std::stringstream ss;
    ss << state;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto dialogStateView = m_dialogStateView.lock()) {
            dialogStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine("Dialog state changed:", state);
        }
    });
    // Special case for test automation
    if (state == AlexaClient::DialogState::IDLE) {
#ifdef OBIGO_AIDAEMON
        log(logger::LoggerHandler::Level::INFO, "Idle...");
        AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_VR_STATE, AIDAEMON::AI_VR_STATE_IDLE);
        if (auto executor = activity->getExecutor()) {
            executor->submit([=]() { activity->notify(Event::onTestAutomationProcess); });
        }
    } else if (state == AlexaClient::DialogState::LISTENING) {
        log(logger::LoggerHandler::Level::INFO, "Listening...");
        AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_VR_STATE, AIDAEMON::AI_VR_STATE_LISTENING);
    } else if (state == AlexaClient::DialogState::THINKING) {
        log(logger::LoggerHandler::Level::INFO, "Thinking...");
        AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_VR_STATE, AIDAEMON::AI_VR_STATE_THINKING);
    } else if (state == AlexaClient::DialogState::SPEAKING) {
        log(logger::LoggerHandler::Level::INFO, "Speaking...");
#endif // OBIGO_AIDAEMON
    }
}

void AlexaClientHandler::authStateChanged(AlexaClient::AuthState state, AlexaClient::AuthError error) {
    std::stringstream ss;
    ss << state << '/' << error;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto authStateView = m_authStateView.lock()) {
            authStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine("Auth state changed:", state, "(", error, ")");
        }

#ifdef OBIGO_AIDAEMON
        if (error == AlexaClient::AuthError::AUTHORIZATION_PENDING) {
            std::ostringstream stream;
            std::string aireason;

            stream << error;
            aireason =  stream.str();

            AIDAEMON::IPCHandler::GetInstance()->sendAIStatus(AIDAEMON::AI_STATUS_UNAUTH, aireason);
        }
#endif // OBIGO_AIDAEMON 
    });
}

void AlexaClientHandler::connectionStatusChanged(AlexaClient::ConnectionStatus status, AlexaClient::ConnectionChangedReason reason) {
    std::stringstream ss;
    ss << status << '/' << reason;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto connectionStatusView = m_connectionStatusView.lock()) {
            connectionStatusView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine("Connection status changed:", status, "(", reason, ")");
        }

#ifdef OBIGO_AIDAEMON
            std::ostringstream stream;
            std::string aistatus;
            std::string aireason;

            stream << status;
            aistatus =  stream.str();

            stream << reason;
            aireason =  stream.str();

            AIDAEMON::IPCHandler::GetInstance()->sendAIStatus(aistatus, aireason);
#endif

    });
    // Special case for test automation
    if (status == AlexaClient::ConnectionStatus::CONNECTED) {
        if (reason == AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST) {
            if (auto executor = activity->getExecutor()) {
                executor->submit([=]() { activity->notify(Event::onTestAutomationConnect); });
            }
        }
    }
}

#ifdef OBIGO_AIDAEMON
void AlexaClientHandler::readyTTS(std::string dialogRequestId) {
  log(logger::LoggerHandler::Level::INFO, "readyTTS dialogRequestId" + dialogRequestId);
  rapidjson::Document ttsready(rapidjson::kObjectType);

  ttsready.AddMember(
      AIDAEMON::DIALOGID,
      rapidjson::Value().SetString(dialogRequestId.c_str(), dialogRequestId.length(), ttsready.GetAllocator()),
      ttsready.GetAllocator());
  AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_AI_TTS_READY, &ttsready);
}

void AlexaClientHandler::startedTTS(std::string dialogRequestId) {
  log(logger::LoggerHandler::Level::INFO, "startedTTS dialogRequestId" + dialogRequestId);
  rapidjson::Document ttsstart(rapidjson::kObjectType);

  ttsstart.AddMember(
      AIDAEMON::DIALOGID,
      rapidjson::Value().SetString(dialogRequestId.c_str(), dialogRequestId.length(), ttsstart.GetAllocator()),
      ttsstart.GetAllocator());
  AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_TTS_START, &ttsstart);
}

void AlexaClientHandler::finishedTTS(std::string dialogRequestId) {
  log(logger::LoggerHandler::Level::INFO, "finishedTTS dialogRequestId" + dialogRequestId);
  rapidjson::Document ttsfinish(rapidjson::kObjectType);

  ttsfinish.AddMember(
      AIDAEMON::DIALOGID,
      rapidjson::Value().SetString(dialogRequestId.c_str(), dialogRequestId.length(), ttsfinish.GetAllocator()),
      ttsfinish.GetAllocator());
  AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_TTS_FINISH, &ttsfinish);
}

#ifdef OBIGO_SPEECH_SENDER
void AlexaClientHandler::sendDataToMVPA(From from, void* data) {
  if (from == avsCommon::sdkInterfaces::MVPAInterface::From::SpeechSynthesizer) {
    log(logger::LoggerHandler::Level::CRITICAL, "sendDataToMVPA From SpeechSynthesizer");
    using namespace avsCommon::sdkInterfaces;
    MVPAInterface::MVPASpeech* pSpeechData = static_cast<MVPASpeech*>(data);
    log(logger::LoggerHandler::Level::CRITICAL, "SeqNum : " + pSpeechData->seqnum);
    log(logger::LoggerHandler::Level::CRITICAL, "readBytes : " + pSpeechData->readBytes);

    rapidjson::Document speechData(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = speechData.GetAllocator();
    speechData.AddMember(AIDAEMON::AI_SPEECH_DIALOGID,
                         rapidjson::Value().SetString(pSpeechData->dialogId.c_str(), pSpeechData->dialogId.length(),
                                                      speechData.GetAllocator()),
                         allocator);
    speechData.AddMember(AIDAEMON::AI_SPEECH_READBYTES,
                         rapidjson::Value().SetString(pSpeechData->readBytes.c_str(), pSpeechData->readBytes.length(),
                                                      speechData.GetAllocator()),
                         allocator);
    speechData.AddMember(AIDAEMON::AI_SPEECH_NUM,
                         rapidjson::Value().SetString(pSpeechData->seqnum.c_str(), pSpeechData->seqnum.length(),
                                                      speechData.GetAllocator()),
                         allocator);

    if (pSpeechData->readBytes > 0) {
      std::string encoded =
          base64_encode(reinterpret_cast<const unsigned char*>(pSpeechData->data), pSpeechData->readBytes);
      speechData.AddMember(rapidjson::StringRef(AIDAEMON::AI_SPEECH_SPEECH), encoded, allocator);
    }

    AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_AI_SPEECH, &speechData);
  } else if (from == avsCommon::sdkInterfaces::MVPAInterface::From::Alerts) {
    log(logger::LoggerHandler::Level::CRITICAL, "sendDataToMVPA From Alerts");
  } else {
    log(logger::LoggerHandler::Level::CRITICAL, "sendDataToMVPA From Unknown");
  }
}
#endif  // OBIGO_SPEECH_SENDER
#endif

// private

void AlexaClientHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlexaClientHandler", message);
}

void AlexaClientHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // text views
    m_authStateView = activity->findViewById("id:AuthState");
    m_connectionStatusView = activity->findViewById("id:ConnectionStatus");
    m_dialogStateView = activity->findViewById("id:DialogState");

    // initial text views
    activity->runOnUIThread([=]() {
        if (auto authStateView = m_authStateView.lock()) {
            std::stringstream ss;
            ss << AlexaClient::AuthState::UNINITIALIZED;
            authStateView->setText(ss.str());
        }
        if (auto connectionStatusView = m_connectionStatusView.lock()) {
            std::stringstream ss;
            ss << AlexaClient::ConnectionStatus::DISCONNECTED;
            connectionStatusView->setText(ss.str());
        }
        if (auto dialogStateView = m_dialogStateView.lock()) {
            std::stringstream ss;
            ss << AlexaClient::DialogState::IDLE;
            dialogStateView->setText(ss.str());
        }
    });
}

} // namespace alexa
} // namespace sampleApp
