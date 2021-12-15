/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Alexa/MediaPlaybackRequestor.h"

namespace aace {
namespace alexa {

MediaPlaybackRequestor::~MediaPlaybackRequestor() = default;  // key function

void MediaPlaybackRequestor::requestMediaPlayback(InvocationReason invocationReason, long long int elapsedBootTime) {
    if (auto m_mediaPlaybackRequestorEngineInterface_lock = m_mediaPlaybackRequestorEngineInterface.lock()) {
        if (elapsedBootTime > 0)
            m_mediaPlaybackRequestorEngineInterface_lock->onRequestMediaPlayback(invocationReason, elapsedBootTime);
        else {
            mediaPlaybackResponse(MediaPlaybackRequestStatus::ERROR);
        }
    }
}

void MediaPlaybackRequestor::setEngineInterface(
    std::shared_ptr<MediaPlaybackRequestorEngineInterface> mediaPlaybackRequestorEngineInterface) {
    m_mediaPlaybackRequestorEngineInterface = mediaPlaybackRequestorEngineInterface;
}

}  // namespace alexa
}  // namespace aace
