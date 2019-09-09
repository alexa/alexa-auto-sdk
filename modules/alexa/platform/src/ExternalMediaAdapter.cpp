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

ExternalMediaAdapter::~ExternalMediaAdapter() = default;

//
// Engine interface methods
//

void ExternalMediaAdapter::reportDiscoveredPlayers( const std::vector<DiscoveredPlayerInfo>& discoveredPlayers ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onReportDiscoveredPlayers( discoveredPlayers );
    }
}

void ExternalMediaAdapter::requestToken( const std::string& localPlayerId ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onRequestToken( localPlayerId );
    }
}

void ExternalMediaAdapter::loginComplete( const std::string& localPlayerId ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onLoginComplete( localPlayerId );
    }
}

void ExternalMediaAdapter::logoutComplete( const std::string& localPlayerId ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onLogoutComplete( localPlayerId );
    }
}

void ExternalMediaAdapter::playerEvent( const std::string& localPlayerId, const std::string& eventName ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onPlayerEvent( localPlayerId, eventName );
    }
}

void ExternalMediaAdapter::playerError( const std::string& localPlayerId, const std::string& errorName, long code, const std::string& description, bool fatal ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onPlayerError( localPlayerId, errorName, code, description, fatal );
    }
}

void ExternalMediaAdapter::setFocus( const std::string& localPlayerId ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onSetFocus( localPlayerId );
    }
}

void ExternalMediaAdapter::removeDiscoveredPlayer( const std::string& localPlayerId ) {
    if( auto m_externalMediaAdapterEngineInterface_lock = m_externalMediaAdapterEngineInterface.lock() ) {
        m_externalMediaAdapterEngineInterface_lock->onRemoveDiscoveredPlayer( localPlayerId );
    }
}

void ExternalMediaAdapter::setEngineInterface( std::shared_ptr<aace::alexa::ExternalMediaAdapterEngineInterface> externalMediaAdapterEngineInterface ) {
    m_externalMediaAdapterEngineInterface = externalMediaAdapterEngineInterface;
}

} // aace::alexa
} // aac
