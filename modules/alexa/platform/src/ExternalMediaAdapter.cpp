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

#include "AACE/Alexa/ExternalMediaAdapter.h"

namespace aace {
namespace alexa {

ExternalMediaAdapter::ExternalMediaAdapter( std::shared_ptr<aace::alexa::Speaker> speaker ) : m_speaker( speaker ) {
}

ExternalMediaAdapter::~ExternalMediaAdapter() = default;

std::shared_ptr<aace::alexa::Speaker> ExternalMediaAdapter::ExternalMediaAdapter::getSpeaker() {
    return m_speaker;
}

//
// Engine interface methods
//

void ExternalMediaAdapter::reportDiscoveredPlayers( const std::vector<DiscoveredPlayerInfo>& discoveredPlayers ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onReportDiscoveredPlayers( discoveredPlayers );
    }
}

void ExternalMediaAdapter::requestToken( const std::string& localPlayerId ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onRequestToken( localPlayerId );
    }
}

void ExternalMediaAdapter::loginComplete( const std::string& localPlayerId ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onLoginComplete( localPlayerId );
    }
}

void ExternalMediaAdapter::logoutComplete( const std::string& localPlayerId ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onLogoutComplete( localPlayerId );
    }
}

void ExternalMediaAdapter::playerEvent( const std::string& localPlayerId, const std::string& eventName ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onPlayerEvent( localPlayerId, eventName );
    }
}

void ExternalMediaAdapter::playerError( const std::string& localPlayerId, const std::string& errorName, long code, const std::string& description, bool fatal ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onPlayerError( localPlayerId, errorName, code, description, fatal );
    }
}

void ExternalMediaAdapter::setFocus( const std::string& localPlayerId ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onSetFocus( localPlayerId );
    }
}

void ExternalMediaAdapter::removeDiscoveredPlayer( const std::string& localPlayerId ) {
    if( m_externalMediaAdapterEngineInterface != nullptr ) {
        m_externalMediaAdapterEngineInterface->onRemoveDiscoveredPlayer( localPlayerId );
    }
}

void ExternalMediaAdapter::setEngineInterface( std::shared_ptr<aace::alexa::ExternalMediaAdapterEngineInterface> externalMediaAdapterEngineInterface ) {
    m_externalMediaAdapterEngineInterface = externalMediaAdapterEngineInterface;
}

} // aace::alexa
} // aac
