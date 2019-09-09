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
#include <iostream>

#include <string>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include "AACE/Engine/Navigation/NavigationCapabilityAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace navigation {

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.NavigationCapabilityAgent");

/// The namespace for this capability agent.
static const std::string NAMESPACE{"Navigation"};

/// The SetDestination directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SET_DESTINATION{NAMESPACE, "SetDestination"};

/// The CancelNavigation directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName CANCEL_NAVIGATION{NAMESPACE, "CancelNavigation"};

/// The NavigationState context signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName NAVIGATION_STATE{NAMESPACE, "NavigationState"};

/// Navigation capability constants
/// Navigation interface type
static const std::string NAVIGATION_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Navigation interface name
static const std::string NAVIGATION_CAPABILITY_INTERFACE_NAME = "Navigation";
/// Navigation interface version
static const std::string NAVIGATION_CAPABILITY_INTERFACE_VERSION = "1.3"; 
/// Navigation interface provider name key
static const std::string CAPABILITY_INTERFACE_NAVIGATION_PROVIDER_NAME_KEY = "provider";

/// NavigationState state accepted values
static const std::string NAVIGATION_STATE_NAVIGATING = "NAVIGATING";
static const std::string NAVIGATION_STATE_NOT_NAVIGATING = "NOT_NAVIGATING";
static const std::string NAVIGATION_STATE_UNKNOWN = "UNKNOWN";

/// Default when provided NavigationState is empty
// clang-format off
static const std::string DEFAULT_NAVIGATION_STATE_PAYLOAD = R"({
	"state": ")" + NAVIGATION_STATE_NOT_NAVIGATING + R"(",
	"waypoints": [],
	"shapes": []
})";

// max number of shapes allowable in context
static const int MAXIMUM_SHAPES_IN_CONTEXT = 100;

/// Geolocation capability constants
/// Geolocation interface type
static const std::string GEOLOCATION_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Geolocation interface name
static const std::string GEOLOCATION_CAPABILITY_INTERFACE_NAME = "Geolocation";
/// Geolocation interface version
static const std::string GEOLOCATION_CAPABILITY_INTERFACE_VERSION = "1.1";

/**
 * Creates the Geolocation capability configuration.
 *
 * @return The Geolocation capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getGeolocationCapabilityConfiguration();

/**
 * Creates the Navigation capability configuration.
 *
 * @return The Navigation capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getNavigationCapabilityConfiguration(const std::string& navigationProviderName);

/**
 * Creates the Navigation capability configurations payload.
 *
 * @return The Navigation capability configurations JSON string payload.
 */
static std::string getProviderNamePayload(const std::string& navigationProviderName);

