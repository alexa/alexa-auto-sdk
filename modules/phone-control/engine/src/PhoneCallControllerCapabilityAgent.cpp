/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

static const std::string NAMESPACE{"PhoneCallController"};

static const alexaClientSDK::avsCommon::avs::NamespaceAndName DIAL{NAMESPACE, "Dial"};

static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_NAME = "Alexa.Comms.PhoneCallController";
static const std::string PHONE_CONTROL_CAPABILITY_INTERFACE_VERSION = "1.0";

static const alexaClientSDK::avsCommon::avs::NamespaceAndName CONTEXT_MANAGER_PHONE_CONTROL_STATE{NAMESPACE, "PhoneCallControllerState"};

static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getPhoneCallControllerCapabilityConfiguration();
    
std::shared_ptr<PhoneCallControllerCapabilityAgent> PhoneCallControllerCapabilityAgent::create (
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender ) {
    
    try {
        ThrowIfNull( contextManager, "nullContextManager" );
        ThrowIfNull( exceptionSender, "nullExceptionSender" );
        ThrowIfNull ( messageSender, "nullMessageSender" );
        
        auto phoneCallControllerCapabilityAgent = std::shared_ptr<PhoneCallControllerCapabilityAgent>( new PhoneCallControllerCapabilityAgent ( contextManager, exceptionSender, messageSender ) );
        
        return phoneCallControllerCapabilityAgent;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}
    
void PhoneCallControllerCapabilityAgent::handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) {
    // Do nothing here as directives are handled in the handle stage.
}
    
void PhoneCallControllerCapabilityAgent::preHandleDirective( std::shared_ptr<DirectiveInfo> info ) {
    // Do nothing here as directives are handled in the handle stage.
}
    
void PhoneCallControllerCapabilityAgent::handleDirective( std::shared_ptr<DirectiveInfo> info ) {
    try {
        ThrowIfNot( info && info->directive, "nullDirectiveInfo" );
        
        if( info->directive->getName() == DIAL.name ) {
            handleDialDirective( info );
        }
        else {
            handleUnknownDirective( info );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleDirective").d("reason", ex.what()));
    }
}
    
void PhoneCallControllerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info ) {
    removeDirective( info );
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration PhoneCallControllerCapabilityAgent::getConfiguration() const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    configuration[DIAL] = alexaClientSDK::avsCommon::avs::BlockingPolicy::NON_BLOCKING;
    return configuration;
}

void PhoneCallControllerCapabilityAgent::addObserver( std::shared_ptr<PhoneCallControllerObserverInterface> observer ) {
    try {
        ThrowIfNull(observer,"observerIsNull");
        
        m_executor.submit([this, observer]() {
            if(!m_observers.insert( observer ).second) {
                AACE_ERROR(LX(TAG,"addObserverInExecutor").m("duplicateObserver"));
            }
        });
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"addObserver").d("reason", ex.what()));
    }
}

void PhoneCallControllerCapabilityAgent::removeObserver( std::shared_ptr<PhoneCallControllerObserverInterface> observer ) {
    try {
        ThrowIfNull(observer,"observerIsNull");
        
        m_executor.submit([this, observer]() {
            if(!m_observers.insert( observer ).second) {
                AACE_ERROR(LX(TAG,"removeObserverInExecutor").m("observerNotInList"));
            }
        });
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"removeObserver").d("reason", ex.what()));
    }
}

PhoneCallControllerCapabilityAgent::PhoneCallControllerCapabilityAgent (
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender ) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"PhoneCallControllerCapabilityAgent"},
        m_contextManager{contextManager},
        m_messageSender{messageSender} {
    m_capabilityConfigurations.insert(getPhoneCallControllerCapabilityConfiguration());
    m_connectionState = aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED;
    updateContextManager();
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getPhoneCallControllerCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, PHONE_CONTROL_CAPABILITY_INTERFACE_VERSION});
    
    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void PhoneCallControllerCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_observers.clear();
    m_messageSender.reset();
    m_contextManager.reset();
}

void PhoneCallControllerCapabilityAgent::sendExceptionEncounteredAndReportFailed( std::shared_ptr<DirectiveInfo> info, const std::string& message, alexaClientSDK::avsCommon::avs::ExceptionErrorType type ) {
    m_exceptionEncounteredSender->sendExceptionEncountered( info->directive->getUnparsedDirective(), type, message );
    
    if( info && info->result ) {
        info->result->setFailed( message );
    }
    
    removeDirective( info );
}
    
