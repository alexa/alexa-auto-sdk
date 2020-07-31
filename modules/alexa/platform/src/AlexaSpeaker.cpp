/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Alexa/AlexaSpeaker.h"

namespace aace {
namespace alexa {

AlexaSpeaker::~AlexaSpeaker() = default;  // key function

void AlexaSpeaker::speakerSettingsChanged(SpeakerType type, bool local, int8_t volume, bool mute) {
}

void AlexaSpeaker::localSetVolume(SpeakerType type, int8_t volume) {
    if (auto m_alexaSpeakerEngineInterface_lock = m_alexaSpeakerEngineInterface.lock()) {
        m_alexaSpeakerEngineInterface_lock->onLocalSetVolume(type, volume);
    }
}

void AlexaSpeaker::localAdjustVolume(SpeakerType type, int8_t delta) {
    if (auto m_alexaSpeakerEngineInterface_lock = m_alexaSpeakerEngineInterface.lock()) {
        m_alexaSpeakerEngineInterface_lock->onLocalAdjustVolume(type, delta);
    }
}

void AlexaSpeaker::localSetMute(SpeakerType type, bool mute) {
    if (auto m_alexaSpeakerEngineInterface_lock = m_alexaSpeakerEngineInterface.lock()) {
        m_alexaSpeakerEngineInterface_lock->onLocalSetMute(type, mute);
    }
}

void AlexaSpeaker::setEngineInterface(
    std::shared_ptr<aace::alexa::AlexaSpeakerEngineInterface> alexaSpeakerEngineInterface) {
    m_alexaSpeakerEngineInterface = alexaSpeakerEngineInterface;
}

}  // namespace alexa
}  // namespace aace
