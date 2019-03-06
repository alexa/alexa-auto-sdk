/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/***************************************************************************************
 * CAUTION: THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT MODIFY THIS FILE!          *
 *          ANY NECESSARY MODIFICATIONS SHOULD BE INCORPORATED INTO THE GENERATOR.     *
 ***************************************************************************************/

#ifndef AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H
#define AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H

#include "AACE/Alexa/ExternalMediaAdapter.h"
#include "aace/core/PlatformInterfaceBinder.h"

class ExternalMediaAdapterBinder : public PlatformInterfaceBinder, public aace::alexa::ExternalMediaAdapter {
public:
    ExternalMediaAdapterBinder( std::shared_ptr<aace::alexa::Speaker> speaker );

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool login( const std::string & localPlayerId, const std::string & accessToken, const std::string & userName, bool forceLogin, std::chrono::milliseconds tokenRefreshInterval ) override;
    bool logout( const std::string & localPlayerId ) override;
    bool play( const std::string & localPlayerId, const std::string & playContextToken, int64_t index, std::chrono::milliseconds offset, bool preload, Navigation navigation ) override;
    bool playControl( const std::string & localPlayerId, PlayControlType requestType ) override;
    bool seek( const std::string & localPlayerId, std::chrono::milliseconds offset ) override;
    bool adjustSeek( const std::string & localPlayerId, std::chrono::milliseconds deltaOffset ) override;
    bool authorize( const std::vector<AuthorizedPlayerInfo>& authorizedPlayers ) override;
    bool getState( const std::string& localPlayerId, ExternalMediaAdapterState& state ) override;

public:
    aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation convertSupportedPlaybackOperation( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::Favorites convertFavorites( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::MediaType convertMediaType( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::Navigation convertNavigation( JNIEnv* env, jobject obj );

    std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo> convertDiscoveredPlayerInfo( JNIEnv* env, jobjectArray obj );
    std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo> convertAuthorizedPlayerInfo( JNIEnv* env, jobjectArray obj );

private:
    jobject convert( aace::alexa::ExternalMediaAdapter::PlayControlType type );
    jobject convert( aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation operation );
    jobject convert( aace::alexa::ExternalMediaAdapter::Favorites favorites );
    jobject convert( aace::alexa::ExternalMediaAdapter::MediaType type );
    jobject convert( aace::alexa::ExternalMediaAdapter::Navigation navigation );

private:
    jmethodID m_javaMethod_login_localPlayerId_accessToken_userName_forceLogin_tokenRefreshInterval = nullptr;
    jmethodID m_javaMethod_logout_localPlayerId = nullptr;
    jmethodID m_javaMethod_play_localPlayerId_playContextToken_index_offset = nullptr;
    jmethodID m_javaMethod_playControl_localPlayerId_playControlType = nullptr;
    jmethodID m_javaMethod_seek_localPlayerId_offset = nullptr;
    jmethodID m_javaMethod_adjustSeek_localPlayerId_deltaOffset = nullptr;
    jmethodID m_javaMethod_authorize_authorizedPlayers = nullptr;
    jmethodID m_javaMethod_getState_localPlayerId = nullptr;

    // PlayControlType
    ObjectRef m_enum_PlayControlType_PAUSE;
    ObjectRef m_enum_PlayControlType_RESUME;
    ObjectRef m_enum_PlayControlType_NEXT;
    ObjectRef m_enum_PlayControlType_PREVIOUS;
    ObjectRef m_enum_PlayControlType_START_OVER;
    ObjectRef m_enum_PlayControlType_FAST_FORWARD;
    ObjectRef m_enum_PlayControlType_REWIND;
    ObjectRef m_enum_PlayControlType_ENABLE_REPEAT_ONE;
    ObjectRef m_enum_PlayControlType_ENABLE_REPEAT;
    ObjectRef m_enum_PlayControlType_DISABLE_REPEAT;
    ObjectRef m_enum_PlayControlType_ENABLE_SHUFFLE;
    ObjectRef m_enum_PlayControlType_DISABLE_SHUFFLE;
    ObjectRef m_enum_PlayControlType_FAVORITE;
    ObjectRef m_enum_PlayControlType_UNFAVORITE;

    // SupportedPlaybackOperation
    ObjectRef m_enum_SupportedPlaybackOperation_PLAY;
    ObjectRef m_enum_SupportedPlaybackOperation_RESUME;
    ObjectRef m_enum_SupportedPlaybackOperation_PAUSE;
    ObjectRef m_enum_SupportedPlaybackOperation_STOP;
    ObjectRef m_enum_SupportedPlaybackOperation_NEXT;
    ObjectRef m_enum_SupportedPlaybackOperation_PREVIOUS;
    ObjectRef m_enum_SupportedPlaybackOperation_START_OVER;
    ObjectRef m_enum_SupportedPlaybackOperation_FAST_FORWARD;
    ObjectRef m_enum_SupportedPlaybackOperation_REWIND;
    ObjectRef m_enum_SupportedPlaybackOperation_ENABLE_REPEAT;
    ObjectRef m_enum_SupportedPlaybackOperation_ENABLE_REPEAT_ONE;
    ObjectRef m_enum_SupportedPlaybackOperation_DISABLE_REPEAT;
    ObjectRef m_enum_SupportedPlaybackOperation_ENABLE_SHUFFLE;
    ObjectRef m_enum_SupportedPlaybackOperation_DISABLE_SHUFFLE;
    ObjectRef m_enum_SupportedPlaybackOperation_FAVORITE;
    ObjectRef m_enum_SupportedPlaybackOperation_UNFAVORITE;
    ObjectRef m_enum_SupportedPlaybackOperation_SEEK;
    ObjectRef m_enum_SupportedPlaybackOperation_ADJUST_SEEK;

    // Favorites
    ObjectRef m_enum_Favorites_FAVORITED;
    ObjectRef m_enum_Favorites_UNFAVORITED;
    ObjectRef m_enum_Favorites_NOT_RATED;

    // MediaType
    ObjectRef m_enum_MediaType_TRACK;
    ObjectRef m_enum_MediaType_PODCAST;
    ObjectRef m_enum_MediaType_STATION;
    ObjectRef m_enum_MediaType_AD;
    ObjectRef m_enum_MediaType_SAMPLE;
    ObjectRef m_enum_MediaType_OTHER;

    // Navigation
    ObjectRef m_enum_Navigation_DEFAULT;
    ObjectRef m_enum_Navigation_NONE;
    ObjectRef m_enum_Navigation_FOREGROUND;

    // DiscoveredPlayerInfo
    ClassRef m_javaClass_DiscoveredPlayerInfo;
    jfieldID m_javaField_DiscoveredPlayerInfo_localPlayerId;
    jfieldID m_javaField_DiscoveredPlayerInfo_spiVersion;
    jfieldID m_javaField_DiscoveredPlayerInfo_validationMethod;
    jfieldID m_javaField_DiscoveredPlayerInfo_validationData;

    // AuthorizedPlayerInfo
    ClassRef m_javaClass_AuthorizedPlayerInfo;
    jfieldID m_javaField_AuthorizedPlayerInfo_localPlayerId;
    jfieldID m_javaField_AuthorizedPlayerInfo_authorized;

    // ExternalMediaAdapterState
    ClassRef m_javaClass_ExternalMediaAdapterState;
    jfieldID m_javaClass_ExternalMediaAdapterState_SessionState;
    jfieldID m_javaClass_ExternalMediaAdapterState_PlaybackState;

    // SessionState
    ClassRef m_javaClass_SessionState;
    jfieldID m_javaField_SessionState_endpointId;
    jfieldID m_javaField_SessionState_loggedIn;
    jfieldID m_javaField_SessionState_userName;
    jfieldID m_javaField_SessionState_isGuest;
    jfieldID m_javaField_SessionState_launched;
    jfieldID m_javaField_SessionState_active;
    jfieldID m_javaField_SessionState_accessToken;
    jfieldID m_javaField_SessionState_tokenRefreshInterval;
    jfieldID m_javaField_SessionState_playerCookie;
    jfieldID m_javaField_SessionState_spiVersion;

    // PlaybackState
    ClassRef m_javaClass_PlaybackState;
    jfieldID m_javaField_PlaybackState_state;
    jfieldID m_javaField_PlaybackState_supportedOperations;
    jfieldID m_javaField_PlaybackState_trackOffset;
    jfieldID m_javaField_PlaybackState_shuffleEnabled;
    jfieldID m_javaField_PlaybackState_repeatEnabled;
    jfieldID m_javaField_PlaybackState_favorites;
    jfieldID m_javaField_PlaybackState_type;
    jfieldID m_javaField_PlaybackState_playbackSource;
    jfieldID m_javaField_PlaybackState_playbackSourceId;
    jfieldID m_javaField_PlaybackState_trackName;
    jfieldID m_javaField_PlaybackState_trackId;
    jfieldID m_javaField_PlaybackState_trackNumber;
    jfieldID m_javaField_PlaybackState_artistName;
    jfieldID m_javaField_PlaybackState_artistId;
    jfieldID m_javaField_PlaybackState_albumName;
    jfieldID m_javaField_PlaybackState_albumId;
    jfieldID m_javaField_PlaybackState_tinyURL;
    jfieldID m_javaField_PlaybackState_smallURL;
    jfieldID m_javaField_PlaybackState_mediumURL;
    jfieldID m_javaField_PlaybackState_largeURL;
    jfieldID m_javaField_PlaybackState_coverId;
    jfieldID m_javaField_PlaybackState_mediaProvider;
    jfieldID m_javaField_PlaybackState_mediaType;
    jfieldID m_javaField_PlaybackState_duration;
};

#endif //AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H
