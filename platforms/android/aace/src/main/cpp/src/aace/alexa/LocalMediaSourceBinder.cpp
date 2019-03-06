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

#include "aace/alexa/LocalMediaSourceBinder.h"


LocalMediaSourceBinder::LocalMediaSourceBinder( aace::alexa::LocalMediaSource::Source source, std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::LocalMediaSource( source, speaker )
{
}

LocalMediaSourceBinder::~LocalMediaSourceBinder() {}

void LocalMediaSourceBinder::initialize( JNIEnv* env )
{
    m_javaMethod_authorize_authorized = env->GetMethodID( getJavaClass(), "authorize", "(Z)Z" );
    m_javaMethod_play_payload = env->GetMethodID( getJavaClass(), "play", "(Ljava/lang/String;)Z" );
    m_javaMethod_playControl_controlType = env->GetMethodID( getJavaClass(), "playControl", "(Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;)Z" );
    m_javaMethod_seek_offset = env->GetMethodID( getJavaClass(), "seek", "(J)Z" );
    m_javaMethod_adjustSeek_deltaOffset = env->GetMethodID( getJavaClass(), "adjustSeek", "(J)Z" );
    m_javaMethod_getState = env->GetMethodID( getJavaClass(), "getState", "()Lcom/amazon/aace/alexa/LocalMediaSource$LocalMediaSourceState;" );

    // PlayControlType
    jclass playControlTypeEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$PlayControlType" );
    m_enum_PlayControlType_PAUSE = NativeLib::FindEnum( env, playControlTypeEnumClass, "PAUSE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_RESUME = NativeLib::FindEnum( env, playControlTypeEnumClass, "RESUME", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_NEXT = NativeLib::FindEnum( env, playControlTypeEnumClass, "NEXT", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_PREVIOUS = NativeLib::FindEnum( env, playControlTypeEnumClass, "PREVIOUS", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_START_OVER = NativeLib::FindEnum( env, playControlTypeEnumClass, "START_OVER", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_FAST_FORWARD = NativeLib::FindEnum( env, playControlTypeEnumClass, "FAST_FORWARD", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_REWIND = NativeLib::FindEnum( env, playControlTypeEnumClass, "REWIND", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_REPEAT_ONE = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_REPEAT_ONE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_REPEAT = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_REPEAT", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_DISABLE_REPEAT = NativeLib::FindEnum( env, playControlTypeEnumClass, "DISABLE_REPEAT", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_SHUFFLE = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_SHUFFLE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_DISABLE_SHUFFLE = NativeLib::FindEnum( env, playControlTypeEnumClass, "DISABLE_SHUFFLE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_FAVORITE = NativeLib::FindEnum( env, playControlTypeEnumClass, "FAVORITE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );
    m_enum_PlayControlType_UNFAVORITE = NativeLib::FindEnum( env, playControlTypeEnumClass, "UNFAVORITE", "Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;" );

    // Source
    jclass sourceEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$Source" );
    m_enum_Source_BLUETOOTH = NativeLib::FindEnum( env, sourceEnumClass, "BLUETOOTH", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_USB = NativeLib::FindEnum( env, sourceEnumClass, "USB", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_FM_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "FM_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_AM_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "AM_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_SATELLITE_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "SATELLITE_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_LINE_IN = NativeLib::FindEnum( env, sourceEnumClass, "LINE_IN", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    m_enum_Source_COMPACT_DISC = NativeLib::FindEnum( env, sourceEnumClass, "COMPACT_DISC", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );

    // AdapterState
    m_javaClass_LocalMediaSourceState = NativeLib::FindClass( env, "com/amazon/aace/alexa/LocalMediaSource$LocalMediaSourceState" );
    m_javaClass_LocalMediaSourceState_SessionState = env->GetFieldID( m_javaClass_LocalMediaSourceState.get(), "sessionState", "Lcom/amazon/aace/alexa/LocalMediaSource$SessionState;");
    m_javaClass_LocalMediaSourceState_PlaybackState = env->GetFieldID( m_javaClass_LocalMediaSourceState.get(), "playbackState", "Lcom/amazon/aace/alexa/LocalMediaSource$PlaybackState;");

    // SessionState
    m_javaClass_SessionState = NativeLib::FindClass(env, "com/amazon/aace/alexa/LocalMediaSource$SessionState");
    m_javaField_SessionState_endpointId = env->GetFieldID( m_javaClass_SessionState.get(), "endpointId", "Ljava/lang/String;");
    m_javaField_SessionState_loggedIn = env->GetFieldID( m_javaClass_SessionState.get(), "loggedIn", "Z");
    m_javaField_SessionState_userName = env->GetFieldID( m_javaClass_SessionState.get(), "userName", "Ljava/lang/String;");
    m_javaField_SessionState_isGuest = env->GetFieldID( m_javaClass_SessionState.get(), "isGuest", "Z");
    m_javaField_SessionState_launched = env->GetFieldID( m_javaClass_SessionState.get(), "launched", "Z");
    m_javaField_SessionState_active = env->GetFieldID( m_javaClass_SessionState.get(), "active", "Z");
    m_javaField_SessionState_accessToken = env->GetFieldID( m_javaClass_SessionState.get(), "accessToken", "Ljava/lang/String;");
    m_javaField_SessionState_tokenRefreshInterval = env->GetFieldID( m_javaClass_SessionState.get(), "tokenRefreshInterval", "J");
    m_javaField_SessionState_playerCookie = env->GetFieldID( m_javaClass_SessionState.get(), "playerCookie", "Ljava/lang/String;");
    m_javaField_SessionState_spiVersion = env->GetFieldID( m_javaClass_SessionState.get(), "spiVersion", "Ljava/lang/String;");

    // PlaybackState
    m_javaClass_PlaybackState = NativeLib::FindClass(env, "com/amazon/aace/alexa/LocalMediaSource$PlaybackState");
    m_javaField_PlaybackState_state = env->GetFieldID( m_javaClass_PlaybackState.get(), "state", "Ljava/lang/String;");
    m_javaField_PlaybackState_supportedOperations = env->GetFieldID( m_javaClass_PlaybackState.get(), "supportedOperations", "[Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;");
    m_javaField_PlaybackState_trackOffset = env->GetFieldID( m_javaClass_PlaybackState.get(), "trackOffset", "J");
    m_javaField_PlaybackState_shuffleEnabled = env->GetFieldID( m_javaClass_PlaybackState.get(), "shuffleEnabled", "Z");
    m_javaField_PlaybackState_repeatEnabled = env->GetFieldID( m_javaClass_PlaybackState.get(), "repeatEnabled", "Z");
    m_javaField_PlaybackState_favorites = env->GetFieldID( m_javaClass_PlaybackState.get(), "favorites", "Lcom/amazon/aace/alexa/LocalMediaSource$Favorites;");
    m_javaField_PlaybackState_type = env->GetFieldID( m_javaClass_PlaybackState.get(), "type", "Ljava/lang/String;");
    m_javaField_PlaybackState_playbackSource = env->GetFieldID( m_javaClass_PlaybackState.get(), "playbackSource", "Ljava/lang/String;");
    m_javaField_PlaybackState_playbackSourceId = env->GetFieldID( m_javaClass_PlaybackState.get(), "playbackSourceId", "Ljava/lang/String;");
    m_javaField_PlaybackState_trackName = env->GetFieldID( m_javaClass_PlaybackState.get(), "trackName", "Ljava/lang/String;");
    m_javaField_PlaybackState_trackId = env->GetFieldID( m_javaClass_PlaybackState.get(), "trackId", "Ljava/lang/String;");
    m_javaField_PlaybackState_trackNumber = env->GetFieldID( m_javaClass_PlaybackState.get(), "trackNumber", "Ljava/lang/String;");
    m_javaField_PlaybackState_artistName = env->GetFieldID( m_javaClass_PlaybackState.get(), "artistName", "Ljava/lang/String;");
    m_javaField_PlaybackState_artistId = env->GetFieldID( m_javaClass_PlaybackState.get(), "artistId", "Ljava/lang/String;");
    m_javaField_PlaybackState_albumName = env->GetFieldID( m_javaClass_PlaybackState.get(), "albumName", "Ljava/lang/String;");
    m_javaField_PlaybackState_albumId = env->GetFieldID( m_javaClass_PlaybackState.get(), "albumId", "Ljava/lang/String;");
    m_javaField_PlaybackState_tinyURL = env->GetFieldID( m_javaClass_PlaybackState.get(), "tinyURL", "Ljava/lang/String;");
    m_javaField_PlaybackState_smallURL = env->GetFieldID( m_javaClass_PlaybackState.get(), "smallURL", "Ljava/lang/String;");
    m_javaField_PlaybackState_mediumURL = env->GetFieldID( m_javaClass_PlaybackState.get(), "mediumURL", "Ljava/lang/String;");
    m_javaField_PlaybackState_largeURL = env->GetFieldID( m_javaClass_PlaybackState.get(), "largeURL", "Ljava/lang/String;");
    m_javaField_PlaybackState_coverId = env->GetFieldID( m_javaClass_PlaybackState.get(), "coverId", "Ljava/lang/String;");
    m_javaField_PlaybackState_mediaProvider = env->GetFieldID( m_javaClass_PlaybackState.get(), "mediaProvider", "Ljava/lang/String;");
    m_javaField_PlaybackState_mediaType = env->GetFieldID( m_javaClass_PlaybackState.get(), "mediaType", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;");
    m_javaField_PlaybackState_duration = env->GetFieldID( m_javaClass_PlaybackState.get(), "duration", "J");

    // SupportedPlaybackOperation
    jclass supportedPlaybackOperationEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation" );
    m_enum_SupportedPlaybackOperation_PLAY = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PLAY", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_RESUME = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "RESUME", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_PAUSE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PAUSE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_STOP = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "STOP", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_NEXT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "NEXT", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_PREVIOUS = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PREVIOUS", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_START_OVER = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "START_OVER", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_FAST_FORWARD = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "FAST_FORWARD", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_REWIND = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "REWIND", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_REPEAT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_REPEAT", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_REPEAT_ONE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_REPEAT_ONE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_DISABLE_REPEAT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "DISABLE_REPEAT", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_SHUFFLE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_SHUFFLE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_DISABLE_SHUFFLE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "DISABLE_SHUFFLE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_FAVORITE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "FAVORITE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_UNFAVORITE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "UNFAVORITE", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_SEEK = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "SEEK", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ADJUST_SEEK = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ADJUST_SEEK", "Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;" );

    // Favorites
    jclass favoritesEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$Favorites" );
    m_enum_Favorites_FAVORITED = NativeLib::FindEnum( env, favoritesEnumClass, "FAVORITED", "Lcom/amazon/aace/alexa/LocalMediaSource$Favorites;" );
    m_enum_Favorites_UNFAVORITED = NativeLib::FindEnum( env, favoritesEnumClass, "UNFAVORITED", "Lcom/amazon/aace/alexa/LocalMediaSource$Favorites;" );
    m_enum_Favorites_NOT_RATED = NativeLib::FindEnum( env, favoritesEnumClass, "NOT_RATED", "Lcom/amazon/aace/alexa/LocalMediaSource$Favorites;" );

    // MediaType
    jclass mediaTypeEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$MediaType" );
    m_enum_MediaType_TRACK = NativeLib::FindEnum( env, mediaTypeEnumClass, "TRACK", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
    m_enum_MediaType_PODCAST = NativeLib::FindEnum( env, mediaTypeEnumClass, "PODCAST", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
    m_enum_MediaType_STATION = NativeLib::FindEnum( env, mediaTypeEnumClass, "STATION", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
    m_enum_MediaType_AD = NativeLib::FindEnum( env, mediaTypeEnumClass, "AD", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
    m_enum_MediaType_SAMPLE = NativeLib::FindEnum( env, mediaTypeEnumClass, "SAMPLE", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
    m_enum_MediaType_OTHER = NativeLib::FindEnum( env, mediaTypeEnumClass, "OTHER", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;" );
}

bool LocalMediaSourceBinder::authorize( bool authorized )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_authorize_authorized != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_authorize_authorized, authorized );
        }
    }
    return result;
}

bool LocalMediaSourceBinder::play( const std::string & payload )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_play_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_play_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
    return result;
}

bool LocalMediaSourceBinder::playControl( aace::alexa::LocalMediaSource::PlayControlType controlType )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_playControl_controlType != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_playControl_controlType, convert( controlType ) );
        }
    }
    return result;
}

bool LocalMediaSourceBinder::seek( std::chrono::milliseconds offset )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_seek_offset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_seek_offset, offset.count());
        }
    }
    return result;
}