void PhoneCallControllerCapabilityAgent::removeDirective( std::shared_ptr<DirectiveInfo> info ) {
    if( info->directive && info->result ) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective( info->directive->getMessageId() );
    }
}

void PhoneCallControllerCapabilityAgent::setHandlingCompleted( std::shared_ptr<DirectiveInfo> info) {
    if( info && info->result ) {
        info->result->setCompleted();
    }
    removeDirective( info );
}

void PhoneCallControllerCapabilityAgent::handleDialDirective( std::shared_ptr<DirectiveInfo> info ) {
    m_executor.submit([this, info]() {
        std::string callId;
        std::string phoneNumber;
        std::string details = "";
        
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse( info->directive->getPayload().c_str() );
        if( !result ) {
            AACE_ERROR(LX(TAG, "handleDialDirective").d("reason", rapidjson::GetParseError_En(result.Code())).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed( info, "Unable to parse payload", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        if( !document.HasMember( "callId" ) ) {
            AACE_ERROR(LX(TAG, "handleDialDirective").d("reason", "missing callId"));
            sendExceptionEncounteredAndReportFailed( info, "Missing callId", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }
        callId = document["callId"].GetString();
        m_currentActiveCallId = callId;

        for( auto observer : m_observers ) {
            if( observer->dial( info->directive->getPayload() ) ) {
                addCall( callId, CallState::IDLE );
                updateContextManager();
            }
        }
        setHandlingCompleted( info );
    });
}

void PhoneCallControllerCapabilityAgent::handleUnknownDirective( std::shared_ptr<DirectiveInfo> info ) {
    AACE_ERROR(LX(TAG,"handleDirectiveFailed")
               .d("reason", "unknownDirective")
               .d("namespace", info->directive->getNamespace())
               .d("name", info->directive->getName()));
    
    m_executor.submit([this, info] {
        const std::string exceptionMessage =
        "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
        sendExceptionEncounteredAndReportFailed( info, exceptionMessage, alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED );
    });
}
    
std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> PhoneCallControllerCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}
    
void PhoneCallControllerCapabilityAgent::connectionStateChanged( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state ) {
    m_connectionState = state;
    updateContextManager();
}
    
void PhoneCallControllerCapabilityAgent::callActivated( const std::string& callId ) {
    m_executor.submit( [this, callId] {
        executeCallActivated( callId );
    });
}
    
void PhoneCallControllerCapabilityAgent::callFailed( const std::string& callId, const std::string& error, const std::string& message ) {
    m_executor.submit( [this, callId, error, message] {
        executeCallFailed( callId, error, message );
    });
}
    
void PhoneCallControllerCapabilityAgent::callTerminated( const std::string& callId ) {
    m_executor.submit( [this, callId] {
        executeCallTerminated( callId );
    });
}

void PhoneCallControllerCapabilityAgent::updateContextManager() {
    std::string contextString = getContextString();

    auto setStateSuccess = m_contextManager->setState( CONTEXT_MANAGER_PHONE_CONTROL_STATE, contextString, alexaClientSDK::avsCommon::avs::StateRefreshPolicy::NEVER );
    if ( setStateSuccess != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS ) {
        AACE_ERROR(LX(TAG + ".PhoneCallControllerConnectionState","updateContextManager").d("reason", static_cast<int>(setStateSuccess)));
    }
}
    
std::string PhoneCallControllerCapabilityAgent::getContextString() {
    rapidjson::Document document( rapidjson::kObjectType );

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember( "connectionState", rapidjson::Value( getConnectionState( m_connectionState ).c_str(), allocator ), allocator );

    rapidjson::Value allCalls( rapidjson::kArrayType );
    for ( auto it : m_allCallsMap ) {
        if( it.second == CallState::IDLE ) {
            continue;
        }
        rapidjson::Value tempCall( rapidjson::kObjectType );
        tempCall.AddMember( "callId", rapidjson::Value( it.first.c_str(), allocator ), allocator );
        tempCall.AddMember( "callState", rapidjson::Value( getCallState( it.second ).c_str(), allocator ), allocator);
        allCalls.PushBack( tempCall, allocator );
    }
    document.AddMember( "allCalls", allCalls, allocator );

    if ( callExist( m_currentActiveCallId ) && getCallState( m_currentActiveCallId ) == CallState::ACTIVE ) {
        rapidjson::Value activeCall( rapidjson::kObjectType );
        activeCall.AddMember( "callID", rapidjson::Value( m_currentActiveCallId.c_str(), allocator ), allocator );
        document.AddMember( "activeCall", activeCall, allocator );
    }

    ThrowIfNot( document.Accept( writer ), "failedToWriteJsonDocument" );

    return buffer.GetString();
}
    
void PhoneCallControllerCapabilityAgent::executeCallActivated( const std::string& callId ) {
    if ( !callExist( callId ) ) {
        addCall( callId, CallState::IDLE );
    }

    rapidjson::Document payload( rapidjson::kObjectType );
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );
    
    payload.AddMember( "callId", rapidjson::Value( callId.c_str(), payload.GetAllocator() ), payload.GetAllocator() );
    ThrowIfNot( payload.Accept( writer ), "failedToWriteJsonDocument" );
    
    m_currentActiveCallId = callId;
    setCallState( callId, CallState::ACTIVE );
    updateContextManager();

    auto event = buildJsonEventString( "CallActivated", "", buffer.GetString() );
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event.second );
    m_messageSender->sendMessage( request );
}

void PhoneCallControllerCapabilityAgent::executeCallFailed( const std::string& callId, const std::string& error, const std::string& message ) {
    if ( !callExist( callId ) ) {
        AACE_WARN(LX(TAG, "callFailed").d("reason", "invalid callId"));
        return;
    }

    rapidjson::Document payload( rapidjson::kObjectType );
    rapidjson::Document errorPayload( rapidjson::kObjectType );
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );

    errorPayload.AddMember( "code", rapidjson::Value( error.c_str(), payload.GetAllocator() ), payload.GetAllocator() );
    errorPayload.AddMember( "message", rapidjson::Value( message.c_str(), payload.GetAllocator() ), payload.GetAllocator() );
    payload.AddMember( "callId", rapidjson::Value( callId.c_str(), payload.GetAllocator() ), payload.GetAllocator() );
    payload.AddMember( "error", errorPayload, payload.GetAllocator());
    ThrowIfNot( payload.Accept( writer ), "failedToWriteJsonDocument" );

    setCallState( callId, CallState::IDLE );
    updateContextManager();

    auto event = buildJsonEventString( "CallFailed", "", buffer.GetString() );
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event.second );
    m_messageSender->sendMessage( request );
}
    
