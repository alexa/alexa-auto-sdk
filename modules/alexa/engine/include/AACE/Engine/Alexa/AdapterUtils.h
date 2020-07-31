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

#ifndef AACE_ENGINE_ALEXA_ADAPTERUTILS_H_
#define AACE_ENGINE_ALEXA_ADAPTERUTILS_H_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include "AVSCommon/AVS/NamespaceAndName.h"
#include "ExternalMediaAdapterInterface.h"
#include "AVSCommon/Utils/RetryTimer.h"

namespace aace {
namespace engine {
namespace alexa {

/// Enumeration class for events sent by adapters to AVS.
enum class AdapterEvent {
    /// ChangeReport event sent after adapter's initialization succeeds/fails.
    CHANGE_REPORT,  // Note: not in ExternalMediaPlayer 1.1

    /// Event to request token from third party.
    REQUEST_TOKEN,

    /// Login event when a guest user logs in.
    LOGIN,

    /// Logout event when a user logs out.
    LOGOUT,

    /// PlayerEvent to announce all kinds of player events - like play/pause/next etc.
    PLAYER_EVENT,

    /// PlayerErrorEvent to report all errors from the adapters.
    PLAYER_ERROR_EVENT,  // Note: rename PLAYER_ERROR in ExternalMediaPlayer 1.1

    // adapter handler specific code
    /// PlayerError event to report all errors from the adapters.
    PLAYER_ERROR,

    /// ReportDiscoveredPlayers event notifies the cloud of discovered players to be authorized.
    REPORT_DISCOVERED_PLAYERS,

    /// AuthorizationComplete event notifies the cloud of players which were authorized (and de-authorized).
    AUTHORIZATION_COMPLETE

};

/// The retry timer for session management (token fetch/changeReport send).
alexaClientSDK::avsCommon::utils::RetryTimer& sessionRetryTimer();

// The NamespaceAndName for events sent from the adapter to AVS.
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName CHANGE_REPORT;  // Note: not in ExternalMediaPlayer 1.1
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName REQUEST_TOKEN;
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName LOGIN;
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName LOGOUT;
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName PLAYER_EVENT;
extern const alexaClientSDK::avsCommon::avs::NamespaceAndName
    PLAYER_ERROR_EVENT;  // Note: rename PLAYER_ERROR in ExternalMediaPlayer 1.1

// adapter handler specific code ( unused )
// extern const alexaClientSDK::avsCommon::avs::NamespaceAndName REPORT_DISCOVERED_PLAYERS;
// extern const alexaClientSDK::avsCommon::avs::NamespaceAndName AUTHORIZATION_COMPLETE;

/**
 * Method to iterate over a collection of supported operation in playback state and convert to JSON.
 *
 * @param supportedOperations The collection of supported operations from the current playback state.
 * @param allocator The rapidjson allocator, required for the results of this function to be mergable with other
 * rapidjson::Value objects.
 * @return The rapidjson::Value representing the array.
 */
rapidjson::Value buildSupportedOperations(
    const std::set<aace::engine::alexa::SupportedPlaybackOperation>& supportedOperations,
    rapidjson::Document::AllocatorType& allocator);

/**
 * Method to convert a playbackState to JSON.
 *
 * @param playerId The playerId that identifies this player. This refers to the one assigned by the cloud.
 * @param playbackState The playback state of the adapter.
 * @param The rapidjson allocator, required for the results of this function to be mergable with other
 * rapidjson::Value objects.
 * @return The rapidjson::Value representing the playback state JSON.
 */
rapidjson::Value buildPlaybackState(
    const std::string& playerId,
    const aace::engine::alexa::AdapterPlaybackState& playbackState,
    rapidjson::Document::AllocatorType& allocator);

/**
 * Method to convert session state  to JSON.
 *
 * @param sessionState The session state of the adapter.
 * @param The rapidjson allocator, required for the results of this function to be mergable with other
 * rapidjson::Value objects.
 * @return The rapidjson::Value representing the session state in JSON.
 */
rapidjson::Value buildSessionState(
    const aace::engine::alexa::AdapterSessionState& sessionState,
    rapidjson::Document::AllocatorType& allocator);

/**
 * Method to build the default player.
 *
 * @param document The JSON Value to write the default player state into.
 * @param allocator The rapidjson allocator, required for the results of this function to be mergable with other
 * rapidjson::Value objects.
 * @return @c true if the build of default player state was successful, @c false otherwise.
 */
bool buildDefaultPlayerState(rapidjson::Value* document, rapidjson::Document::AllocatorType& allocator);

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // end
// AACE_ENGINE_ALEXA_ADAPTERUTILS_H_
