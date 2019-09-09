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

#include <AACE/JNI/Audio/AudioStreamBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.audio.AudioStreamBinder";

namespace aace {
namespace jni {
namespace audio {

    //
    // AudioStreamBinder
    //

    AudioStreamBinder::AudioStreamBinder( std::shared_ptr<aace::audio::AudioStream> stream ) : m_stream( stream ) {
    }

} // aace::jni::audio
} // aace::jni
} // aace

#define AUDIO_STREAM_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioStreamBinder *>( ref )

extern "C"
{
    JNIEXPORT void JNICALL
    Java_com_amazon_aace_audio_AudioStream_disposeBinder( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
            ThrowIfNull( audioStreamBinder, "invalidAudioStreamBinder" );
            delete audioStreamBinder;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_audio_AudioStream_disposeBinder",ex.what());
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_audio_AudioStream_isClosed( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
            ThrowIfNull( audioStreamBinder, "invalidAudioStreamBinder" );

            return audioStreamBinder->getAudioStream()->isClosed();
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_audio_AudioStream_isClosed",ex.what());
            return true;
        }
    }

    JNIEXPORT jint JNICALL
    Java_com_amazon_aace_audio_AudioStream_read( JNIEnv* env, jobject /* this */, jlong ref, jbyteArray data, jlong offset, jlong size )
    {
        try
        {
            auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
            ThrowIfNull( audioStreamBinder, "invalidAudioStreamBinder" );

            return static_cast<jint>( audioStreamBinder->getAudioStream()->read( (char *) JByteArray( data ).ptr(), size ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_audio_AudioStream_read",ex.what());
            return 0;
        }
    }

    JNIEXPORT jobject JNICALL
    Java_com_amazon_aace_audio_AudioStream_getEncoding( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
            ThrowIfNull( audioStreamBinder, "invalidAudioStreamBinder" );

            jobject encodingObj;
            ThrowIfNot( aace::jni::audio::JEncoding::checkType( audioStreamBinder->getAudioStream()->getEncoding(), &encodingObj ), "invalidEncoding" );

            return encodingObj;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_audio_AudioStream_getEncoding",ex.what());
            return nullptr;
        }
    }
}