bool LocalMediaSourceBinder::adjustSeek( std::chrono::milliseconds deltaOffset )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_adjustSeek_deltaOffset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_adjustSeek_deltaOffset, deltaOffset.count() );
        }
    }
    return result;
}

aace::alexa::LocalMediaSource::LocalMediaSourceState LocalMediaSourceBinder::getState()
{
    aace::alexa::LocalMediaSource::LocalMediaSourceState state;
    if( getJavaObject() != nullptr && m_javaMethod_getState != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            JNIEnv *env = context.getEnv();

            jobject localLocalMediaSourceState = env->CallObjectMethod( getJavaObject(), m_javaMethod_getState );

            if( localLocalMediaSourceState != nullptr )
            {
                // SessionState
                jobject localSessionState = env->GetObjectField( localLocalMediaSourceState, m_javaClass_LocalMediaSourceState_SessionState);

                jstring localEndpointId = ( jstring ) env->GetObjectField( localSessionState, m_javaField_SessionState_endpointId);
                state.sessionState.endpointId = NativeLib::convert(env, localEndpointId);
                env->DeleteLocalRef(localEndpointId);

                jboolean localLoggedIn = env->GetBooleanField( localSessionState, m_javaField_SessionState_loggedIn);
                state.sessionState.loggedIn = localLoggedIn;

                jstring localUserName = ( jstring ) env->GetObjectField( localSessionState, m_javaField_SessionState_userName);
                state.sessionState.userName = NativeLib::convert(env, localUserName);
                env->DeleteLocalRef(localUserName);

                jboolean localIsGuest = env->GetBooleanField( localSessionState, m_javaField_SessionState_isGuest);
                state.sessionState.isGuest = localIsGuest;

                jboolean localLaunched = env->GetBooleanField( localSessionState, m_javaField_SessionState_launched);
                state.sessionState.launched = localLaunched;

                jboolean localActive = env->GetBooleanField( localSessionState, m_javaField_SessionState_active);
                state.sessionState.active = localActive;

                jstring localAccessToken = ( jstring ) env->GetObjectField( localSessionState, m_javaField_SessionState_accessToken);
                state.sessionState.accessToken = NativeLib::convert(env, localAccessToken);
                env->DeleteLocalRef(localAccessToken);

                jlong localTokenRefreshInterval = env->GetLongField( localSessionState, m_javaField_SessionState_tokenRefreshInterval);
                state.sessionState.tokenRefreshInterval = std::chrono::milliseconds( localTokenRefreshInterval );

                jstring spiVersion = (jstring) env->GetObjectField(localSessionState, m_javaField_SessionState_spiVersion);
                state.sessionState.spiVersion = NativeLib::convert(env, spiVersion);
                env->DeleteLocalRef(spiVersion);

                jstring playerCookie = (jstring) env->GetObjectField(localSessionState, m_javaField_SessionState_playerCookie);
                state.sessionState.playerCookie = NativeLib::convert(env, playerCookie);
                env->DeleteLocalRef(playerCookie);

                env->DeleteLocalRef(localSessionState);

                // PlaybackState
                jobject localPlaybackState = env->GetObjectField( localLocalMediaSourceState, m_javaClass_LocalMediaSourceState_PlaybackState);

                jstring localState = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_state);
                state.playbackState.state = NativeLib::convert(env, localState);
                env->DeleteLocalRef(localState);

                jobjectArray localSupportedOperations = ( jobjectArray ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_supportedOperations);
                int localSupportedOperationsLength = env->GetArrayLength( localSupportedOperations );
                std::vector<ExternalMediaAdapterBinder::SupportedPlaybackOperation> supportedOperations;
                for ( int i = 0; i < localSupportedOperationsLength; i++ ) {
                    supportedOperations.push_back( convertSupportedPlaybackOperation(env, env->GetObjectArrayElement(localSupportedOperations, i)) );
                }
                state.playbackState.supportedOperations = supportedOperations;
                env->DeleteLocalRef(localSupportedOperations);
                supportedOperations.clear();

                jlong localTrackOffset = env->GetLongField( localPlaybackState, m_javaField_PlaybackState_trackOffset);
                state.playbackState.trackOffset = std::chrono::milliseconds( (long) localTrackOffset );

                jboolean localShuffleEnabled = env->GetBooleanField( localPlaybackState, m_javaField_PlaybackState_shuffleEnabled);
                state.playbackState.shuffleEnabled = localShuffleEnabled;

                jboolean localRepeatEnabled = env->GetBooleanField( localPlaybackState, m_javaField_PlaybackState_repeatEnabled);
                state.playbackState.repeatEnabled = localRepeatEnabled;

                jobject localFavorites = env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_favorites);
                state.playbackState.favorites = convertFavorites(env, localFavorites);
                env->DeleteLocalRef( localFavorites );

                jstring localType = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_type);
                state.playbackState.type = NativeLib::convert(env, localType);
                env->DeleteLocalRef(localType);

                jstring localPlaybackSource = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_playbackSource);
                state.playbackState.playbackSource = NativeLib::convert(env, localPlaybackSource);
                env->DeleteLocalRef(localPlaybackSource);

                jstring localPlaybackSourceId = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_playbackSourceId);
                state.playbackState.playbackSourceId = NativeLib::convert(env, localPlaybackSourceId);
                env->DeleteLocalRef(localPlaybackSourceId);

                jstring localTrackName = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_trackName);
                state.playbackState.trackName = NativeLib::convert(env, localTrackName);
                env->DeleteLocalRef(localTrackName);

                jstring localTrackId = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_trackId);
                state.playbackState.trackId = NativeLib::convert(env, localTrackId);
                env->DeleteLocalRef(localTrackId);

                jstring localArtistName = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_artistName);
                state.playbackState.artistName = NativeLib::convert(env, localArtistName);
                env->DeleteLocalRef(localArtistName);

                jstring localArtistId = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_artistId);
                state.playbackState.artistId = NativeLib::convert(env, localArtistId);
                env->DeleteLocalRef(localArtistId);

                jstring localAlbumName = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_albumName);
                state.playbackState.albumName = NativeLib::convert(env, localAlbumName);
                env->DeleteLocalRef(localAlbumName);

                jstring localAlbumId = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_albumId);
                state.playbackState.albumId = NativeLib::convert(env, localAlbumId);
                env->DeleteLocalRef(localAlbumId);

                jstring localTinyURL = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_tinyURL);
                state.playbackState.tinyURL = NativeLib::convert(env, localTinyURL);
                env->DeleteLocalRef(localTinyURL);

                jstring localSmallURL = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_smallURL);
                state.playbackState.smallURL = NativeLib::convert(env, localSmallURL);
                env->DeleteLocalRef(localSmallURL);

                jstring localMediumURL = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_mediumURL);
                state.playbackState.mediumURL = NativeLib::convert(env, localMediumURL);
                env->DeleteLocalRef(localMediumURL);

                jstring localLargeURL = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_largeURL);
                state.playbackState.largeURL = NativeLib::convert(env, localLargeURL);
                env->DeleteLocalRef(localLargeURL);

                jstring localCoverId = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_coverId);
                state.playbackState.coverId = NativeLib::convert(env, localCoverId);
                env->DeleteLocalRef(localCoverId);

                jstring localMediaProvider = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_mediaProvider);
                state.playbackState.mediaProvider = NativeLib::convert(env, localMediaProvider);
                env->DeleteLocalRef(localMediaProvider);

                jobject localMediaType = env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_mediaType);
                state.playbackState.mediaType = convertMediaType(env, localMediaType);
                env->DeleteLocalRef( localMediaType );


                jlong localDuration = env->GetLongField( localPlaybackState, m_javaField_PlaybackState_duration);
                state.playbackState.duration = std::chrono::milliseconds( (long) localDuration );

                env->DeleteLocalRef( localPlaybackState );
                env->DeleteLocalRef( localLocalMediaSourceState );
            }
        }
    }
    return state;
}

