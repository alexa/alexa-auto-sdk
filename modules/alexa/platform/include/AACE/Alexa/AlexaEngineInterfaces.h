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

class MediaPlayerEngineInterface {
public:
    /**
     * Type used to identify an error that can occur during media operations.
     */
    enum class ErrorType {
        /**
         * An unknown error occurred.
         */
        MEDIA_ERROR_UNKNOWN,
        /**
         * The server recognized the request as being malformed (bad request, unauthorized, forbidden, not found, etc).
         */
        MEDIA_ERROR_INVALID_REQUEST,
        /**
         * The client was unable to reach the service.
         */
        MEDIA_ERROR_SERVICE_UNAVAILABLE,
        /**
         * The server accepted the request, but was unable to process the request as expected.
         */
        MEDIA_ERROR_INTERNAL_SERVER_ERROR,
        /**
         * There was an internal error on the client.
         */
        MEDIA_ERROR_INTERNAL_DEVICE_ERROR
    };
    virtual void onPlaybackStarted() = 0;
    virtual void onPlaybackFinished() = 0;
    virtual void onPlaybackPaused() = 0;
    virtual void onPlaybackResumed() = 0;
    virtual void onPlaybackStopped() = 0;
    virtual void onPlaybackError( const ErrorType& type, const std::string& error ) = 0;
    virtual ssize_t read( char* data, const size_t size ) = 0;
    virtual bool isRepeating() = 0;
};

class SpeechRecognizerEngineInterface {
public:
    /**
     * Initiate a Recognize Event from a press-and-hold action. The @c SpeechRecognizer will start streaming
     * the audio samples provided by @c write() to AVS.
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the Recognize Event was started successfully,
     * else @c false.
     */
    virtual bool onHoldToTalk() = 0;

    /**
     * Initiate a Recognize Event from a tap-to-talk action. The @c SpeechRecognizer will start streaming
     * the audio samples provided with @c write() to AVS.
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the Recognize Event was stopped successfully,
     * else @c false.
     */
    virtual bool onTapToTalk() = 0;

    /**
     * Request the @c SpeechRecognizer to stop streaming audio and terminate the current Recognize Event, which
     * transitions it to the @c BUSY state. This function can only be called in the @c RECOGNIZING state; calling it
     * while in another state will fail.
     *
     * @return @c true if called in the correct state and a Recognize Event's audio streaming was stopped successfully,
     * else @c false.
     */
    virtual bool onStopCapture() = 0;

    /**
     * Write a chunk of audio samples to be processed by the wake word engine and/or streamed to AVS.
     * Audio samples should be formatted in LPCM encoded with 16 bits per sample mono
     * at a sample rate of 16 kHz. The are samples are in little endian format.
     * Typically, samples are provided every 10 milliseconds in 160 sample chunks.
     *
     * @param [in] data The data buffer to write.
     * @param [in] size The size of the data to write in bytes.
     * @return The number of int16_t written, or zero if the stream has closed, or a negative error code
     * if the stream is still open, but no data could be written.
     */
    virtual ssize_t write( const int16_t* data, const size_t size ) = 0;

    /**
     * Enables wakeword detection in the speech recognizer. When wakeword detection is enabled, audio must be continuously
     * streamed to @c SpeechRecognizer by calling the @c write() method.
     *
     * @return @c true if wakeword detection was enebaled, else @c false.
     */
    virtual bool enableWakewordDetection() = 0;

    /**
     * Disables wakeword detection in the speech recognizer.
     *
     * @return @c true if wakeword detection was disabled, else @c false.
     */
    virtual bool disableWakewordDetection() = 0;
};

class AlertsEngineInterface {
public:
    /**
     * This function provides a way for application code to request this object stop any active alert as the result
     * of a user action, such as pressing a physical 'stop' button on the device.
     */
    virtual void onLocalStop() = 0;
    /**
     * A function that allows an application to clear all alerts from storage.  This may be useful for a scenario
     * where a user logs out of a device, and another user will log in.  As the first user logs out, their pending
     * alerts should not go off.
     */
    virtual void removeAllAlerts() = 0;
};

class PlaybackControllerEngineInterface {
public:
    virtual void onPlayButtonPressed() = 0;
    virtual void onPauseButtonPressed() = 0;
    virtual void onNextButtonPressed() = 0;
    virtual void onPreviousButtonPressed() = 0;
};

class AuthProviderEngineInterface {
public:
    /**
     * The enum AuthState describes the state of authorization.
     */
    enum class AuthState {
        /**
         * Authorization not yet acquired.
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
         * Authorization failed in a manner that cannot be corrected by retrying.
         */
        UNRECOVERABLE_ERROR
    };

    /**
     * The enum AuthError encodes possible errors which may occur when changing authorization state.
     */
    enum class AuthError {
        /**
         * No error.
         */
        NO_ERROR,
        /**
         * An unknown body containing no error field has been encountered.
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
         * The request is missing a required parameter, has an invalid value, or is otherwise improperly formed.
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

    /**
     * Allows the platform to set a new AuthState and AuthError for the Engine
     */
    virtual void onAuthStateChanged( AuthState state, AuthError error ) = 0;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H
