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

#ifndef AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H
#define AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>

/** @file */

namespace aace {
namespace alexa {

/**
 * AlexaSpeakerEngineInterface
 */
class AlexaSpeakerEngineInterface {
public:
    /**
     * Specifies the type of the AlexaSpeaker to control
     */
    enum class SpeakerType {

        /**
         * The Speaker type that is controlled by AVS
         */
        ALEXA_VOLUME,

        /**
         * The Speaker type that is controlled locally by the platform
         */
        ALERTS_VOLUME
    };

    virtual void onLocalSetVolume(SpeakerType type, int8_t volume) = 0;
    virtual void onLocalAdjustVolume(SpeakerType type, int8_t delta) = 0;
    virtual void onLocalSetMute(SpeakerType type, bool mute) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AlexaSpeakerEngineInterface::SpeakerType& type) {
    switch (type) {
        case AlexaSpeakerEngineInterface::SpeakerType::ALEXA_VOLUME:
            stream << "ALEXA_VOLUME";
            break;
        case AlexaSpeakerEngineInterface::SpeakerType::ALERTS_VOLUME:
            stream << "ALERTS_VOLUME";
            break;
    }
    return stream;
}

/**
 * SpeechRecognizerEngineInterface
 */
class SpeechRecognizerEngineInterface {
public:
    /**
     * Describes type of event that initiated the speech request.
     */
    enum class Initiator {

        /**
         * Hold-to-talk speech initiator type.
         */
        HOLD_TO_TALK,
        /**
         * Tap-to-talk speech initiator type.
         */
        TAP_TO_TALK,
        /**
         * Wakeword speech initiator type.
         */
        WAKEWORD
    };

    /*
     * Defines an unspecified value for the speech recognizer's audio index.
     */
    static constexpr uint64_t UNSPECIFIED_INDEX = std::numeric_limits<uint64_t>::max();

    virtual bool onStartCapture(
        Initiator initiator,
        uint64_t keywordBegin,
        uint64_t keywordEnd,
        const std::string& keyword) = 0;
    virtual bool onStopCapture() = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const SpeechRecognizerEngineInterface::Initiator& initiator) {
    switch (initiator) {
        case SpeechRecognizerEngineInterface::Initiator::HOLD_TO_TALK:
            stream << "HOLD_TO_TALK";
            break;
        case SpeechRecognizerEngineInterface::Initiator::TAP_TO_TALK:
            stream << "TAP_TO_TALK";
            break;
        case SpeechRecognizerEngineInterface::Initiator::WAKEWORD:
            stream << "WAKEWORD";
            break;
    }
    return stream;
}

/**
 * AlertsEngineInterface
 */
class AlertsEngineInterface {
public:
    virtual void onLocalStop() = 0;
    virtual void removeAllAlerts() = 0;
};

/**
 * PlaybackControllerEngineInterface
 */
class PlaybackControllerEngineInterface {
public:
    /**
     * Describes the playback button type
     */
    enum class PlaybackButton {
        /**
         * 'Play' button.
         */
        PLAY,
        /**
         * 'Pause' button.
         */
        PAUSE,
        /**
         * 'Next' button.
         */
        NEXT,
        /**
         * 'Previous' button.
         */
        PREVIOUS,
        /**
         * 'Skip Forward' button.
         */
        SKIP_FORWARD,
        /**
         * 'Skip Backward' button.
         */
        SKIP_BACKWARD
    };

    /**
     * Describes the playback button type
     */
    enum class PlaybackToggle {
        /**
         * 'Shuffle' toggle.
         */
        SHUFFLE,
        /**
         * 'Loop' toggle.
         */
        LOOP,
        /**
         * 'Repeat' toggle.
         */
        REPEAT,
        /**
         * 'Thumbs Up' toggle.
         */
        THUMBS_UP,
        /**
         * 'Thumbs Down' toggle.
         */
        THUMBS_DOWN
    };

    virtual void onButtonPressed(PlaybackButton button) = 0;
    virtual void onTogglePressed(PlaybackToggle toggle, bool action) = 0;
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
         * One of the values in the request was invalid.
         */
        INVALID_VALUE,

        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         */
        AUTHORIZATION_EXPIRED,

        /**
         * The client specified the wrong token type.
         */
        UNSUPPORTED_GRANT_TYPE,

