/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/PhoneCallController/PhoneCallControllerCapabilityAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace phoneCallController {

static const std::string TAG("aace.phoneCallController.PhoneCallControllerCapabilityAgent");

static const std::string NAMESPACE{"Alexa.Comms.PhoneCallController"};

static const alexaClientSDK::avsCommon::avs::NamespaceAndName DIAL{NAMESPACE, "Dial"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName REDIAL{NAMESPACE, "Redial"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName STOP{NAMESPACE, "Stop"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANSWER{NAMESPACE, "Answer"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName PLAY_RINGTONE{NAMESPACE, "PlayRingtone"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SEND_DTMF{NAMESPACE, "SendDTMF"};

static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_NAME = "Alexa.Comms.PhoneCallController";
static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_VERSION = "2.0";

static const alexaClientSDK::avsCommon::avs::NamespaceAndName CONTEXT_MANAGER_PHONE_CONTROL_STATE{
    NAMESPACE,
    "PhoneCallControllerState"};

static const std::string CHANNEL_NAME =
    alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME;

static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getPhoneCallControllerCapabilityConfiguration();

std::shared_ptr<PhoneCallControllerCapabilityAgent> PhoneCallControllerCapabilityAgent::create(
    std::shared_ptr<PhoneCallControllerInterface> phoneCallController,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager) {
    try {
        ThrowIfNull(phoneCallController, "nullPhoneCallControllerInterface");
        ThrowIfNull(contextManager, "nullContextManager");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(focusManager, "nullFocusManager");

        auto phoneCallControllerCapabilityAgent =
            std::shared_ptr<PhoneCallControllerCapabilityAgent>(new PhoneCallControllerCapabilityAgent(
                phoneCallController, contextManager, exceptionSender, messageSender, focusManager));

        return phoneCallControllerCapabilityAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void PhoneCallControllerCapabilityAgent::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    // Do nothing here as directives are handled in the handle stage.
}

void PhoneCallControllerCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    // Do nothing here as directives are handled in the handle stage.
}

void PhoneCallControllerCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    try {
        ThrowIfNot(info && info->directive, "nullDirectiveInfo");

        if (m_connectionState !=
            aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED) {
            sendExceptionEncounteredAndReportFailed(
                info,
                "Connection state is not CONNECTED",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);
            return;
        }

        if (info->directive->getName() == DIAL.name) {
            handleDialDirective(info);
        } else if (info->directive->getName() == REDIAL.name) {
            handleRedialDirective(info);
        } else if (info->directive->getName() == ANSWER.name) {
            handleAnswerDirective(info);
        } else if (info->directive->getName() == STOP.name) {
            handleStopDirective(info);
        } else if (info->directive->getName() == PLAY_RINGTONE.name) {
            handlePlayRingtoneDirective(info);
        } else if (info->directive->getName() == SEND_DTMF.name) {
            handleSendDTMFDirective(info);
        } else {
            handleUnknownDirective(info);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleDirective").d("reason", ex.what()));
    }
}

void PhoneCallControllerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration PhoneCallControllerCapabilityAgent::getConfiguration()
    const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioNonBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy(
        alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUM_AUDIO, false);
    configuration[DIAL] = audioNonBlockingPolicy;
    configuration[REDIAL] = audioNonBlockingPolicy;
    configuration[ANSWER] = audioNonBlockingPolicy;
    configuration[STOP] = audioNonBlockingPolicy;
    configuration[PLAY_RINGTONE] = audioNonBlockingPolicy;
    configuration[SEND_DTMF] = audioNonBlockingPolicy;
    return configuration;
}

PhoneCallControllerCapabilityAgent::PhoneCallControllerCapabilityAgent(
    std::shared_ptr<PhoneCallControllerInterface> phoneCallController,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"PhoneCallControllerCapabilityAgent"},
        m_contextManager{contextManager},
        m_messageSender{messageSender},
        m_focusManager{focusManager},
        m_phoneCallController{phoneCallController} {
    m_capabilityConfigurations.insert(getPhoneCallControllerCapabilityConfiguration());
    m_connectionState = aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED;
    updateContextManager();
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getPhoneCallControllerCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_NAME});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void PhoneCallControllerCapabilityAgent::doShutdown() {
    AACE_INFO(LX(TAG));

    m_executor.shutdown();
    m_phoneCallController.reset();
    m_messageSender.reset();
    m_contextManager.reset();

    if (m_focusManager != nullptr) {
        // Release channel and wait on promise before resetting pointer
        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this()).get();
        m_focusManager.reset();
    }
}

void PhoneCallControllerCapabilityAgent::sendExceptionEncounteredAndReportFailed(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& message,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType type) {
    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);

    if (info && info->result) {
        info->result->setFailed(message);
    }

    removeDirective(info);
}

void PhoneCallControllerCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    if (info->directive && info->result) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void PhoneCallControllerCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info && info->result) {
        info->result->setCompleted();
    }
    removeDirective(info);
}