jobject LocalMediaSourceBinder::convert( aace::alexa::LocalMediaSource::PlayControlType type )
{
    switch( type )
    {
        case aace::alexa::LocalMediaSource::PlayControlType::PAUSE:
            return m_enum_PlayControlType_PAUSE.get();
        case aace::alexa::LocalMediaSource::PlayControlType::RESUME:
            return m_enum_PlayControlType_RESUME.get();
        case aace::alexa::LocalMediaSource::PlayControlType::NEXT:
            return m_enum_PlayControlType_NEXT.get();
        case aace::alexa::LocalMediaSource::PlayControlType::PREVIOUS:
            return m_enum_PlayControlType_PREVIOUS.get();
        case aace::alexa::LocalMediaSource::PlayControlType::START_OVER:
            return m_enum_PlayControlType_START_OVER.get();
        case aace::alexa::LocalMediaSource::PlayControlType::FAST_FORWARD:
            return m_enum_PlayControlType_FAST_FORWARD.get();
        case aace::alexa::LocalMediaSource::PlayControlType::REWIND:
            return m_enum_PlayControlType_REWIND.get();
        case aace::alexa::LocalMediaSource::PlayControlType::ENABLE_REPEAT_ONE:
            return m_enum_PlayControlType_ENABLE_REPEAT_ONE.get();
        case aace::alexa::LocalMediaSource::PlayControlType::ENABLE_REPEAT:
            return m_enum_PlayControlType_ENABLE_REPEAT.get();
        case aace::alexa::LocalMediaSource::PlayControlType::DISABLE_REPEAT:
            return m_enum_PlayControlType_DISABLE_REPEAT.get();
        case aace::alexa::LocalMediaSource::PlayControlType::ENABLE_SHUFFLE:
            return m_enum_PlayControlType_ENABLE_SHUFFLE.get();
        case aace::alexa::LocalMediaSource::PlayControlType::DISABLE_SHUFFLE:
            return m_enum_PlayControlType_DISABLE_SHUFFLE.get();
        case aace::alexa::LocalMediaSource::PlayControlType::FAVORITE:
            return m_enum_PlayControlType_FAVORITE.get();
        case aace::alexa::LocalMediaSource::PlayControlType::UNFAVORITE:
            return m_enum_PlayControlType_UNFAVORITE.get();
    }
}