        /**
         * Invalid code pair provided in Code-based linking token request.
         */
        INVALID_CODE_PAIR,

        /**
         * Waiting for user to authorize the specified code pair.
         */
        AUTHORIZATION_PENDING,

        /**
         * Client should slow down in the rate of requests polling for an access token.
         */
        SLOW_DOWN,

        /**
         * Internal error in client code.
         */
        INTERNAL_ERROR,

        /**
         * Client ID not valid for use with code based linking.
         */
        INVALID_CBL_CLIENT_ID
    };

    virtual void onAuthStateChanged(AuthState state, AuthError error) = 0;
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
        case AuthProviderEngineInterface::AuthError::INVALID_VALUE:
            stream << "INVALID_VALUE";
            break;
        case AuthProviderEngineInterface::AuthError::AUTHORIZATION_EXPIRED:
            stream << "AUTHORIZATION_EXPIRED";
            break;
        case AuthProviderEngineInterface::AuthError::UNSUPPORTED_GRANT_TYPE:
            stream << "UNSUPPORTED_GRANT_TYPE";
            break;
        case AuthProviderEngineInterface::AuthError::INVALID_CODE_PAIR:
            stream << "INVALID_CODE_PAIR";
            break;
        case AuthProviderEngineInterface::AuthError::AUTHORIZATION_PENDING:
            stream << "AUTHORIZATION_PENDING";
            break;
        case AuthProviderEngineInterface::AuthError::SLOW_DOWN:
            stream << "SLOW_DOWN";
            break;
        case AuthProviderEngineInterface::AuthError::INTERNAL_ERROR:
            stream << "INTERNAL_ERROR";
            break;
        case AuthProviderEngineInterface::AuthError::INVALID_CBL_CLIENT_ID:
            stream << "INVALID_CBL_CLIENT_ID";
            break;
    }
    return stream;
}

/**
 * ExternalMediaAdapterEngineInterface
 */
class ExternalMediaAdapterEngineInterface {
public:
    /**
     * Describes a discovered external media player app
     */
    class DiscoveredPlayerInfo {
    public:
        /// The opaque token that uniquely identifies the local external player app
        std::string localPlayerId;
        /// The only spiVersion that currently exists is "1.0"
        std::string spiVersion;
        /** Validation methods :
         *  1. "SIGNING_CERTIFICATE"
         *  2. "GENERATED_CERTIFICATE"
         *  3. "NONE"
         */
        std::string validationMethod;
        /** Validation data :
         *  1. Device platform issued app signing certificate. A list of certificates may be attached.
         *  2. In some cases validation is performed locally. The certificate is trasmitted as validationData during discovery to announce the activated app's identity in order to allow app activation to be revoked.
         *  3. empty
         */
        std::vector<std::string> validationData;
    };

    virtual void onReportDiscoveredPlayers(const std::vector<DiscoveredPlayerInfo>& discoveredPlayers) = 0;
    virtual void onRequestToken(const std::string& localPlayerId) = 0;
    virtual void onLoginComplete(const std::string& localPlayerId) = 0;
    virtual void onLogoutComplete(const std::string& localPlayerId) = 0;
    virtual void onPlayerEvent(const std::string& localPlayerId, const std::string& eventName) = 0;
    virtual void onPlayerError(
        const std::string& localPlayerId,
        const std::string& errorName,
        long code,
        const std::string& description,
        bool fatal) = 0;
    virtual void onSetFocus(const std::string& playerId) = 0;
    virtual void onRemoveDiscoveredPlayer(const std::string& localPlayerId) = 0;
};

/**
 * LocalMediaSourceEngineInterface
 */
class LocalMediaSourceEngineInterface {
public:
    virtual void onPlayerEvent(const std::string& eventName) = 0;
    virtual void onPlayerError(const std::string& errorName, long code, const std::string& description, bool fatal) = 0;
    virtual void onSetFocus(bool focusAcquire = true) = 0;
};

/**
 * DoNotDisturbEngineInterface
 */
class DoNotDisturbEngineInterface {
public:
    /**
     * @internal
     * Notifies the Engine that A DND change has been initiated by the client
     * 
     * @param [in] doNotDisturb The DND Setting value
     * @return true if successful, false if change was rejected
     */
    virtual bool onDoNotDisturbChanged(const bool doNotDisturb) = 0;
};

