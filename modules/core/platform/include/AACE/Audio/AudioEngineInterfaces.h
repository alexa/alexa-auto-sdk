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

#ifndef AACE_AUDIO_AUDIO_INTERFACES_H
#define AACE_AUDIO_AUDIO_INTERFACES_H

/** @file */

#include <iostream>

namespace aace {
namespace audio {

class AudioInputEngineInterface {
public:
    virtual ssize_t write(const int16_t* data, const size_t size) = 0;
};

class AudioOutputEngineInterface {
public:
    /**
     * Describes an error during a audio playback operation
     */
    enum class MediaError {

        /**
         * An unknown error occurred.
         */
        MEDIA_ERROR_UNKNOWN,

        /**
         * The server recognized the request as malformed (e.g. bad request, unauthorized, forbidden, not found, etc).
         */
        MEDIA_ERROR_INVALID_REQUEST,

        /**
         * The client was unable to reach the service.
         */
        MEDIA_ERROR_SERVICE_UNAVAILABLE,

        /**
         * The server accepted the request but was unable to process it as expected.
         */
        MEDIA_ERROR_INTERNAL_SERVER_ERROR,

        /**
         * There was an internal error on the client.
         */
        MEDIA_ERROR_INTERNAL_DEVICE_ERROR
    };

    /**
     * Describes the playback state of the platform audio channel
     */
    enum class MediaState {

        /**
         * The audio channel is not currently playing. It may have paused, stopped, or finished.
         */
        STOPPED,

        /**
         * The audio channel is currently playing.
         */
        PLAYING,

        /**
         * The audio channel is currently buffering data.
         */
        BUFFERING
    };

    // media player interface
    virtual void onMediaStateChanged(MediaState state) = 0;
    virtual void onMediaError(MediaError error, const std::string& description) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioOutputEngineInterface::MediaState& state) {
    switch (state) {
        case AudioOutputEngineInterface::MediaState::STOPPED:
            stream << "STOPPED";
            break;
        case AudioOutputEngineInterface::MediaState::PLAYING:
            stream << "PLAYING";
            break;
        case AudioOutputEngineInterface::MediaState::BUFFERING:
            stream << "BUFFERING";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AudioOutputEngineInterface::MediaError& error) {
    switch (error) {
        case AudioOutputEngineInterface::MediaError::MEDIA_ERROR_UNKNOWN:
            stream << "MEDIA_ERROR_UNKNOWN";
            break;
        case AudioOutputEngineInterface::MediaError::MEDIA_ERROR_INVALID_REQUEST:
            stream << "MEDIA_ERROR_INVALID_REQUEST";
            break;
        case AudioOutputEngineInterface::MediaError::MEDIA_ERROR_SERVICE_UNAVAILABLE:
            stream << "MEDIA_ERROR_SERVICE_UNAVAILABLE";
            break;
        case AudioOutputEngineInterface::MediaError::MEDIA_ERROR_INTERNAL_SERVER_ERROR:
            stream << "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
            break;
        case AudioOutputEngineInterface::MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR:
            stream << "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
            break;
    }
    return stream;
}

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_INTERFACES_H