/*
aace::alexa::LocalMediaSource::PlayControlType LocalMediaSourceBinder::convertPlayControlType( JNIEnv* env, jobject obj )
{
    if( m_enum_PlayControlType_PAUSE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::PAUSE;
    }
    else if( m_enum_PlayControlType_RESUME.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::RESUME;
    }
    else if( m_enum_PlayControlType_NEXT.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::NEXT;
    }
    else if( m_enum_PlayControlType_PREVIOUS.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::PREVIOUS;
    }
    else if( m_enum_PlayControlType_START_OVER.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::START_OVER;
    }
    else if( m_enum_PlayControlType_FAST_FORWARD.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::FAST_FORWARD;
    }
    else if( m_enum_PlayControlType_REWIND.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::REWIND;
    }
    else if( m_enum_PlayControlType_ENABLE_REPEAT_ONE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::ENABLE_REPEAT_ONE;
    }
    else if( m_enum_PlayControlType_ENABLE_REPEAT.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::ENABLE_REPEAT;
    }
    else if( m_enum_PlayControlType_DISABLE_REPEAT.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::DISABLE_REPEAT;
    }
    else if( m_enum_PlayControlType_ENABLE_SHUFFLE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::ENABLE_SHUFFLE;
    }
    else if( m_enum_PlayControlType_DISABLE_SHUFFLE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::DISABLE_SHUFFLE;
    }
    else if( m_enum_PlayControlType_FAVORITE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::FAVORITE;
    }
    else if( m_enum_PlayControlType_UNFAVORITE.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::PlayControlType::UNFAVORITE;
    }
    else {
        return aace::alexa::LocalMediaSource::PlayControlType::UNFAVORITE;
    }
}
*/

