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

#ifndef AACE_ALEXA_MEDIAPLAYBACKREQUESTOR_H_
#define AACE_ALEXA_MEDIAPLAYBACKREQUESTOR_H_

#include <AACE/Core/PlatformInterface.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * @c MediaPlaybackRequestor platform interface should be extended and registered into the Alexa Engine.
 * This class is used to send an event to AVS when client is expected to automatically resume the last playing media.
 * Alexa can use this event to push the last playing media on the device.
 */
class MediaPlaybackRequestor : public aace::core::PlatformInterface {
protected:
    MediaPlaybackRequestor() = default;

public:
    virtual ~MediaPlaybackRequestor();

    using InvocationReason = aace::alexa::MediaPlaybackRequestorEngineInterface::InvocationReason;
    using MediaPlaybackRequestStatus = aace::alexa::MediaPlaybackRequestorEngineInterface::MediaPlaybackRequestStatus;

    /**
     * Notifies the platform implementation of the delivery status of @c requestMediaPlayback. 
     */
    virtual void mediaPlaybackResponse(MediaPlaybackRequestStatus mediaPlaybackRequestStatus) = 0;

    /**
     * Platform implementation is expected to call this method whenever Alexa is the right source for the media resume.
     * Automatic media resume is supported only within the threashold duration to avoid sudden surprise music playback to driver while driving.
     * 
     * @Note: Please refer the README file to know more about the threashold and its configuration.
     * 
     * @param @c invocationReason Reason to call this API. currently only @c AUTOMOTIVE_STARTUP is supported.
     * @param @c elapsedBootTime Provide the elapsedBootTime values in ms only. This time is critical and related to the safety of the driver. Please ensure that accurate value is provided.
     */
    void requestMediaPlayback(
        InvocationReason invocationReason = InvocationReason::AUTOMOTIVE_STARTUP,
        long long int elapsedBootTime = 0);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(
        std::shared_ptr<MediaPlaybackRequestorEngineInterface> mediaPlaybackRequestorEngineInterface);

private:
    std::weak_ptr<MediaPlaybackRequestorEngineInterface> m_mediaPlaybackRequestorEngineInterface;
};

/**
 * This function returns the std::string equivalent of the input enum value
 */
inline std::string mediaPlaybackRequestorMediaPlaybackRequestStatusToString(
    const MediaPlaybackRequestor::MediaPlaybackRequestStatus& mediaPlaybackRequestStatus) {
    switch (mediaPlaybackRequestStatus) {
        case MediaPlaybackRequestor::MediaPlaybackRequestStatus::SUCCESS:
            return "SUCCESS";
        case MediaPlaybackRequestor::MediaPlaybackRequestStatus::FAILED_CAN_RETRY:
            return "FAILED_CAN_RETRY";
        case MediaPlaybackRequestor::MediaPlaybackRequestStatus::FAILED_TIMEOUT:
            return "FAILED_TIMEOUT";
        case MediaPlaybackRequestor::MediaPlaybackRequestStatus::ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}

inline std::ostream& operator<<(
    std::ostream& stream,
    const MediaPlaybackRequestor::MediaPlaybackRequestStatus& mediaPlaybackRequestStatus) {
    stream << mediaPlaybackRequestorMediaPlaybackRequestStatusToString(mediaPlaybackRequestStatus);
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_MEDIAPLAYBACKREQUESTOR_H_
