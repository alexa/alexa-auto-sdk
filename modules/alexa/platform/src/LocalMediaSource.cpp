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

#include "AACE/Alexa/LocalMediaSource.h"

namespace aace {
namespace alexa {

LocalMediaSource::LocalMediaSource( Source source, std::shared_ptr<aace::alexa::Speaker> speaker ) : m_source( source ), m_speaker( speaker ) {
}

LocalMediaSource::~LocalMediaSource() = default;

LocalMediaSource::Source LocalMediaSource::getSource() {
    return m_source;
}

std::shared_ptr<aace::alexa::Speaker> LocalMediaSource::LocalMediaSource::getSpeaker() {
    return m_speaker;
}

//
// Engine interface methods
//

void LocalMediaSource::playerEvent( const std::string& eventName ) {
    if( m_localMediaSourceEngineInterface != nullptr ) {
        m_localMediaSourceEngineInterface->onPlayerEvent( eventName );
    }
}

void LocalMediaSource::playerError( const std::string& errorName, long code, const std::string& description, bool fatal ) {
    if( m_localMediaSourceEngineInterface != nullptr ) {
        m_localMediaSourceEngineInterface->onPlayerError( errorName, code, description, fatal );
    }
}

void LocalMediaSource::setFocus() {
    if( m_localMediaSourceEngineInterface != nullptr ) {
        m_localMediaSourceEngineInterface->onSetFocus();
    }
}

void LocalMediaSource::setEngineInterface( std::shared_ptr<aace::alexa::LocalMediaSourceEngineInterface> localMediaSourceEngineInterface ) {
    m_localMediaSourceEngineInterface = localMediaSourceEngineInterface;
}

} // aace::alexa
} // aac
