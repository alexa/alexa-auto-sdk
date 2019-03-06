/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/PhoneCallController/PhoneCallControllerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace phoneCallController {

// String to identify log entries originating from this file.
static const std::string TAG("aace.phoneCallController.PhoneCallControllerEngineImpl");

PhoneCallControllerEngineImpl::PhoneCallControllerEngineImpl( std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_phoneCallControllerPlatformInterface( phoneCallControllerPlatformInterface ) {
}

bool PhoneCallControllerEngineImpl::initialize (
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager ) {

    try {
        m_phoneCallControllerCapabilityAgent = PhoneCallControllerCapabilityAgent::create( shared_from_this(), contextManager, exceptionSender, messageSender, focusManager );
        ThrowIfNull( m_phoneCallControllerCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add capability agent to the directive sequencer 
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_phoneCallControllerCapabilityAgent ), "addDirectiveHandlerFailed" );

        m_phoneCallControllerPlatformInterface->setEngineInterface( std::dynamic_pointer_cast<aace::phoneCallController::PhoneCallControllerEngineInterface>( shared_from_this() ) );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_phoneCallControllerCapabilityAgent ), "registerCapabilityFailed");

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PhoneCallControllerEngineImpl> PhoneCallControllerEngineImpl::create(
    std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface, 
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager ) {

    try {
        ThrowIfNull( phoneCallControllerPlatformInterface, "nullPlatformInterface" );
        ThrowIfNull( capabilitiesDelegate, "nullCapabilitiesDelegate" );
        ThrowIfNull( contextManager, "nullContextManager" );
        ThrowIfNull( directiveSequencer, "nullDirectiveSequencer" );
        ThrowIfNull( exceptionSender, "nullExceptionSender" );
        ThrowIfNull( messageSender, "nullMessageSender" );
        ThrowIfNull( focusManager, "nullFocusManager" );

        std::shared_ptr<PhoneCallControllerEngineImpl> phoneCallControllerEngineImpl = std::shared_ptr<PhoneCallControllerEngineImpl>( new PhoneCallControllerEngineImpl( phoneCallControllerPlatformInterface ) );

        ThrowIfNot( phoneCallControllerEngineImpl->initialize( capabilitiesDelegate, contextManager, directiveSequencer, exceptionSender, messageSender, focusManager ), "initializePhoneCallControllerEngineImplFailed" );

        return phoneCallControllerEngineImpl;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void PhoneCallControllerEngineImpl::doShutdown() {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->shutdown();
        m_phoneCallControllerCapabilityAgent.reset();

        if( m_phoneCallControllerPlatformInterface != nullptr ) {
            m_phoneCallControllerPlatformInterface->setEngineInterface( nullptr );
        }
    }
}

void PhoneCallControllerEngineImpl::onConnectionStateChanged( ConnectionState state  ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->connectionStateChanged( state );
    } 
}

void PhoneCallControllerEngineImpl::onCallStateChanged ( CallState state, const std::string& callId, const std::string& callerId ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callStateChanged( state, callId, callerId );
    } 
}

void PhoneCallControllerEngineImpl::onCallFailed( const std::string& callId, CallError code, const std::string& message ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callFailed( callId, code, message );
    }
}

void PhoneCallControllerEngineImpl::onCallerIdReceived( const std::string& callId, const std::string& callerId ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callerIdReceived( callId, callerId );
    }
}

void PhoneCallControllerEngineImpl::onSendDTMFSucceeded( const std::string& callId) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->sendDTMFSucceeded( callId );
    }
}

void PhoneCallControllerEngineImpl::onSendDTMFFailed( const std::string& callId, DTMFError code, const std::string& message ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->sendDTMFFailed( callId, code, message );
    }
}

void PhoneCallControllerEngineImpl::onDeviceConfigurationUpdated( std::unordered_map<PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool> configurationMap ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->deviceConfigurationUpdated( configurationMap );
    }
}
std::string PhoneCallControllerEngineImpl::onCreateCallId() {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        return m_phoneCallControllerCapabilityAgent->createCallId();
    }
    return "";
}

bool PhoneCallControllerEngineImpl::dial( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        return m_phoneCallControllerPlatformInterface->dial( payload );
    }
    return false;
}

bool PhoneCallControllerEngineImpl::redial( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        return m_phoneCallControllerPlatformInterface->redial( payload );
    }
    return false;
}

void PhoneCallControllerEngineImpl::answer( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        m_phoneCallControllerPlatformInterface->answer( payload );
    }
}

void PhoneCallControllerEngineImpl::stop( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        m_phoneCallControllerPlatformInterface->stop( payload );
    }
}

void PhoneCallControllerEngineImpl::playRingtone( const std::string& payload ) {
}

void PhoneCallControllerEngineImpl::sendDTMF( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        m_phoneCallControllerPlatformInterface->sendDTMF( payload );
    }
}

} // aace::engine::phoneCallController
} // aace::engine
} // aace