void PhoneCallControllerCapabilityAgent::executeCallTerminated( const std::string& callId ) {
    if ( !callExist( callId ) ) {
        AACE_WARN(LX(TAG, "callTerminated").d("reason", "invalid callId"));
        return;
    }

    rapidjson::Document payload( rapidjson::kObjectType );
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );
    
    payload.AddMember( "callId", rapidjson::Value( callId.c_str(), payload.GetAllocator() ), payload.GetAllocator() );
    ThrowIfNot( payload.Accept( writer ), "failedToWriteJsonDocument" );
    
    removeCall( callId );
    updateContextManager();

    auto event = buildJsonEventString("CallTerminated", "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event.second );
    m_messageSender->sendMessage( request );
}
    
std::string PhoneCallControllerCapabilityAgent::getConnectionState( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state ) {
    switch( state ) {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED:
            return "CONNECTED";
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "";
    }
}

std::string PhoneCallControllerCapabilityAgent::getCallState ( CallState state ) {
    switch( state ) {
        case CallState::ACTIVE:
            return "ACTIVE";
        case CallState::IDLE:
            return "IDLE";
        default:
            return "";
    }
}

void PhoneCallControllerCapabilityAgent::addCall( std::string callId, CallState state ) {
    m_allCallsMap[callId] = state;
}

PhoneCallControllerCapabilityAgent::CallState PhoneCallControllerCapabilityAgent::getCallState( std::string callId ) {
    return m_allCallsMap[callId];
}

void PhoneCallControllerCapabilityAgent::setCallState( std::string callId, CallState state ) {
    m_allCallsMap[callId] = state;
}

void PhoneCallControllerCapabilityAgent::removeCall( std::string callId ) {
    m_allCallsMap.erase( callId );
}

bool PhoneCallControllerCapabilityAgent::callExist( std::string callId ) {
    return m_allCallsMap.find( callId ) != m_allCallsMap.end();
}

} // aace::engine::phoneCallController
} // aace::engine
} // aace
