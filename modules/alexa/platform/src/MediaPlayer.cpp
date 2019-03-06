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

#include "AACE/Alexa/MediaPlayer.h"

namespace aace {
namespace alexa {

bool MediaPlayer::isRepeating() {
    return m_mediaPlayerEngineInterface != nullptr && m_mediaPlayerEngineInterface->isRepeating();
}

bool MediaPlayer::isClosed() {
    return m_mediaPlayerEngineInterface == nullptr || m_mediaPlayerEngineInterface->isClosed();
}

void MediaPlayer::mediaStateChanged( MediaState state )
{
    if( m_mediaPlayerEngineInterface != nullptr ) {
        m_mediaPlayerEngineInterface->onMediaStateChanged( state );
    }
}

void MediaPlayer::mediaError( MediaError error, const std::string& description )
{
    if( m_mediaPlayerEngineInterface != nullptr ) {
        m_mediaPlayerEngineInterface->onMediaError( error, description );
    }
}

ssize_t MediaPlayer::read( char* data, const size_t size ) {
    return m_mediaPlayerEngineInterface != nullptr ? m_mediaPlayerEngineInterface->read( data, size ) : 0;
}

void MediaPlayer::setEngineInterface( std::shared_ptr<aace::alexa::MediaPlayerEngineInterface> mediaPlayerEngineInterface ) {
    m_mediaPlayerEngineInterface = mediaPlayerEngineInterface;
}

} // aace::alexa
} // aac
