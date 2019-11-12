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

#include <AACE/Alexa/AudioPlayer.h>

namespace aace {
namespace alexa {

AudioPlayer::~AudioPlayer() = default; // key function

#ifdef OBIGO_AIDAEMON
bool AudioPlayer::setMVPAAudioPlayer() {
  if (auto m_audioPlayerEngineInterface_lock = m_audioPlayerEngineInterface.lock()) {
      return m_audioPlayerEngineInterface_lock->onSetMVPAAudioPlayer();
  } else {
      return false;
  }
}

void AudioPlayer::setEngineInterface(
    std::shared_ptr<aace::alexa::AudioPlayerEngineInterface> audioPlayerEngineInterface) {
  m_audioPlayerEngineInterface = audioPlayerEngineInterface;
}
#endif
} // aace::alexa
} // aac