jobject LocalMediaSourceBinder::convert( aace::alexa::LocalMediaSource::Source source )
{
    switch( source )
    {
        case aace::alexa::LocalMediaSource::Source::BLUETOOTH:
            return m_enum_Source_BLUETOOTH.get();
        case aace::alexa::LocalMediaSource::Source::USB:
            return m_enum_Source_USB.get();
        case aace::alexa::LocalMediaSource::Source::FM_RADIO:
            return m_enum_Source_FM_RADIO.get();
        case aace::alexa::LocalMediaSource::Source::AM_RADIO:
            return m_enum_Source_AM_RADIO.get();
        case aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO:
            return m_enum_Source_SATELLITE_RADIO.get();
        case aace::alexa::LocalMediaSource::Source::LINE_IN:
            return m_enum_Source_LINE_IN.get();
        case aace::alexa::LocalMediaSource::Source::COMPACT_DISC:
            return m_enum_Source_COMPACT_DISC.get();
    }
}

aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation LocalMediaSourceBinder::convertSupportedPlaybackOperation( JNIEnv* env, jobject obj )
{
    if( m_enum_SupportedPlaybackOperation_PLAY.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PLAY;
    }
    else if( m_enum_SupportedPlaybackOperation_RESUME.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::RESUME;
    }
    else if( m_enum_SupportedPlaybackOperation_PAUSE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE;
    }
    else if( m_enum_SupportedPlaybackOperation_STOP.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::STOP;
    }
    else if( m_enum_SupportedPlaybackOperation_NEXT.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::NEXT;
    }
    else if( m_enum_SupportedPlaybackOperation_PREVIOUS.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PREVIOUS;
    }
    else if( m_enum_SupportedPlaybackOperation_START_OVER.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::START_OVER;
    }
    else if( m_enum_SupportedPlaybackOperation_FAST_FORWARD.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAST_FORWARD;
    }
    else if( m_enum_SupportedPlaybackOperation_REWIND.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::REWIND;
    }
    else if( m_enum_SupportedPlaybackOperation_ENABLE_REPEAT.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT;
    }
    else if( m_enum_SupportedPlaybackOperation_ENABLE_REPEAT_ONE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT_ONE;
    }
    else if( m_enum_SupportedPlaybackOperation_DISABLE_REPEAT.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_REPEAT;
    }
    else if( m_enum_SupportedPlaybackOperation_ENABLE_SHUFFLE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_SHUFFLE;
    }
    else if( m_enum_SupportedPlaybackOperation_DISABLE_SHUFFLE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_SHUFFLE;
    }
    else if( m_enum_SupportedPlaybackOperation_FAVORITE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAVORITE;
    }
    else if( m_enum_SupportedPlaybackOperation_UNFAVORITE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::UNFAVORITE;
    }
    else if( m_enum_SupportedPlaybackOperation_SEEK.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::SEEK;
    }
    else if( m_enum_SupportedPlaybackOperation_ADJUST_SEEK.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK;
    }
    else {
        return aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK;
    }
}

