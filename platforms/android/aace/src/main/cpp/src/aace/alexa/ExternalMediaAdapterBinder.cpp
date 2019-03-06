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

#include "aace/alexa/ExternalMediaAdapterBinder.h"

ExternalMediaAdapterBinder::ExternalMediaAdapterBinder( std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::ExternalMediaAdapter( speaker )
{
}

void ExternalMediaAdapterBinder::initialize( JNIEnv* env )
{
    m_javaMethod_login_localPlayerId_accessToken_userName_forceLogin_tokenRefreshInterval = env->GetMethodID( getJavaClass(), "login", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJ)Z" );
    m_javaMethod_logout_localPlayerId = env->GetMethodID( getJavaClass(), "logout", "(Ljava/lang/String;)Z" );
    m_javaMethod_play_localPlayerId_playContextToken_index_offset = env->GetMethodID( getJavaClass(), "play", "(Ljava/lang/String;Ljava/lang/String;JJZLcom/amazon/aace/alexa/ExternalMediaAdapter$Navigation;)Z" );
    m_javaMethod_playControl_localPlayerId_playControlType = env->GetMethodID( getJavaClass(), "playControl", "(Ljava/lang/String;Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;)Z" );
    m_javaMethod_seek_localPlayerId_offset = env->GetMethodID( getJavaClass(), "seek", "(Ljava/lang/String;J)Z" );
    m_javaMethod_adjustSeek_localPlayerId_deltaOffset = env->GetMethodID( getJavaClass(), "adjustSeek", "(Ljava/lang/String;J)Z" );
    m_javaMethod_authorize_authorizedPlayers = env->GetMethodID( getJavaClass(), "authorize", "([Lcom/amazon/aace/alexa/ExternalMediaAdapter$AuthorizedPlayerInfo;)Z" );
    m_javaMethod_getState_localPlayerId = env->GetMethodID( getJavaClass(), "getState", "(Ljava/lang/String;)Lcom/amazon/aace/alexa/ExternalMediaAdapter$ExternalMediaAdapterState;" );

    // PlayControlType
    jclass playControlTypeEnumClass = env->FindClass( "com/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType" );
    m_enum_PlayControlType_RESUME = NativeLib::FindEnum( env, playControlTypeEnumClass, "RESUME", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_PAUSE = NativeLib::FindEnum( env, playControlTypeEnumClass, "PAUSE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_NEXT = NativeLib::FindEnum( env, playControlTypeEnumClass, "NEXT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_PREVIOUS = NativeLib::FindEnum( env, playControlTypeEnumClass, "PREVIOUS", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_START_OVER = NativeLib::FindEnum( env, playControlTypeEnumClass, "START_OVER", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_FAST_FORWARD = NativeLib::FindEnum( env, playControlTypeEnumClass, "FAST_FORWARD", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_REWIND = NativeLib::FindEnum( env, playControlTypeEnumClass, "REWIND", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_REPEAT_ONE = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_REPEAT_ONE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_REPEAT = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_REPEAT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_DISABLE_REPEAT = NativeLib::FindEnum( env, playControlTypeEnumClass, "DISABLE_REPEAT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_ENABLE_SHUFFLE = NativeLib::FindEnum( env, playControlTypeEnumClass, "ENABLE_SHUFFLE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_DISABLE_SHUFFLE = NativeLib::FindEnum( env, playControlTypeEnumClass, "DISABLE_SHUFFLE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_FAVORITE = NativeLib::FindEnum( env, playControlTypeEnumClass, "FAVORITE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );
    m_enum_PlayControlType_UNFAVORITE = NativeLib::FindEnum( env, playControlTypeEnumClass, "UNFAVORITE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;" );

    // SupportedPlaybackOperation
    jclass supportedPlaybackOperationEnumClass = env->FindClass( "com/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation" );
    m_enum_SupportedPlaybackOperation_PLAY = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PLAY", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_RESUME = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "RESUME", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_PAUSE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PAUSE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_STOP = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "STOP", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_NEXT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "NEXT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_PREVIOUS = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "PREVIOUS", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_START_OVER = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "START_OVER", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_FAST_FORWARD = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "FAST_FORWARD", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_REWIND = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "REWIND", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_REPEAT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_REPEAT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_REPEAT_ONE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_REPEAT_ONE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_DISABLE_REPEAT = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "DISABLE_REPEAT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ENABLE_SHUFFLE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ENABLE_SHUFFLE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_DISABLE_SHUFFLE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "DISABLE_SHUFFLE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_FAVORITE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "FAVORITE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_UNFAVORITE = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "UNFAVORITE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_SEEK = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "SEEK", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );
    m_enum_SupportedPlaybackOperation_ADJUST_SEEK = NativeLib::FindEnum( env, supportedPlaybackOperationEnumClass, "ADJUST_SEEK", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;" );

    // Favorites
    jclass favoritesEnumClass = env->FindClass( "com/amazon/aace/alexa/ExternalMediaAdapter$Favorites" );
    m_enum_Favorites_FAVORITED = NativeLib::FindEnum( env, favoritesEnumClass, "FAVORITED", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Favorites;" );
    m_enum_Favorites_UNFAVORITED = NativeLib::FindEnum( env, favoritesEnumClass, "UNFAVORITED", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Favorites;" );
    m_enum_Favorites_NOT_RATED = NativeLib::FindEnum( env, favoritesEnumClass, "NOT_RATED", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Favorites;" );

    // MediaType
    jclass mediaTypeEnumClass = env->FindClass( "com/amazon/aace/alexa/ExternalMediaAdapter$MediaType" );
    m_enum_MediaType_TRACK = NativeLib::FindEnum( env, mediaTypeEnumClass, "TRACK", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );
    m_enum_MediaType_PODCAST = NativeLib::FindEnum( env, mediaTypeEnumClass, "PODCAST", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );
    m_enum_MediaType_STATION = NativeLib::FindEnum( env, mediaTypeEnumClass, "STATION", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );
    m_enum_MediaType_AD = NativeLib::FindEnum( env, mediaTypeEnumClass, "AD", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );
    m_enum_MediaType_SAMPLE = NativeLib::FindEnum( env, mediaTypeEnumClass, "SAMPLE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );
    m_enum_MediaType_OTHER = NativeLib::FindEnum( env, mediaTypeEnumClass, "OTHER", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;" );

    // Navigation
    jclass navigationEnumClass = env->FindClass( "com/amazon/aace/alexa/ExternalMediaAdapter$Navigation" );
    m_enum_Navigation_DEFAULT = NativeLib::FindEnum( env, navigationEnumClass, "DEFAULT", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Navigation;" );
    m_enum_Navigation_FOREGROUND = NativeLib::FindEnum( env, navigationEnumClass, "FOREGROUND", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Navigation;" );
    m_enum_Navigation_NONE = NativeLib::FindEnum( env, navigationEnumClass, "NONE", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Navigation;" );

    // DiscoveredPlayerInfo
    m_javaClass_DiscoveredPlayerInfo = NativeLib::FindClass( env, "com/amazon/aace/alexa/ExternalMediaAdapter$DiscoveredPlayerInfo" );
    m_javaField_DiscoveredPlayerInfo_localPlayerId = env->GetFieldID( m_javaClass_DiscoveredPlayerInfo.get(), "localPlayerId", "Ljava/lang/String;" );
    m_javaField_DiscoveredPlayerInfo_spiVersion = env->GetFieldID( m_javaClass_DiscoveredPlayerInfo.get(), "spiVersion", "Ljava/lang/String;" );
    m_javaField_DiscoveredPlayerInfo_validationMethod = env->GetFieldID( m_javaClass_DiscoveredPlayerInfo.get(), "validationMethod", "Ljava/lang/String;" );
    m_javaField_DiscoveredPlayerInfo_validationData = env->GetFieldID( m_javaClass_DiscoveredPlayerInfo.get(), "validationData", "[Ljava/lang/String;" );

    // AuthorizedPlayerInfo
    m_javaClass_AuthorizedPlayerInfo = NativeLib::FindClass( env, "com/amazon/aace/alexa/ExternalMediaAdapter$AuthorizedPlayerInfo" );
    m_javaField_AuthorizedPlayerInfo_localPlayerId = env->GetFieldID( m_javaClass_AuthorizedPlayerInfo.get(), "localPlayerId", "Ljava/lang/String;" );
    m_javaField_AuthorizedPlayerInfo_authorized = env->GetFieldID( m_javaClass_AuthorizedPlayerInfo.get(), "authorized", "Z" );

    // AdapterState
    m_javaClass_ExternalMediaAdapterState = NativeLib::FindClass( env, "com/amazon/aace/alexa/ExternalMediaAdapter$ExternalMediaAdapterState" );
    m_javaClass_ExternalMediaAdapterState_SessionState = env->GetFieldID( m_javaClass_ExternalMediaAdapterState.get(), "sessionState", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SessionState;");
    m_javaClass_ExternalMediaAdapterState_PlaybackState = env->GetFieldID( m_javaClass_ExternalMediaAdapterState.get(), "playbackState", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlaybackState;");

    // SessionState
    m_javaClass_SessionState = NativeLib::FindClass(env, "com/amazon/aace/alexa/ExternalMediaAdapter$SessionState");
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
    m_javaClass_PlaybackState = NativeLib::FindClass(env, "com/amazon/aace/alexa/ExternalMediaAdapter$PlaybackState");
    m_javaField_PlaybackState_state = env->GetFieldID( m_javaClass_PlaybackState.get(), "state", "Ljava/lang/String;");
    m_javaField_PlaybackState_supportedOperations = env->GetFieldID( m_javaClass_PlaybackState.get(), "supportedOperations", "[Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;");
    m_javaField_PlaybackState_trackOffset = env->GetFieldID( m_javaClass_PlaybackState.get(), "trackOffset", "J");
    m_javaField_PlaybackState_shuffleEnabled = env->GetFieldID( m_javaClass_PlaybackState.get(), "shuffleEnabled", "Z");
    m_javaField_PlaybackState_repeatEnabled = env->GetFieldID( m_javaClass_PlaybackState.get(), "repeatEnabled", "Z");
    m_javaField_PlaybackState_favorites = env->GetFieldID( m_javaClass_PlaybackState.get(), "favorites", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Favorites;");
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
    m_javaField_PlaybackState_mediaType = env->GetFieldID( m_javaClass_PlaybackState.get(), "mediaType", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;");
    m_javaField_PlaybackState_duration = env->GetFieldID( m_javaClass_PlaybackState.get(), "duration", "J");
}

bool ExternalMediaAdapterBinder::login( const std::string& localPlayerId, const std::string& accessToken, const std::string& userName, bool forceLogin, std::chrono::milliseconds tokenRefreshInterval )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_login_localPlayerId_accessToken_userName_forceLogin_tokenRefreshInterval != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );
            jstring accessTokenStr = context.getEnv()->NewStringUTF( accessToken.c_str() );
            jstring userNameStr = context.getEnv()->NewStringUTF( userName.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_login_localPlayerId_accessToken_userName_forceLogin_tokenRefreshInterval, localPlayerIdStr, accessTokenStr, userNameStr, forceLogin, tokenRefreshInterval.count() );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
            context.getEnv()->DeleteLocalRef( accessTokenStr );
            context.getEnv()->DeleteLocalRef( userNameStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::logout( const std::string& localPlayerId )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_logout_localPlayerId != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_logout_localPlayerId, localPlayerIdStr );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::play( const std::string & localPlayerId, const std::string & playContextToken, int64_t index, std::chrono::milliseconds offset, bool preload, Navigation navigation )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_play_localPlayerId_playContextToken_index_offset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );
            jstring playContextTokenStr = context.getEnv()->NewStringUTF( playContextToken.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_play_localPlayerId_playContextToken_index_offset, localPlayerIdStr, playContextTokenStr, index, offset.count(), preload, convert( navigation ) );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
            context.getEnv()->DeleteLocalRef( playContextTokenStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::playControl( const std::string& localPlayerId, aace::alexa::ExternalMediaAdapter::PlayControlType playControlType )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_playControl_localPlayerId_playControlType != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_playControl_localPlayerId_playControlType, localPlayerIdStr, convert( playControlType ) );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::seek( const std::string& localPlayerId, std::chrono::milliseconds offset )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_seek_localPlayerId_offset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_seek_localPlayerId_offset, localPlayerIdStr, offset.count() );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::adjustSeek( const std::string& localPlayerId, std::chrono::milliseconds deltaOffset )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_adjustSeek_localPlayerId_deltaOffset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring localPlayerIdStr = context.getEnv()->NewStringUTF( localPlayerId.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_adjustSeek_localPlayerId_deltaOffset, localPlayerIdStr, deltaOffset.count() );

            context.getEnv()->DeleteLocalRef( localPlayerIdStr );
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::authorize( const std::vector<AuthorizedPlayerInfo>& authorizedPlayers )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_authorize_authorizedPlayers != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jobjectArray localAuthorizedPlayersArr = context.getEnv()->NewObjectArray( authorizedPlayers.size(), m_javaClass_AuthorizedPlayerInfo.get(), NULL );
            for ( int i = 0; i < authorizedPlayers.size(); i++ ) {
                jobject localAuthorizedPlayer = context.getEnv()->AllocObject(m_javaClass_AuthorizedPlayerInfo.get());

                jstring localPlayerIdStr = context.getEnv()->NewStringUTF( authorizedPlayers[i].localPlayerId.c_str() );
                context.getEnv()->SetObjectField( localAuthorizedPlayer, m_javaField_AuthorizedPlayerInfo_localPlayerId, localPlayerIdStr);
                context.getEnv()->SetBooleanField( localAuthorizedPlayer, m_javaField_AuthorizedPlayerInfo_authorized, (jboolean) authorizedPlayers[i].authorized);

                context.getEnv()->SetObjectArrayElement( localAuthorizedPlayersArr, i, localAuthorizedPlayer );

                context.getEnv()->DeleteLocalRef(localAuthorizedPlayer);
                context.getEnv()->DeleteLocalRef(localPlayerIdStr);
            }

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_authorize_authorizedPlayers, localAuthorizedPlayersArr );

            context.getEnv()->DeleteLocalRef(localAuthorizedPlayersArr);
        }
    }
    return result;
}

bool ExternalMediaAdapterBinder::getState( const std::string& localPlayerId, ExternalMediaAdapterState& state )
{
    bool success = false;

    if( getJavaObject() != nullptr && m_javaMethod_getState_localPlayerId != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            JNIEnv *env = context.getEnv();

            jstring localPlayerIdStr = env->NewStringUTF( localPlayerId.c_str() );
            jobject localExternalMediaAdapterState = env->CallObjectMethod( getJavaObject(), m_javaMethod_getState_localPlayerId, localPlayerIdStr);

            if( localExternalMediaAdapterState != nullptr )
            {
                // SessionState
                jobject localSessionState = env->GetObjectField( localExternalMediaAdapterState, m_javaClass_ExternalMediaAdapterState_SessionState);

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
                state.sessionState.accessToken = NativeLib::convert(context.getEnv(), localAccessToken);
                env->DeleteLocalRef(localAccessToken);

                jlong localTokenRefreshInterval = env->GetLongField( localSessionState, m_javaField_SessionState_tokenRefreshInterval);
                state.sessionState.tokenRefreshInterval = std::chrono::milliseconds( localTokenRefreshInterval );

                jstring spiVersion = (jstring) env->GetObjectField(localSessionState, m_javaField_SessionState_spiVersion);
                state.sessionState.spiVersion = NativeLib::convert(context.getEnv(), spiVersion);
                env->DeleteLocalRef(spiVersion);

                jstring playerCookie = (jstring) env->GetObjectField(localSessionState, m_javaField_SessionState_playerCookie);
                state.sessionState.playerCookie = NativeLib::convert(context.getEnv(), playerCookie);
                env->DeleteLocalRef(playerCookie);

                env->DeleteLocalRef(localSessionState);

                // PlaybackState
                jobject localPlaybackState = env->GetObjectField( localExternalMediaAdapterState, m_javaClass_ExternalMediaAdapterState_PlaybackState);

                jstring localState = ( jstring ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_state);
                state.playbackState.state = NativeLib::convert(env, localState);
                env->DeleteLocalRef(localState);

                jobjectArray localSupportedOperations = ( jobjectArray ) env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_supportedOperations);
                int localSupportedOperationsLength = env->GetArrayLength( localSupportedOperations );
                std::vector<SupportedPlaybackOperation> supportedOperations;
                for ( int i = 0; i < localSupportedOperationsLength; i++ ) {
                    SupportedPlaybackOperation operation = convertSupportedPlaybackOperation(env, env->GetObjectArrayElement(localSupportedOperations, i));
                    supportedOperations.push_back( operation );
                }
                state.playbackState.supportedOperations = supportedOperations;
                env->DeleteLocalRef(localSupportedOperations);

                jlong localTrackOffset = env->GetLongField( localPlaybackState, m_javaField_PlaybackState_trackOffset);
                state.playbackState.trackOffset = std::chrono::milliseconds( localTrackOffset );

                jboolean localShuffleEnabled = env->GetBooleanField( localPlaybackState, m_javaField_PlaybackState_shuffleEnabled);
                state.playbackState.shuffleEnabled = localShuffleEnabled;

                jboolean localRepeatEnabled = env->GetBooleanField( localPlaybackState, m_javaField_PlaybackState_repeatEnabled);
                state.playbackState.repeatEnabled = localRepeatEnabled;

                jobject localFavorites = env->GetObjectField( localPlaybackState, m_javaField_PlaybackState_favorites);
                Favorites favorites = convertFavorites(env, localFavorites);
                state.playbackState.favorites = favorites;
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
                MediaType mediaType = convertMediaType(env, localMediaType);
                state.playbackState.mediaType = mediaType;
                env->DeleteLocalRef( localMediaType );

                jlong localDuration = env->GetLongField( localPlaybackState, m_javaField_PlaybackState_duration);
                state.playbackState.duration = std::chrono::milliseconds( localDuration );

                env->DeleteLocalRef( localPlaybackState );
                env->DeleteLocalRef( localExternalMediaAdapterState );

                success = true;
            }

            env->DeleteLocalRef( localPlayerIdStr );
        }
    }
    
    return success;
}

jobject ExternalMediaAdapterBinder::convert( aace::alexa::ExternalMediaAdapter::PlayControlType type )
{
    switch( type )
    {
        case aace::alexa::ExternalMediaAdapter::PlayControlType::RESUME:
            return m_enum_PlayControlType_RESUME.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::PAUSE:
            return m_enum_PlayControlType_PAUSE.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::NEXT:
            return m_enum_PlayControlType_NEXT.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::PREVIOUS:
            return m_enum_PlayControlType_PREVIOUS.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::START_OVER:
            return m_enum_PlayControlType_START_OVER.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::FAST_FORWARD:
            return m_enum_PlayControlType_FAST_FORWARD.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::REWIND:
            return m_enum_PlayControlType_REWIND.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::ENABLE_REPEAT_ONE:
            return m_enum_PlayControlType_ENABLE_REPEAT_ONE.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::ENABLE_REPEAT:
            return m_enum_PlayControlType_ENABLE_REPEAT.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::DISABLE_REPEAT:
            return m_enum_PlayControlType_DISABLE_REPEAT.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::ENABLE_SHUFFLE:
            return m_enum_PlayControlType_ENABLE_SHUFFLE.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::DISABLE_SHUFFLE:
            return m_enum_PlayControlType_DISABLE_SHUFFLE.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::FAVORITE:
            return m_enum_PlayControlType_FAVORITE.get();
        case aace::alexa::ExternalMediaAdapter::PlayControlType::UNFAVORITE:
            return m_enum_PlayControlType_UNFAVORITE.get();
    }
}

jobject ExternalMediaAdapterBinder::convert( aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation operation )
{
    switch( operation )
    {
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PLAY:
            return m_enum_SupportedPlaybackOperation_PLAY.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::RESUME:
            return m_enum_SupportedPlaybackOperation_RESUME.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE:
            return m_enum_SupportedPlaybackOperation_PAUSE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::STOP:
            return m_enum_SupportedPlaybackOperation_STOP.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::NEXT:
            return m_enum_SupportedPlaybackOperation_NEXT.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PREVIOUS:
            return m_enum_SupportedPlaybackOperation_PREVIOUS.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::START_OVER:
            return m_enum_SupportedPlaybackOperation_START_OVER.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAST_FORWARD:
            return m_enum_SupportedPlaybackOperation_FAST_FORWARD.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::REWIND:
            return m_enum_SupportedPlaybackOperation_REWIND.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT:
            return m_enum_SupportedPlaybackOperation_ENABLE_REPEAT.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT_ONE:
            return m_enum_SupportedPlaybackOperation_ENABLE_REPEAT_ONE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_REPEAT:
            return m_enum_SupportedPlaybackOperation_DISABLE_REPEAT.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_SHUFFLE:
            return m_enum_SupportedPlaybackOperation_ENABLE_SHUFFLE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_SHUFFLE:
            return m_enum_SupportedPlaybackOperation_DISABLE_SHUFFLE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAVORITE:
            return m_enum_SupportedPlaybackOperation_FAVORITE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::UNFAVORITE:
            return m_enum_SupportedPlaybackOperation_UNFAVORITE.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::SEEK:
            return m_enum_SupportedPlaybackOperation_SEEK.get();
        case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK:
            return m_enum_SupportedPlaybackOperation_ADJUST_SEEK.get();
    }
}

aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation ExternalMediaAdapterBinder::convertSupportedPlaybackOperation( JNIEnv* env, jobject obj )
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

jobject ExternalMediaAdapterBinder::convert( aace::alexa::ExternalMediaAdapter::Favorites favorites )
{
    switch( favorites )
    {
        case aace::alexa::ExternalMediaAdapter::Favorites::FAVORITED:
            return m_enum_Favorites_FAVORITED.get();
        case aace::alexa::ExternalMediaAdapter::Favorites::UNFAVORITED:
            return m_enum_Favorites_UNFAVORITED.get();
        case aace::alexa::ExternalMediaAdapter::Favorites::NOT_RATED:
            return m_enum_Favorites_NOT_RATED.get();
    }
}

aace::alexa::ExternalMediaAdapter::Favorites ExternalMediaAdapterBinder::convertFavorites( JNIEnv* env, jobject obj )
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

jobject ExternalMediaAdapterBinder::convert( aace::alexa::ExternalMediaAdapter::MediaType type )
{
    switch( type )
    {
        case aace::alexa::ExternalMediaAdapter::MediaType::TRACK:
            return m_enum_MediaType_TRACK.get();
        case aace::alexa::ExternalMediaAdapter::MediaType::PODCAST:
            return m_enum_MediaType_PODCAST.get();
        case aace::alexa::ExternalMediaAdapter::MediaType::STATION:
            return m_enum_MediaType_STATION.get();
        case aace::alexa::ExternalMediaAdapter::MediaType::AD:
            return m_enum_MediaType_AD.get();
        case aace::alexa::ExternalMediaAdapter::MediaType::SAMPLE:
            return m_enum_MediaType_SAMPLE.get();
        case aace::alexa::ExternalMediaAdapter::MediaType::OTHER:
            return m_enum_MediaType_OTHER.get();
    }
}

aace::alexa::ExternalMediaAdapter::MediaType ExternalMediaAdapterBinder::convertMediaType( JNIEnv* env, jobject obj )
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

jobject ExternalMediaAdapterBinder::convert( aace::alexa::ExternalMediaAdapter::Navigation navigation )
{
    switch( navigation )
    {
        case aace::alexa::ExternalMediaAdapter::Navigation::DEFAULT:
            return m_enum_Navigation_DEFAULT.get();
        case aace::alexa::ExternalMediaAdapter::Navigation::FOREGROUND:
            return m_enum_Navigation_FOREGROUND.get();
        case aace::alexa::ExternalMediaAdapter::Navigation::NONE:
            return m_enum_Navigation_NONE.get();
    }
}

std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo> ExternalMediaAdapterBinder::convertDiscoveredPlayerInfo(JNIEnv * env, jobjectArray discoveredPlayers)
{
    std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo> discoveredPlayerInfo;
    int localDiscoveredPlayersLength = env->GetArrayLength( discoveredPlayers );

    for ( int i = 0; i < localDiscoveredPlayersLength; i++ ) {
        aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo playerInfoInstance;
        jobject localPlayerInfo = env->GetObjectArrayElement(discoveredPlayers, i);

        jstring localLocalPlayerId = ( jstring ) env->GetObjectField( localPlayerInfo, m_javaField_DiscoveredPlayerInfo_localPlayerId);
        playerInfoInstance.localPlayerId = NativeLib::convert( env, localLocalPlayerId );
        env->DeleteLocalRef( localLocalPlayerId );

        jstring localSpiVersion = ( jstring ) env->GetObjectField( localPlayerInfo, m_javaField_DiscoveredPlayerInfo_spiVersion);
        playerInfoInstance.spiVersion = NativeLib::convert( env, localSpiVersion );
        env->DeleteLocalRef( localSpiVersion );

        jstring localValidationMethod = ( jstring ) env->GetObjectField( localPlayerInfo, m_javaField_DiscoveredPlayerInfo_validationMethod);
        playerInfoInstance.validationMethod = NativeLib::convert( env, localValidationMethod );
        env->DeleteLocalRef( localValidationMethod );

        std::vector<std::string> validationData;
        jobjectArray localValidationData = ( jobjectArray ) env->GetObjectField( localPlayerInfo, m_javaField_DiscoveredPlayerInfo_validationData);
        int localValidationDataLength = env->GetArrayLength( localValidationData );
        for ( int j = 0; j < localValidationDataLength; j++ ) {
            jstring localValidationDataInstance = ( jstring ) env->GetObjectArrayElement( localValidationData, j);
            validationData.push_back( NativeLib::convert( env, localValidationDataInstance ) );
            env->DeleteLocalRef( localValidationDataInstance );
        }
        playerInfoInstance.validationData = validationData;
        env->DeleteLocalRef( localValidationData );

        discoveredPlayerInfo.push_back( playerInfoInstance );
        env->DeleteLocalRef( localPlayerInfo );
    }
    return discoveredPlayerInfo;
}

std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo > ExternalMediaAdapterBinder::convertAuthorizedPlayerInfo(JNIEnv *env, jobjectArray authorizedPlayers)
{
    std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo > authorizedPlayerInfo;
    int localAuthorizedPlayersLength = env->GetArrayLength( authorizedPlayers );

    for ( int i = 0; i < localAuthorizedPlayersLength; i++ ) {
        aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo playerInfoInstance;
        jobject localPlayerInfo = env->GetObjectArrayElement(authorizedPlayers, i);

        jstring localLocalPlayerId = (jstring) env->GetObjectField(localPlayerInfo, m_javaField_AuthorizedPlayerInfo_localPlayerId);
        playerInfoInstance.localPlayerId = NativeLib::convert(env, localLocalPlayerId);
        env->DeleteLocalRef(localLocalPlayerId);

        jboolean localAuthorized = env->GetBooleanField(localPlayerInfo, m_javaField_AuthorizedPlayerInfo_authorized);
        playerInfoInstance.authorized = localAuthorized;

        authorizedPlayerInfo.push_back( playerInfoInstance );
        env->DeleteLocalRef( localPlayerInfo );
    }
    return authorizedPlayerInfo;
}



// JNI
#define EXTERNALMEDIAADAPTER(cptr) ((ExternalMediaAdapterBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_reportDiscoveredPlayers( JNIEnv * env , jobject /* this */, jlong cptr, jobjectArray discoveredPlayers ) {
    EXTERNALMEDIAADAPTER(cptr)->reportDiscoveredPlayers( EXTERNALMEDIAADAPTER(cptr)->convertDiscoveredPlayerInfo( env, discoveredPlayers ));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_requestToken( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId ) {
    EXTERNALMEDIAADAPTER(cptr)->requestToken( NativeLib::convert( env, localPlayerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_loginComplete( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId ) {
    EXTERNALMEDIAADAPTER(cptr)->loginComplete( NativeLib::convert( env, localPlayerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_logoutComplete( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId ) {
    EXTERNALMEDIAADAPTER(cptr)->logoutComplete( NativeLib::convert( env, localPlayerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerEvent( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId, jstring eventName ) {
    EXTERNALMEDIAADAPTER(cptr)->playerEvent( NativeLib::convert( env, localPlayerId ), NativeLib::convert( env, eventName ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerError( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId, jstring errorName, jlong code, jstring description, jboolean fatal ) {
    EXTERNALMEDIAADAPTER(cptr)->playerError( NativeLib::convert( env, localPlayerId ), NativeLib::convert( env, errorName ), (long) code, NativeLib::convert( env, description ), fatal );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_setFocus( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId ) {
    EXTERNALMEDIAADAPTER(cptr)->setFocus( NativeLib::convert( env, localPlayerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_removeDiscoveredPlayer( JNIEnv * env , jobject /* this */, jlong cptr, jstring localPlayerId ) {
    EXTERNALMEDIAADAPTER(cptr)->removeDiscoveredPlayer( NativeLib::convert( env, localPlayerId ) );
}

}

// END OF FILE
