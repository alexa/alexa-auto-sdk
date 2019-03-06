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

#include <string>
#include <rapidjson/error/en.h>
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

/// Navigation capability constants
/// Navigation interface type
static const std::string NAVIGATION_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Navigation interface name
static const std::string NAVIGATION_CAPABILITY_INTERFACE_NAME = "Navigation";
/// Navigation interface version
static const std::string NAVIGATION_CAPABILITY_INTERFACE_VERSION = "1.2";

/**
 * Creates the Navigation capability configuration.
 *
 * @return The Navigation capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getNavigationCapabilityConfiguration();

std::shared_ptr<NavigationCapabilityAgent> NavigationCapabilityAgent::create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender )
{
    try
    {
        ThrowIfNull( exceptionSender, "nullExceptionSender" );
        
        auto navigationCapabilityAgent = std::shared_ptr<NavigationCapabilityAgent>( new NavigationCapabilityAgent( exceptionSender ) );
        
        return navigationCapabilityAgent;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationCapabilityAgent::handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) {
    preHandleDirective( std::make_shared<DirectiveInfo>( directive, nullptr ) );
}

void NavigationCapabilityAgent::preHandleDirective( std::shared_ptr<DirectiveInfo> info )
{
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
        AACE_ERROR(LX(TAG,"preHandleDirective").d("reason", ex.what()));
    }
}

void NavigationCapabilityAgent::handleDirective( std::shared_ptr<DirectiveInfo> info ) {
    // Do nothing here as directives are handled in the preHandle stage.
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

NavigationCapabilityAgent::NavigationCapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) :
    alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
    alexaClientSDK::avsCommon::utils::RequiresShutdown{"NavigationCapabilityAgent"} {
    m_capabilityConfigurations.insert(getNavigationCapabilityConfiguration());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getNavigationCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, NAVIGATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, NAVIGATION_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, NAVIGATION_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
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

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> NavigationCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

} // aace::engine::navigation
} // aace::engine
} // aace