aace::alexa::LocalMediaSource::Source LocalMediaSourceBinder::convertSource( JNIEnv* env, jobject obj )
{
    if( m_enum_Source_BLUETOOTH.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::BLUETOOTH;
    }
    else if( m_enum_Source_USB.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::USB;
    }
    else if( m_enum_Source_FM_RADIO.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::FM_RADIO;
    }
    else if( m_enum_Source_AM_RADIO.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::AM_RADIO;
    }
    else if( m_enum_Source_SATELLITE_RADIO.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO;
    }
    else if( m_enum_Source_LINE_IN.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::LINE_IN;
    }
    else if( m_enum_Source_COMPACT_DISC.isSameObject( env, obj ) ) {
        return aace::alexa::LocalMediaSource::Source::COMPACT_DISC;
    }
    else {
        return aace::alexa::LocalMediaSource::Source::COMPACT_DISC;
    }
}

aace::alexa::ExternalMediaAdapter::MediaType LocalMediaSourceBinder::convertMediaType( JNIEnv* env, jobject obj )
{
    if( m_enum_MediaType_TRACK.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::TRACK;
    }
    else if( m_enum_MediaType_PODCAST.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::PODCAST;
    }
    else if( m_enum_MediaType_STATION.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::STATION;
    }
    else if( m_enum_MediaType_AD.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::AD;
    }
    else if( m_enum_MediaType_SAMPLE.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::SAMPLE;
    }
    else if( m_enum_MediaType_OTHER.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::MediaType::OTHER;
    }
    else {
        return aace::alexa::ExternalMediaAdapter::MediaType::OTHER;
    }
}

