/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_EXTERNALMEDIAADAPTERCONSTANTS_H
#define AACE_ENGINE_ALEXA_EXTERNALMEDIAADAPTERCONSTANTS_H

#include <AVSCommon/AVS/NamespaceAndName.h>

namespace aace {
namespace engine {
namespace alexa {

// The key values used in the context payload from External Media Player to AVS.
const char PLAYER_ID[] = "playerId";
const char ENDPOINT_ID[] = "endpointId";
const char LOGGED_IN[] = "loggedIn";
const char USERNAME[] = "username";
const char IS_GUEST[] = "isGuest";
const char LAUNCHED[] = "launched";
const char ACTIVE[] = "active";
const char SPI_VERSION[] = "spiVersion";
const char SPI_VERSION_DEFAULT[] = "1.0";
const char PLAYER_COOKIE[] = "playerCookie";
const char SKILL_TOKEN[] = "skillToken";
const char PLAYBACK_SESSION_ID[] = "playbackSessionId";
const char AGENT[] = "agent";
const char AGENT_DEFAULT[] = "RUHAV8PRLD";

// player events
const char PLAYBACK_SESSION_STARTED[] = "PlaybackSessionStarted";
const char PLAYBACK_SESSION_ENDED[] = "PlaybackSessionEnded";
const char PLAYBACK_STARTED[] = "PlaybackStarted";
const char PLAYBACK_STOPPED[] = "PlaybackStopped";
const char PLAYBACK_PREVIOUS[] = "PlaybackPrevious";
const char PLAYBACK_NEXT[] = "PlaybackNext";
const char TRACK_CHANGED[] = "TrackChanged";
const char PLAY_MODE_CHANGED[] = "PlayModeChanged";
// player errors
const char UNPLAYABLE_BY_ACCOUNT[] = "UNPLAYABLE_BY_ACCOUNT";
const char INTERNAL_ERROR[] = "INTERNAL_ERROR";
const char UNPLAYABLE_BY_AUTHORIZATION[] = "UNPLAYABLE_BY_AUTHORIZATION";
const char UNPLAYABLE_BY_STREAM_CONCURRENCY[] = "UNPLAYABLE_BY_STREAM_CONCURRENCY";
const char OPERATION_REJECTED_UNINTERRUPTIBLE[] = "OPERATION_REJECTED_UNINTERRUPTIBLE";
const char OPERATION_REJECTED_END_OF_QUEUE[] = "OPERATION_REJECTED_END_OF_QUEUE";
const char UNPLAYABLE_BY_REGION[] = "UNPLAYABLE_BY_REGION";
const char OPERATION_UNSUPPORTED[] = "OPERATION_UNSUPPORTED";
const char UNPLAYABLE_BY_PARENTAL_CONTROL[] = "UNPLAYABLE_BY_PARENTAL_CONTROL";
const char UNPLAYABLE_BY_SUBSCRIPTION[] = "UNPLAYABLE_BY_SUBSCRIPTION";
const char OPERATION_REJECTED_SKIP_LIMIT[] = "OPERATION_REJECTED_SKIP_LIMIT";
const char UNKNOWN_ERROR[] = "UNKNOWN_ERROR";

// plaback state strings
const char IDLE[] = "IDLE";
const char PLAYING[] = "PLAYING";
const char PAUSED[] = "PAUSED";
const char STOPPED[] = "STOPPED";
const char FINISHED[] = "FINISHED";

// The key values used in the context payload from External Media Player to AVS.
const char STATE[] = "state";
const char OPERATIONS[] = "supportedOperations";
const char MEDIA[] = "media";
const char POSITIONINMS[] = "positionMilliseconds";
const char SHUFFLE[] = "shuffle";
const char REPEAT[] = "repeat";
const char FAVORITE[] = "favorite";
const char PLAYBACK_SOURCE[] = "playbackSource";
const char TYPE[] = "type";
const char PLAYBACK_SOURCE_ID[] = "playbackSourceId";
const char TRACKNAME[] = "trackName";
const char TRACK_ID[] = "trackId";
const char TRACK_NUMBER[] = "trackNumber";
const char ARTIST[] = "artist";
const char ARTIST_ID[] = "artistId";
const char ALBUM[] = "album";
const char ALBUM_ID[] = "albumId";
const char COVER_URLS[] = "coverUrls";
const char TINY_URL[] = "tiny";
const char SMALL_URL[] = "small";
const char MEDIUM_URL[] = "medium";
const char LARGE_URL[] = "large";
const char COVER_ID[] = "coverId";
const char MEDIA_PROVIDER[] = "mediaProvider";
const char MEDIA_TYPE[] = "mediaType";
const char DURATIONINMS[] = "durationInMilliseconds";
const char VALUE[] = "value";

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNALMEDIAADAPTERCONSTANTS_H
