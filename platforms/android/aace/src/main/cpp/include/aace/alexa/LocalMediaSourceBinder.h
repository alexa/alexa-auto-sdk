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

#ifndef AACE_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H
#define AACE_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H

#include "AACE/Alexa/LocalMediaSource.h"
#include "aace/alexa/ExternalMediaAdapterBinder.h"
#include "aace/core/PlatformInterfaceBinder.h"

class LocalMediaSourceBinder : public PlatformInterfaceBinder, public aace::alexa::LocalMediaSource {
public:
    LocalMediaSourceBinder( aace::alexa::LocalMediaSource::Source source, std::shared_ptr<aace::alexa::Speaker> speaker );
    ~LocalMediaSourceBinder();

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool authorize( bool authorized ) override;
    bool play( const std::string & payload ) override;
    bool playControl( aace::alexa::LocalMediaSource::PlayControlType controlType ) override;
    bool seek( std::chrono::milliseconds offset ) override;
    bool adjustSeek( std::chrono::milliseconds deltaOffset ) override;
    aace::alexa::LocalMediaSource::LocalMediaSourceState getState() override;


public:
    //aace::alexa::ExternalMediaAdapter::PlayControlType convertPlayControlType( JNIEnv* env, jobject obj );
    aace::alexa::LocalMediaSource::Source convertSource( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation convertSupportedPlaybackOperation( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::Favorites convertFavorites( JNIEnv* env, jobject obj );
    aace::alexa::ExternalMediaAdapter::MediaType convertMediaType( JNIEnv* env, jobject obj );
    jobject convert( aace::alexa::ExternalMediaAdapter::PlayControlType type );
    jobject convert( aace::alexa::LocalMediaSource::Source source );


private:
    jmethodID m_javaMethod_authorize_authorized = nullptr;
    jmethodID m_javaMethod_play_payload = nullptr;
    jmethodID m_javaMethod_playControl_controlType = nullptr;
    jmethodID m_javaMethod_seek_offset = nullptr;
    jmethodID m_javaMethod_adjustSeek_deltaOffset = nullptr;
    jmethodID m_javaMethod_getState = nullptr;

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

    // Source
    ObjectRef m_enum_Source_BLUETOOTH;
    ObjectRef m_enum_Source_USB;
    ObjectRef m_enum_Source_FM_RADIO;
    ObjectRef m_enum_Source_AM_RADIO;
    ObjectRef m_enum_Source_SATELLITE_RADIO;
    ObjectRef m_enum_Source_LINE_IN;
    ObjectRef m_enum_Source_COMPACT_DISC;

    // LocalMediaSourceState == ExternalMediaAdapterState
    ClassRef m_javaClass_LocalMediaSourceState;
    jfieldID m_javaClass_LocalMediaSourceState_SessionState;
    jfieldID m_javaClass_LocalMediaSourceState_PlaybackState;

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
};

#endif //AACE_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H