void PhoneCallControllerCapabilityAgent::handleDialDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handleDialDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        if (!document.HasMember("callId")) {
            AACE_ERROR(LX(TAG, "handleDialDirective").d("reason", "missing callId"));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Missing callId",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }
        auto callId = document["callId"].GetString();
        addCall(callId, CallState::IDLE);

        if (m_phoneCallController->dial(info->directive->getPayload())) {
            m_callMethodMap[callId] = CallMethod::DIAL;
            m_currentCallId = callId;
        } else {
            removeCall(callId);
        }
        updateContextManager();
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handleRedialDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handleRedialDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        if (!document.HasMember("callId")) {
            AACE_ERROR(LX(TAG, "handleRedialDirective").d("reason", "missing callId"));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Missing callId",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }
        auto callId = document["callId"].GetString();
        addCall(callId, CallState::IDLE);

        if (m_phoneCallController->redial(info->directive->getPayload())) {
            m_callMethodMap[callId] = CallMethod::REDIAL;
            m_currentCallId = callId;
        } else {
            removeCall(callId);
        }
        updateContextManager();
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handleAnswerDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handleAnswerDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        m_phoneCallController->answer(info->directive->getPayload());
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handleStopDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handleStopDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        m_phoneCallController->stop(info->directive->getPayload());
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handlePlayRingtoneDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handlePlayRingtoneDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        m_phoneCallController->playRingtone(info->directive->getPayload());
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handleSendDTMFDirective(std::shared_ptr<DirectiveInfo> info) {
    m_executor.submit([this, info]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(info->directive->getPayload().c_str());
        if (!result) {
            AACE_ERROR(LX(TAG, "handleSendDTMFDirective")
                           .d("reason", rapidjson::GetParseError_En(result.Code()))
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        m_phoneCallController->sendDTMF(info->directive->getPayload());
        setHandlingCompleted(info);
    });
}

void PhoneCallControllerCapabilityAgent::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_ERROR(LX(TAG, "handleDirectiveFailed")
                   .d("reason", "unknownDirective")
                   .d("namespace", info->directive->getNamespace())
                   .d("name", info->directive->getName()));

    m_executor.submit([this, info] {
        const std::string exceptionMessage =
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
        sendExceptionEncounteredAndReportFailed(
            info,
            exceptionMessage,
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    });
}

void PhoneCallControllerCapabilityAgent::onFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newState,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    m_executor.submit([this, newState, behavior]() { executeOnFocusChanged(newState, behavior); });
}
std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
PhoneCallControllerCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void PhoneCallControllerCapabilityAgent::connectionStateChanged(
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state) {
    m_connectionState = state;
    updateContextManager();
}

void PhoneCallControllerCapabilityAgent::callStateChanged(
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallState state,
    const std::string& callId,
    const std::string& callerId) {
    CallState internalState;

    switch (state) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::DIALING:
            internalState = CallState::TRYING;
            break;
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::OUTBOUND_RINGING:
            internalState = CallState::OUTBOUND_RINGING;
            break;
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::ACTIVE:
            internalState = CallState::ACTIVE;
            break;
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::INBOUND_RINGING:
            internalState = CallState::INBOUND_RINGING;
            break;
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::IDLE:
            internalState = CallState::IDLE;
            break;
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::CALL_RECEIVED:
            internalState = CallState::INVITED;
            break;
    }

    m_executor.submit(
        [this, internalState, callId, callerId] { executeCallStateChanged(internalState, callId, callerId); });
}

void PhoneCallControllerCapabilityAgent::callFailed(
    const std::string& callId,
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallError code,
    const std::string& message) {
    m_executor.submit([this, callId, code, message] { executeCallFailed(callId, code, message); });
}

void PhoneCallControllerCapabilityAgent::callerIdReceived(const std::string& callId, const std::string& callerId) {
    m_executor.submit([this, callId, callerId] { executeCallerIdReceived(callId, callerId); });
}
void PhoneCallControllerCapabilityAgent::sendDTMFSucceeded(const std::string& callId) {
    m_executor.submit([this, callId] { executeSendDTMFSucceeded(callId); });
}
void PhoneCallControllerCapabilityAgent::sendDTMFFailed(
    const std::string& callId,
    aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError code,
    const std::string& message) {
    m_executor.submit([this, callId, code, message] { executeSendDTMFFailed(callId, code, message); });
}
void PhoneCallControllerCapabilityAgent::deviceConfigurationUpdated(
    std::unordered_map<
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty,
        bool> configurationMap) {
    m_deviceConfigurationMap = configurationMap;
    updateContextManager();
}
std::string PhoneCallControllerCapabilityAgent::createCallId() {
    return alexaClientSDK::avsCommon::utils::uuidGeneration::generateUUID();
}

void PhoneCallControllerCapabilityAgent::updateContextManager() {
    std::string contextString = getContextString();

    auto setStateSuccess = m_contextManager->setState(
        CONTEXT_MANAGER_PHONE_CONTROL_STATE, contextString, alexaClientSDK::avsCommon::avs::StateRefreshPolicy::NEVER);
    if (setStateSuccess != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS) {
        AACE_ERROR(LX(TAG + ".PhoneCallControllerConnectionState", "updateContextManager")
                       .d("reason", static_cast<int>(setStateSuccess)));
    }
}

std::string PhoneCallControllerCapabilityAgent::getContextString() {
    rapidjson::Document document(rapidjson::kObjectType);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    rapidjson::Value device(rapidjson::kObjectType);
    device.AddMember(
        "connectionState", rapidjson::Value(connectionStateToString(m_connectionState).c_str(), allocator), allocator);
    document.AddMember("device", device, allocator);

    rapidjson::Value configuration(rapidjson::kObjectType);
    rapidjson::Value callingFeature(rapidjson::kArrayType);
    for (auto it : m_deviceConfigurationMap) {
        rapidjson::Value tempConfig(rapidjson::kObjectType);
        tempConfig.AddMember(
            rapidjson::Value(configurationFeatureToString(it.first).c_str(), allocator),
            rapidjson::Value().SetBool(it.second),
            allocator);
        callingFeature.PushBack(tempConfig, allocator);
    }
    rapidjson::Value tempConfig(rapidjson::kObjectType);
    tempConfig.AddMember("OVERRIDE_RINGTONE_SUPPORTED", rapidjson::Value().SetBool(false), allocator);
    callingFeature.PushBack(tempConfig, allocator);

    configuration.AddMember("callingFeature", callingFeature, allocator);
    document.AddMember("configuration", configuration, allocator);

    rapidjson::Value allCalls(rapidjson::kArrayType);
    for (auto it : m_allCallsMap) {
        if (it.second == CallState::IDLE) {
            continue;
        }
        rapidjson::Value tempCall(rapidjson::kObjectType);
        tempCall.AddMember("callId", rapidjson::Value(it.first.c_str(), allocator), allocator);
        tempCall.AddMember("callState", rapidjson::Value(callStateToString(it.second).c_str(), allocator), allocator);
        allCalls.PushBack(tempCall, allocator);
    }
    document.AddMember("allCalls", allCalls, allocator);

    if (callExist(m_currentCallId) && getCallState(m_currentCallId) != CallState::IDLE) {
        rapidjson::Value currentCall(rapidjson::kObjectType);
        currentCall.AddMember("callId", rapidjson::Value(m_currentCallId.c_str(), allocator), allocator);
        document.AddMember("currentCall", currentCall, allocator);
    }

    ThrowIfNot(document.Accept(writer), "failedToWriteJsonDocument");

    return buffer.GetString();
}

void PhoneCallControllerCapabilityAgent::executeOnFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newState,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    switch (newState) {
        case alexaClientSDK::avsCommon::avs::FocusState::BACKGROUND:
            AACE_VERBOSE(LX(TAG, "executeOnFocusChanged").d("focusState", newState));
            break;
        case alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND:
            AACE_VERBOSE(LX(TAG, "executeOnFocusChanged").d("focusState", newState));
            break;
        case alexaClientSDK::avsCommon::avs::FocusState::NONE:
            AACE_VERBOSE(LX(TAG, "executeOnFocusChanged").d("focusState", newState));
            break;
    }
}
void PhoneCallControllerCapabilityAgent::executeCallStateChanged(
    CallState state,
    const std::string& callId,
    const std::string& callerId) {
    if (!callExist(callId)) {
        addCall(callId, CallState::IDLE);
    }
    auto eventName = getEventName(state, callId);
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    m_currentCallId = callId;

    // Context Handling
    if (state == CallState::IDLE) {
        removeCall(callId);
        m_callMethodMap.erase(callId);
    } else {
        setCallState(callId, state);
    }
    updateContextManager();

    // Focus Handling
    if (state != CallState::IDLE) {
        acquireCommunicationsChannelFocus();
    } else {
        releaseCommunicationsChannelFocus();
    }

    if (state == CallState::INVITED) {
        rapidjson::Document allCallsPayload(rapidjson::kArrayType);
        rapidjson::Document receivedCallPayload(rapidjson::kObjectType);

        for (auto it : m_allCallsMap) {
            if (it.second == CallState::IDLE) {
                continue;
            }
            rapidjson::Value tempCall(rapidjson::kObjectType);
            tempCall.AddMember(
                "callId", rapidjson::Value(it.first.c_str(), payload.GetAllocator()), payload.GetAllocator());
            tempCall.AddMember(
                "callState",
                rapidjson::Value(callStateToString(it.second).c_str(), payload.GetAllocator()),
                payload.GetAllocator());
            allCallsPayload.PushBack(tempCall, payload.GetAllocator());
        }
        payload.AddMember("allCalls", allCallsPayload, payload.GetAllocator());
        receivedCallPayload.AddMember(
            "callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
        if (callerId != "") {
            receivedCallPayload.AddMember(
                "callerId", rapidjson::Value(callerId.c_str(), payload.GetAllocator()), payload.GetAllocator());
        }
        payload.AddMember("receivedCall", receivedCallPayload, payload.GetAllocator());
    } else {
        payload.AddMember("callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    }
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto event = buildJsonEventString(eventName, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
    m_messageSender->sendMessage(request);
}

void PhoneCallControllerCapabilityAgent::executeCallFailed(
    const std::string& callId,
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallError code,
    const std::string& message) {
    if (!callExist(callId)) {
        AACE_WARN(LX(TAG, "callFailed").d("reason", "invalid callId"));
        return;
    }

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document errorPayload(rapidjson::kObjectType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    errorPayload.AddMember(
        "code", rapidjson::Value(callErrorToString(code).c_str(), payload.GetAllocator()), payload.GetAllocator());
    errorPayload.AddMember(
        "message", rapidjson::Value(message.c_str(), payload.GetAllocator()), payload.GetAllocator());
    payload.AddMember("callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    payload.AddMember("error", errorPayload, payload.GetAllocator());
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    removeCall(callId);
    m_callMethodMap.erase(callId);
    updateContextManager();
    releaseCommunicationsChannelFocus();

    auto event = buildJsonEventString("CallFailed", "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
    m_messageSender->sendMessage(request);
}

void PhoneCallControllerCapabilityAgent::executeCallerIdReceived(
    const std::string& callId,
    const std::string& callerId) {
    if (!callExist(callId)) {
        AACE_WARN(LX(TAG, "callerIdReceived").d("reason", "invalid callId"));
        return;
    }

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document receivedCallPayload(rapidjson::kObjectType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    receivedCallPayload.AddMember(
        "callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    receivedCallPayload.AddMember(
        "callerId", rapidjson::Value(callerId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    payload.AddMember("receivedCall", receivedCallPayload, payload.GetAllocator());
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto event = buildJsonEventString("CallerIdReceived", "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
    m_messageSender->sendMessage(request);
}

void PhoneCallControllerCapabilityAgent::executeSendDTMFSucceeded(const std::string& callId) {
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    payload.AddMember("callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto event = buildJsonEventString("SendDTMFSucceeded", "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
    m_messageSender->sendMessage(request);
}

void PhoneCallControllerCapabilityAgent::executeSendDTMFFailed(
    const std::string& callId,
    aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError code,
    const std::string& message) {
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document errorPayload(rapidjson::kObjectType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    errorPayload.AddMember(
        "code", rapidjson::Value(dtmfErrorToString(code).c_str(), payload.GetAllocator()), payload.GetAllocator());
    errorPayload.AddMember(
        "message", rapidjson::Value(message.c_str(), payload.GetAllocator()), payload.GetAllocator());
    payload.AddMember("callId", rapidjson::Value(callId.c_str(), payload.GetAllocator()), payload.GetAllocator());
    payload.AddMember("error", errorPayload, payload.GetAllocator());
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto event = buildJsonEventString("SendDTMFFailed", "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
    m_messageSender->sendMessage(request);
}

void PhoneCallControllerCapabilityAgent::acquireCommunicationsChannelFocus() {
    m_executor.submit([this] {
        if (!m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), NAMESPACE)) {
            AACE_ERROR(LX(TAG, "acquireChannel").d("reason", "failedToAcquireChannel"));
        }
    });
}

void PhoneCallControllerCapabilityAgent::releaseCommunicationsChannelFocus() {
    m_executor.submit([this] { m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this()); });
}

std::string PhoneCallControllerCapabilityAgent::getEventName(CallState state, const std::string& callId) {
    std::string eventName;
    switch (state) {
        case CallState::ACTIVE:
            eventName = "CallActivated";
            break;
        case CallState::TRYING:
            if (m_callMethodMap.find(callId) != m_callMethodMap.end() &&
                m_callMethodMap[callId] == CallMethod::REDIAL) {
                eventName = "RedialStarted";
            } else {
                eventName = "DialStarted";
            }
            break;
        case CallState::OUTBOUND_RINGING:
            eventName = "OutboundRingingStarted";
            break;
        case CallState::INBOUND_RINGING:
            eventName = "InboundRingingStarted";
            break;
        case CallState::IDLE:
            eventName = "CallTerminated";
            break;
        case CallState::INVITED:
            eventName = "CallReceived";
            break;
    }
    return eventName;
}

std::string PhoneCallControllerCapabilityAgent::connectionStateToString(
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state) {
    switch (state) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED:
            return "CONNECTED";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "";
    }
}

std::string PhoneCallControllerCapabilityAgent::callStateToString(CallState state) {
    switch (state) {
        case CallState::ACTIVE:
            return "ACTIVE";
        case CallState::IDLE:
            return "IDLE";
        case CallState::INBOUND_RINGING:
            return "INBOUND_RINGING";
        case CallState::INVITED:
            return "INVITED";
        case CallState::OUTBOUND_RINGING:
            return "OUTBOUND_RINGING";
        case CallState::TRYING:
            return "TRYING";
        default:
            return "";
    }
}

std::string PhoneCallControllerCapabilityAgent::configurationFeatureToString(
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty feature) {
    switch (feature) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty::
            DTMF_SUPPORTED:
            return "DTMF_SUPPORTED";
        default:
            return "";
    }
}

std::string PhoneCallControllerCapabilityAgent::callErrorToString(
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallError error) {
    switch (error) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_CARRIER:
            return "NO_CARRIER";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::BUSY:
            return "BUSY";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_ANSWER:
            return "NO_ANSWER";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_NUMBER_FOR_REDIAL:
            return "NO_NUMBER_FOR_REDIAL";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::OTHER:
            return "OTHER";
        default:
            return "";
    }
}
std::string PhoneCallControllerCapabilityAgent::dtmfErrorToString(
    aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError error) {
    switch (error) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError::CALL_NOT_IN_PROGRESS:
            return "CALL_NOT_IN_PROGRESS";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError::DTMF_FAILED:
            return "DTMF_FAILED";
        default:
            return "";
    }
}

void PhoneCallControllerCapabilityAgent::addCall(std::string callId, CallState state) {
    m_allCallsMap[callId] = state;
}

PhoneCallControllerCapabilityAgent::CallState PhoneCallControllerCapabilityAgent::getCallState(std::string callId) {
    return m_allCallsMap[callId];
}

void PhoneCallControllerCapabilityAgent::setCallState(std::string callId, CallState state) {
    m_allCallsMap[callId] = state;
}

void PhoneCallControllerCapabilityAgent::removeCall(std::string callId) {
    m_allCallsMap.erase(callId);
}

bool PhoneCallControllerCapabilityAgent::callExist(std::string callId) {
    return m_allCallsMap.find(callId) != m_allCallsMap.end();
}

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aace
