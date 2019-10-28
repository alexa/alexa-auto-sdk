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

#include "AACE/Engine/VPA/VPACapabilityAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace vpa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.vpa.VPACapabilityAgent");

/// The namespace for this capability agent.
static const std::string NAMESPACE{"*"};

/// The name for RenderTemplate directive.
static const std::string NAME{"*"};

/// The RenderTemplate directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ALL{NAMESPACE, NAME};

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

std::shared_ptr<VPACapabilityAgent> VPACapabilityAgent::create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    auto vpaCapabilityAgent = std::shared_ptr<VPACapabilityAgent>( new VPACapabilityAgent( exceptionSender ));
    ThrowIfNull( vpaCapabilityAgent, "nullVPACapabilityAgent");
    return vpaCapabilityAgent;
}

void VPACapabilityAgent::handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) {
    handleSendDirective( std::make_shared<DirectiveInfo>( directive, nullptr ) );
}

void VPACapabilityAgent::preHandleDirective( std::shared_ptr<DirectiveInfo> info )
{
    // Intentional no-op (removeDirective() can only be called from handleDirective() and cancelDirective() functions).
}

void VPACapabilityAgent::handleDirective( std::shared_ptr<DirectiveInfo> info ) 
{
    AACE_ERROR(LX(TAG,"Shout not be called"));
}

void VPACapabilityAgent::cancelDirective( std::shared_ptr<DirectiveInfo> info ) {
    removeDirective( info );
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration VPACapabilityAgent::getConfiguration() const
{
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy( alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_NONE, false);
    configuration[ALL] = audioVisualBlockingPolicy;
    return configuration;    
}

void VPACapabilityAgent::addObserver( std::shared_ptr<VPAObserverInterface> observer )
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

void VPACapabilityAgent::removeObserver( std::shared_ptr<VPAObserverInterface> observer )
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

VPACapabilityAgent::VPACapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) :
    alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
    alexaClientSDK::avsCommon::utils::RequiresShutdown{"VPACapabilityAgent"} {
    m_capabilityConfigurations.insert( getGeolocationCapabilityConfiguration() );
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getGeolocationCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, GEOLOCATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, GEOLOCATION_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, GEOLOCATION_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void VPACapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_observers.clear();
}

void VPACapabilityAgent::removeDirective( std::shared_ptr<DirectiveInfo> info ) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if( info->directive && info->result ) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective( info->directive->getMessageId() );
    }
}

void VPACapabilityAgent::setHandlingCompleted( std::shared_ptr<DirectiveInfo> info )
{
    if( info && info->result ) {
        info->result->setCompleted();
    }
    
    removeDirective( info );
}

void VPACapabilityAgent::handleSendDirective( std::shared_ptr<DirectiveInfo> info )
{
    m_executor.submit([this, info]() {
        for( auto observer : m_observers ) {
            observer->sendDirective( info->directive->getUnparsedDirective() );
        }
        setHandlingCompleted( info );
    });
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> VPACapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

} // aace::engine::navigation
} // aace::engine
} // aace

