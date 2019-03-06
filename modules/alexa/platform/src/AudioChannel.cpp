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

#include "AACE/Alexa/AudioChannel.h"

namespace aace {
namespace alexa {

AudioChannel::AudioChannel( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker, aace::alexa::Speaker::Type speakerType ) :
    m_mediaPlayer( mediaPlayer ),
    m_speaker( speaker ),
    m_speakerType( speakerType ) {
}

std::shared_ptr<aace::alexa::MediaPlayer> AudioChannel::getMediaPlayer() {
    return m_mediaPlayer;
}

std::shared_ptr<aace::alexa::Speaker> AudioChannel::getSpeaker() {
    return m_speaker;
}

aace::alexa::Speaker::Type AudioChannel::getSpeakerType() {
    return m_speakerType;
}

} // aace::alexa
} // aac