aace::alexa::ExternalMediaAdapter::Favorites LocalMediaSourceBinder::convertFavorites( JNIEnv* env, jobject obj )
{
    if( m_enum_Favorites_FAVORITED.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::Favorites::FAVORITED;
    }
    else if( m_enum_Favorites_UNFAVORITED.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::Favorites::UNFAVORITED;
    }
    else if( m_enum_Favorites_NOT_RATED.isSameObject( env, obj ) ) {
        return aace::alexa::ExternalMediaAdapter::Favorites::NOT_RATED;
    }
    else {
        return aace::alexa::ExternalMediaAdapter::Favorites::NOT_RATED;
    }
}

// JNI
#define LOCALMEDIASOURCE(cptr) ((LocalMediaSourceBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_playerEvent( JNIEnv * env , jobject /* this */, jlong cptr, jstring eventName ) {
    LOCALMEDIASOURCE(cptr)->playerEvent( NativeLib::convert( env, eventName ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_playerError( JNIEnv * env , jobject /* this */, jlong cptr, jstring errorName, jlong code, jstring description, jboolean fatal ) {
    LOCALMEDIASOURCE(cptr)->playerError( NativeLib::convert( env, errorName ), (long) code, NativeLib::convert( env, description ), fatal );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_setFocus( JNIEnv * env , jobject /* this */, jlong cptr ) {
    LOCALMEDIASOURCE(cptr)->setFocus();
}

}

// END OF FILE