std::shared_ptr<NavigationCapabilityAgent> NavigationCapabilityAgent::create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager, const std::string& navigationProviderName)
{
    try
    {
        ThrowIfNull( exceptionSender, "nullExceptionSender" );
        
        auto navigationCapabilityAgent = std::shared_ptr<NavigationCapabilityAgent>( new NavigationCapabilityAgent( exceptionSender, contextManager, navigationProviderName ) );

        ThrowIfNull( navigationCapabilityAgent, "nullNavigationCapabilityAgent");

        contextManager->setStateProvider( NAVIGATION_STATE, navigationCapabilityAgent);
        
        return navigationCapabilityAgent;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationCapabilityAgent::handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) {
    handleDirective( std::make_shared<DirectiveInfo>( directive, nullptr ) );
}

void NavigationCapabilityAgent::preHandleDirective( std::shared_ptr<DirectiveInfo> info )
{
    // Intentional no-op (removeDirective() can only be called from handleDirective() and cancelDirective() functions).
}

void NavigationCapabilityAgent::handleDirective( std::shared_ptr<DirectiveInfo> info ) {
    try
    {
        ThrowIfNot( info && info->directive, "nullDirectiveInfo" );

        if( info->directive->getName() == SET_DESTINATION.name ) {
            handleSetDestinationDirective( info );
        }
        else if( info->directive->getName() == CANCEL_NAVIGATION.name ) {
            handleCancelNavigationDirective( info );
        }
        else {
            handleUnknownDirective( info );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleDirective").d("reason", ex.what()));
    }
}

void NavigationCapabilityAgent::cancelDirective( std::shared_ptr<DirectiveInfo> info ) {
    removeDirective( info );
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration NavigationCapabilityAgent::getConfiguration() const
{
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy( alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[SET_DESTINATION] = audioVisualBlockingPolicy;
    configuration[CANCEL_NAVIGATION] = audioVisualBlockingPolicy;
    return configuration;
}

void NavigationCapabilityAgent::addObserver( std::shared_ptr<NavigationObserverInterface> observer )
{
    try
    {
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

void NavigationCapabilityAgent::removeObserver( std::shared_ptr<NavigationObserverInterface> observer )
{
    try
    {
        ThrowIfNull(observer,"observerIsNull");

        m_executor.submit([this, observer]() {
            if(!m_observers.erase( observer )) {
                AACE_ERROR(LX(TAG,"removeObserverInExecutor").m("observerNotInList"));
            }
        });
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"removeObserver").d("reason", ex.what()));
    }
}

NavigationCapabilityAgent::NavigationCapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager, const std::string& navigationProviderName ) :
    alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
    alexaClientSDK::avsCommon::utils::RequiresShutdown{"NavigationCapabilityAgent"},
    m_contextManager{ contextManager } {
    m_capabilityConfigurations.insert( getGeolocationCapabilityConfiguration() );
    m_capabilityConfigurations.insert( getNavigationCapabilityConfiguration( navigationProviderName ) );
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getGeolocationCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, GEOLOCATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, GEOLOCATION_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, GEOLOCATION_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getNavigationCapabilityConfiguration(const std::string& navigationProviderName) {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, NAVIGATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, NAVIGATION_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, NAVIGATION_CAPABILITY_INTERFACE_VERSION});

    if (navigationProviderName.empty()) {
        AACE_WARN(LX(TAG,"getNavigationCapabilityConfigurationWarning").d("reason", "navigationProviderNameEmpty"));
        return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
    }

    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, getProviderNamePayload(navigationProviderName)});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

std::string getProviderNamePayload(const std::string& navigationProviderName) {
    return "{ \"" + CAPABILITY_INTERFACE_NAVIGATION_PROVIDER_NAME_KEY + "\" : \"" + navigationProviderName + "\" }";
}

void NavigationCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_observers.clear();
}

void NavigationCapabilityAgent::sendExceptionEncounteredAndReportFailed( std::shared_ptr<DirectiveInfo> info, const std::string& message, alexaClientSDK::avsCommon::avs::ExceptionErrorType type )
{
    m_exceptionEncounteredSender->sendExceptionEncountered( info->directive->getUnparsedDirective(), type, message );
    
    if( info && info->result ) {
        info->result->setFailed( message );
    }
    
    removeDirective( info );
}

void NavigationCapabilityAgent::removeDirective( std::shared_ptr<DirectiveInfo> info ) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if( info->directive && info->result ) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective( info->directive->getMessageId() );
    }
}

void NavigationCapabilityAgent::setHandlingCompleted( std::shared_ptr<DirectiveInfo> info )
{
    if( info && info->result ) {
        info->result->setCompleted();
    }
    
    removeDirective( info );
}

void NavigationCapabilityAgent::handleSetDestinationDirective( std::shared_ptr<DirectiveInfo> info )
{
    m_executor.submit([this, info]() {
        for( auto observer : m_observers ) {
            observer->setDestination( info->directive->getPayload() );
        }
        setHandlingCompleted( info );
    });
}

void NavigationCapabilityAgent::handleCancelNavigationDirective( std::shared_ptr<DirectiveInfo> info )
{
    m_executor.submit([this, info]() {
        for( auto observer : m_observers ) {
            observer->cancelNavigation();
        }
        setHandlingCompleted( info );
    });
}

