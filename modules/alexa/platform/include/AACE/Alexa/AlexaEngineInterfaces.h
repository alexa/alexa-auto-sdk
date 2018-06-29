/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H
#define AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

/** @file */

namespace aace {
namespace alexa {

/**
 * MediaPlayerEngineInterface
 */
class MediaPlayerEngineInterface {
public:

    /**
     * Describes an error during a media playback operation
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
     * Describes the playback state of the platform media player
     */
    enum class MediaState {

        /**
         * The media player is not currently playing. It may have paused, stopped, or finished.
         */
        STOPPED,

        /**
         * The media player is currently playing.
         */
        PLAYING,

        /**
         * The media player is currently buffering data.
         */
        BUFFERING
    };
    
    virtual void onMediaStateChanged( MediaState state ) = 0;
    virtual void onMediaError( MediaError error, const std::string& description ) = 0;
    virtual ssize_t read( char* data, const size_t size ) = 0;
    virtual bool isRepeating() = 0;
    virtual bool isClosed() = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const MediaPlayerEngineInterface::MediaState& state) {
    switch (state) {
        case MediaPlayerEngineInterface::MediaState::STOPPED:
            stream << "STOPPED";
            break;
        case MediaPlayerEngineInterface::MediaState::PLAYING:
            stream << "PLAYING";
            break;
        case MediaPlayerEngineInterface::MediaState::BUFFERING:
            stream << "BUFFERING";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const MediaPlayerEngineInterface::MediaError& error) {
    switch (error) {
        case MediaPlayerEngineInterface::MediaError::MEDIA_ERROR_UNKNOWN:
            stream << "MEDIA_ERROR_UNKNOWN";
            break;
        case MediaPlayerEngineInterface::MediaError::MEDIA_ERROR_INVALID_REQUEST:
            stream << "MEDIA_ERROR_INVALID_REQUEST";
            break;
        case MediaPlayerEngineInterface::MediaError::MEDIA_ERROR_SERVICE_UNAVAILABLE:
            stream << "MEDIA_ERROR_SERVICE_UNAVAILABLE";
            break;
        case MediaPlayerEngineInterface::MediaError::MEDIA_ERROR_INTERNAL_SERVER_ERROR:
            stream << "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
            break;
        case MediaPlayerEngineInterface::MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR:
            stream << "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
            break;
    }
    return stream;
}

class SpeakerEngineInterface {
public:

    virtual void onLocalVolumeSet( int8_t volume ) = 0;
    virtual void onLocalMuteSet( bool mute ) = 0;
};

class SpeechRecognizerEngineInterface {
public:

    virtual bool onHoldToTalk() = 0;
    virtual bool onTapToTalk() = 0;
    virtual bool onStopCapture() = 0;
    virtual ssize_t write( const int16_t* data, const size_t size ) = 0;
    virtual bool enableWakewordDetection() = 0;
    virtual bool disableWakewordDetection() = 0;
};

class AlertsEngineInterface {
public:

    virtual void onLocalStop() = 0;
    virtual void removeAllAlerts() = 0;
};

class PlaybackControllerEngineInterface {
public:
    virtual void onPlayButtonPressed() = 0;
    virtual void onPauseButtonPressed() = 0;
    virtual void onNextButtonPressed() = 0;
    virtual void onPreviousButtonPressed() = 0;
};

/**
 * AuthProviderEngineInterface
 */
class AuthProviderEngineInterface {
public:

    /**
     * Describes the state of client authorization with AVS
     */
    enum class AuthState {

        /**
         * Authorization has not yet been acquired.
         */
        UNINITIALIZED,

        /**
         * Authorization has been refreshed.
         */
        REFRESHED,

        /**
         * Authorization has expired.
         */
        EXPIRED,

        /**
         * Authorization has failed in a manner that cannot be corrected by retrying.
         */
        UNRECOVERABLE_ERROR
    };

    /**
     * Describes an error during an attempt to authorize with AVS
     */
    enum class AuthError {

        /**
         * No error encountered
         */
        NO_ERROR,

        /**
         * An error was encountered, but no error description can be determined.
         */
        UNKNOWN_ERROR,

        /**
         * The client authorization failed.
         */
        AUTHORIZATION_FAILED,

        /**
         * The client is not authorized to use authorization codes.
         */
        UNAUTHORIZED_CLIENT,

        /**
         * The server encountered a runtime error.
         */
        SERVER_ERROR,

        /**
         * The request is missing a required parameter, has an invalid value, or is otherwise malformed.
         */
        INVALID_REQUEST,

        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         */
        AUTHORIZATION_EXPIRED,

        /**
         * The client specified the wrong token type.
         */
        UNSUPPORTED_GRANT_TYPE
    };

    virtual void onAuthStateChanged( AuthState state, AuthError error ) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AuthProviderEngineInterface::AuthState& state) {
    switch (state) {
        case AuthProviderEngineInterface::AuthState::UNINITIALIZED:
            stream << "UNINITIALIZED";
            break;
        case AuthProviderEngineInterface::AuthState::REFRESHED:
            stream << "REFRESHED";
            break;
        case AuthProviderEngineInterface::AuthState::EXPIRED:
            stream << "EXPIRED";
            break;
        case AuthProviderEngineInterface::AuthState::UNRECOVERABLE_ERROR:
            stream << "UNRECOVERABLE_ERROR";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AuthProviderEngineInterface::AuthError& error) {
    switch (error) {
        case AuthProviderEngineInterface::AuthError::NO_ERROR:
            stream << "NO_ERROR";
            break;
        case AuthProviderEngineInterface::AuthError::UNKNOWN_ERROR:
            stream << "UNKNOWN_ERROR";
            break;
        case AuthProviderEngineInterface::AuthError::AUTHORIZATION_FAILED:
            stream << "AUTHORIZATION_FAILED";
            break;
        case AuthProviderEngineInterface::AuthError::UNAUTHORIZED_CLIENT:
            stream << "UNAUTHORIZED_CLIENT";
            break;
        case AuthProviderEngineInterface::AuthError::SERVER_ERROR:
            stream << "SERVER_ERROR";
            break;
        case AuthProviderEngineInterface::AuthError::INVALID_REQUEST:
            stream << "INVALID_REQUEST";
            break;
        case AuthProviderEngineInterface::AuthError::AUTHORIZATION_EXPIRED:
            stream << "AUTHORIZATION_EXPIRED";
            break;
        case AuthProviderEngineInterface::AuthError::UNSUPPORTED_GRANT_TYPE:
            stream << "UNSUPPORTED_GRANT_TYPE";
            break;
    }
    return stream;
}

} // aace::alexa
} // aace

#endif // AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H
