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
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender ) {
    
    try {
        m_phoneCallControllerCapabilityAgent = PhoneCallControllerCapabilityAgent::create( contextManager, exceptionSender, messageSender );
        ThrowIfNull( m_phoneCallControllerCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add phoneCallController runtime observer
        m_phoneCallControllerCapabilityAgent->addObserver( shared_from_this() );

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
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender ) {

    try {
        ThrowIfNull( phoneCallControllerPlatformInterface, "couldNotCreatePhoneCallControllerEngineImpl" );
        ThrowIfNull( capabilitiesDelegate, "couldNotCreatePhoneCallControllerEngineImpl" );
        ThrowIfNull( contextManager, "couldNotCreatePhoneCallControllerEngineImpl" );
        ThrowIfNull( directiveSequencer, "couldNotCreatePhoneCallControllerEngineImpl" );
        ThrowIfNull( exceptionSender, "couldNotCreatePhoneCallControllerEngineImpl" );
        ThrowIfNull( messageSender, "couldNotCreatePhoneCallControllerEngineImpl" );

        std::shared_ptr<PhoneCallControllerEngineImpl> phoneCallControllerEngineImpl = std::shared_ptr<PhoneCallControllerEngineImpl>( new PhoneCallControllerEngineImpl( phoneCallControllerPlatformInterface ) );

        ThrowIfNot( phoneCallControllerEngineImpl->initialize( capabilitiesDelegate, contextManager, directiveSequencer, exceptionSender, messageSender ), "initializePhoneCallControllerEngineImplFailed" );

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
    }
}

void PhoneCallControllerEngineImpl::onConnectionStateChanged( ConnectionState state  ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->connectionStateChanged( state );
    } 
}
void PhoneCallControllerEngineImpl::onCallActivated( const std::string& callId ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callActivated( callId );
    }
}
void PhoneCallControllerEngineImpl::onCallFailed( const std::string& callId, const std::string& error, const std::string& message ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callFailed( callId, error, message );
    }
}
    
void PhoneCallControllerEngineImpl::onCallTerminated( const std::string& callId ) {
    if( m_phoneCallControllerCapabilityAgent != nullptr ) {
        m_phoneCallControllerCapabilityAgent->callTerminated( callId );
    }
}

bool PhoneCallControllerEngineImpl::dial( const std::string& payload ) {
    if( m_phoneCallControllerPlatformInterface != nullptr ) {
        return m_phoneCallControllerPlatformInterface->dial( payload );
    }
    return false;
}

} // aace::engine::phoneCallController
} // aace::engine
} // aace
