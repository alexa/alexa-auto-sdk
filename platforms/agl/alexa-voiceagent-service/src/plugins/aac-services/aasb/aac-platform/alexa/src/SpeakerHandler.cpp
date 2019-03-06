/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "SpeakerHandler.h"

namespace aasb {
namespace alexa {

std::shared_ptr<SpeakerHandler> SpeakerHandler::create(std::shared_ptr<aasb::core::logger::LoggerHandler> logger) {
    auto speakerHandler = std::shared_ptr<SpeakerHandler>(new SpeakerHandler());

    speakerHandler->m_logger = logger;

    return speakerHandler;
}

bool SpeakerHandler::setVolume(int8_t volume) {
    return true;
}

bool SpeakerHandler::adjustVolume(int8_t delta) {
    return true;
}

bool SpeakerHandler::setMute(bool mute) {
    return true;
}

int8_t SpeakerHandler::getVolume() {
    return 0;
}

bool SpeakerHandler::isMuted() {
    return true;
}

}  // namespace alexa
}  // namespace aasb