void NavigationCapabilityAgent::handleUnknownDirective( std::shared_ptr<DirectiveInfo> info )
{
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

void NavigationCapabilityAgent::provideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken ) {
     m_executor.submit( [this, stateProviderName, stateRequestToken] {
        executeProvideState( stateProviderName, stateRequestToken );
    });
}

void NavigationCapabilityAgent::executeProvideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken )
{
    try
    {
        ThrowIfNull( m_contextManager, "contextManagerIsNull" );
        bool payloadChanged = true; // change to false PENDING cloud service bugfix
        std::string payload;
        for( auto observer : m_observers ) { // should only be one
            payload = observer->getNavigationState();
            
            if( payload.empty() /* && m_navigationStatePayload.compare(DEFAULT_NAVIGATION_STATE_PAYLOAD) != 0*/ ) {// uncomment PENDING cloud service bugfix
                payload = DEFAULT_NAVIGATION_STATE_PAYLOAD;
                payloadChanged = true;
            } else if ( !payload.empty() && payload.compare( m_navigationStatePayload ) != 0 ){ 
                payloadChanged = true;
            }
        }
        
        if( payloadChanged ) { 
            AACE_INFO(LX(TAG,"provideState").d("payload",payload)); 
            if ( isNavigationStateValid( payload )  ) {
                // set the context NavigationState
                ThrowIf( m_contextManager->setState( NAVIGATION_STATE, m_navigationStatePayload, alexaClientSDK::avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken ) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS, "contextManagerSetStateFailed" );
            }
        } else {
            // send empty if no change
            ThrowIf( m_contextManager->setState( NAVIGATION_STATE, "", alexaClientSDK::avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken ) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS, "contextManagerSetStateEmptyPayloadFailed" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG + ".AlexaEngineLocationStateProvider","executeProvideState").d("reason", ex.what()));
    }
}

bool NavigationCapabilityAgent::isNavigationStateValid( std::string navigationState ) 
{
    try 
    { 
        rapidjson::Document document;
        document.Parse<0>( navigationState.c_str() );
        
        if( document.HasParseError() ) {
            rapidjson::ParseErrorCode ok = document.GetParseError();
            AACE_ERROR(LX( TAG,"isNavigationStateValid" ).d( "HasParseError", GetParseError_En(ok) ) );
            Throw( "parseError" );
        }
        
        ThrowIfNot( document.HasMember("state"), "stateKeyMissing");

        if( document[ "state" ].IsNull() || !document[ "state" ].IsString() ) {
            Throw( "stateNotValid" );
        }
        std::string state = document[ "state" ].GetString();
        if( ( state.compare(NAVIGATION_STATE_NAVIGATING) != 0 ) 
            && ( state.compare(NAVIGATION_STATE_NOT_NAVIGATING) != 0 )
            && ( state.compare(NAVIGATION_STATE_UNKNOWN) != 0 ) ) {
            Throw( "stateValueNotValid" );
        }

        ThrowIfNot( document.HasMember("waypoints"), "waypointsKeyMissing" );

        if( !document[ "waypoints" ].IsArray() ) {
            Throw( "waypointsArrayNotValid" );
        }

        ThrowIfNot( document.HasMember("shapes"), "shapesKeyMissing" );

        if( !document[ "shapes" ].IsArray() ) {
            Throw( "shapesArrayNotValid" );
        }
        if( document[ "shapes" ].Size() > MAXIMUM_SHAPES_IN_CONTEXT ) {
            AACE_WARN(LX(TAG, "isNavigationStateValid").d("shapes", "Too many shapes in payload. Only using first 100."));

            ThrowIfNot( document[ "shapes" ].Erase(document[ "shapes" ].Begin() + MAXIMUM_SHAPES_IN_CONTEXT, document[ "shapes" ].End()), "unable to operate on shapes Array" );

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            navigationState = buffer.GetString();
        }
    
        // set current navigation state payload
        m_navigationStatePayload = navigationState;

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX( TAG,"isNavigationStateValid" ).d( "reason", ex.what() ) );
        return false;
    }
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> NavigationCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

} // aace::engine::navigation
} // aace::engine
} // aace

