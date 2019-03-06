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

#include "AACE/Alexa/Speaker.h"

namespace aace {
namespace alexa {

void Speaker::localVolumeSet( int8_t volume ) {
    if( m_speakerEngineInterface != nullptr ) {
        m_speakerEngineInterface->onLocalVolumeSet( volume );
    }
}

void Speaker::localMuteSet( bool mute ) {
    if( m_speakerEngineInterface != nullptr ) {
        m_speakerEngineInterface->onLocalMuteSet( mute );
    }
}

void Speaker::setEngineInterface( std::shared_ptr<aace::alexa::SpeakerEngineInterface> speakerEngineInterface ) {
    m_speakerEngineInterface = speakerEngineInterface;
}

} // aace::alexa
} // aac