/**
 * EqualizerControllerEngineInterface
 */
class EqualizerControllerEngineInterface {
public:
    /**
     * Describes the equalizer bands supported by Alexa. The platform implementation may support a subset of these.
     */
    enum class EqualizerBand {
        /// Bass equalizer band
        BASS,
        /// Mid-range equalizer band
        MIDRANGE,
        /// Treble equalizer band
        TREBLE
    };

    /**
     * Describes the level of gain of a particular equalizer band as an integer dB value. This is an
     * @c aace::alexa::EqualizerController::EqualizerBand and @c int pair.
     */
    using EqualizerBandLevel = std::pair<EqualizerBand, int>;

    /**
     * @internal
     * Notifies the Engine that gain levels for one or more equalizer bands are being set directly on the device. If 
     * unsupported levels are provided, the Engine should truncate the settings to the configured range.
     * 
     * @param [in] bandLevels The equalizer bands to change and their gain settings as integer dB values.
     */
    virtual void onLocalSetBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) = 0;

    /**
     * @internal
     * Notifies the Engine that relative adjustments to equalizer band gain levels are being made directly on the 
     * device. If adjustments put the band level settings beyond the configured dB range, the Engine should truncate 
     * the settings to the configured range.
     * 
     * @param [in] bandAdjustments The equalizer bands to adjust and their relative gain adjustments as integer dB 
     *             values.
     */
    virtual void onLocalAdjustBandLevels(const std::vector<EqualizerBandLevel>& bandAdjustments) = 0;

    /**
     * @internal
     * Notifies the Engine that the gain levels for the equalizer bands are being reset to their defaults.
     * 
     * @param [in] bands The equalizer bands to reset. Empty @a bands resets all supported equalizer bands.
     */
    virtual void onLocalResetBands(const std::vector<EqualizerBand>& bands) = 0;
};

/**
 * Provides a string representation for an @c EqualizerControllerEngineInterface::EqualizerBand
 *
 * @param band The @c EqualizerBand
 * @return A string representation for the @c EqualizerBand
 */
inline std::string equalizerBandToString(const EqualizerControllerEngineInterface::EqualizerBand& band) {
    switch (band) {
        case EqualizerControllerEngineInterface::EqualizerBand::BASS:
            return "BASS";
        case EqualizerControllerEngineInterface::EqualizerBand::MIDRANGE:
            return "MIDRANGE";
        case EqualizerControllerEngineInterface::EqualizerBand::TREBLE:
            return "TREBLE";
    }
    return "UNKNOWN";
}

/**
 * Write an @c EqualizerControllerEngineInterface::EqualizerBand value to an @c ostream as a string
 *
 * @param stream The stream to write to
 * @param band The @c EqualizerBand value to write to the @c ostream
 * @return The @c ostream argument that was written to
 */
inline std::ostream& operator<<(std::ostream& stream, const EqualizerControllerEngineInterface::EqualizerBand& band) {
    stream << equalizerBandToString(band);
    return stream;
}

/**
 * AudioPlayerEngineInterface
 */
class AudioPlayerEngineInterface {
public:
    /**
     * Returns the current playback position of the audio player.
     * If the audio source is not playing, the most recent position played
     * should be returned.
     *
     * @return The audio player's playback position in milliseconds,
     * or @c TIME_UNKNOWN if the current media position is unknown or invalid.
     */
    virtual int64_t onGetPlayerPosition() = 0;

    /**
     * Returns the playback duration of the audio player.
     *
     * @return The audio player's playback duration in milliseconds,
     * or @c TIME_UNKNOWN if the duration is unknown or invalid.
     */
    virtual int64_t onGetPlayerDuration() = 0;
};

/**
 * TemplateRuntimeEngineInterface
 */
class TemplateRuntimeEngineInterface {
public:
    /**
     * Notifies the Engine that a display card has been cleared from the screen. Upon getting
     * this notification, the @c TemplateRuntime will release the visual channel.
     */
    virtual void onDisplayCardCleared() = 0;
};

/**
 * AlexaClientEngineInterface
 */
class AlexaClientEngineInterface {
public:
    virtual void onStopForegroundActivity() = 0;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_ALEXA_ENGINE_INTERFACES_H
