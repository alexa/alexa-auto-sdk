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

AudioStreamBinder::AudioStreamBinder(std::shared_ptr<aace::audio::AudioStream> stream) : m_stream(stream) {
}

}  // namespace audio
}  // namespace jni
}  // namespace aace

#define AUDIO_STREAM_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioStreamBinder*>(ref)

extern "C" {
JNIEXPORT void JNICALL
Java_com_amazon_aace_audio_AudioStream_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");
        delete audioStreamBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_audio_AudioStream_isClosed(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");

        return audioStreamBinder->getAudioStream()->isClosed();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_isClosed", ex.what());
        return true;
    }
}

JNIEXPORT jint JNICALL Java_com_amazon_aace_audio_AudioStream_read(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");

        return static_cast<jint>(audioStreamBinder->getAudioStream()->read((char*)JByteArray(data).ptr(), size));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_read", ex.what());
        return 0;
    }
}

JNIEXPORT jobject JNICALL
Java_com_amazon_aace_audio_AudioStream_getEncoding(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");

        jobject encodingObj;
        ThrowIfNot(
            aace::jni::audio::JAudioStreamEncoding::checkType(
                audioStreamBinder->getAudioStream()->getEncoding(), &encodingObj),
            "invalidEncoding");

        return encodingObj;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_getEncoding", ex.what());
        return nullptr;
    }
}

JNIEXPORT jobject JNICALL
Java_com_amazon_aace_audio_AudioStream_getAudioFormat(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");

        auto audioFormatClass = aace::jni::native::JavaClass::find("com/amazon/aace/audio/AudioFormat");
        ThrowIfNull(audioFormatClass, "findClassFailed");

        // get the audio format from the stream
        auto audioFormat = audioStreamBinder->getAudioStream()->getAudioFormat();

        // get the encoding
        jobject encodingObj;
        ThrowIfNot(
            aace::jni::audio::JAudioFormatEncoding::checkType(audioFormat.getEncoding(), &encodingObj),
            "invalidAudioFormatEncoding");

        // get the format
        jobject sampleFormatObj;
        ThrowIfNot(
            aace::jni::audio::JAudioFormatSampleFormat::checkType(audioFormat.getSampleFormat(), &sampleFormatObj),
            "invalidAudioFormatSampleFormat");

        // get the layout
        jobject layoutObj;
        ThrowIfNot(
            aace::jni::audio::JAudioFormatLayout::checkType(audioFormat.getLayout(), &layoutObj),
            "invalidAudioFormatLayout");

        // get the endianness
        jobject endiannessObj;
        ThrowIfNot(
            aace::jni::audio::JAudioFormatEndianness::checkType(audioFormat.getEndianness(), &endiannessObj),
            "invalidAudioFormatEndianness");

        jobject audioFormatObj = audioFormatClass->newInstance(
            "(Lcom/amazon/aace/audio/AudioFormat$Encoding;Lcom/amazon/aace/audio/AudioFormat$SampleFormat;Lcom/amazon/"
            "aace/audio/AudioFormat$Layout;Lcom/amazon/aace/audio/AudioFormat$Endianness;III)V",
            encodingObj,
            sampleFormatObj,
            layoutObj,
            endiannessObj,
            audioFormat.getSampleRate(),
            audioFormat.getSampleSize(),
            audioFormat.getNumChannels());
        ThrowIfNull(audioFormatObj, "createAudioFormatFailed");

        return audioFormatObj;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_getAudioFormat", ex.what());
        return nullptr;
    }
}

JNIEXPORT jobjectArray JNICALL
Java_com_amazon_aace_audio_AudioStream_getProperties(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioStreamBinder = AUDIO_STREAM_BINDER(ref);
        ThrowIfNull(audioStreamBinder, "invalidAudioStreamBinder");

        // get the property list
        auto properties = audioStreamBinder->getAudioStream()->getProperties();

        // find the java audio stream property class
        auto audioStreamPropertyClass =
            aace::jni::native::JavaClass::find("com/amazon/aace/audio/AudioStream$AudioStreamProperty");
        ThrowIfNull(audioStreamPropertyClass, "findClassFailed");

        // create audio stream property array
        JObjectArray arr(properties.size(), audioStreamPropertyClass);
        ThrowIfNot(arr.isValid(), "invalidObjectArray");

        for (int j = 0; j < properties.size(); j++) {
            jobject audioStreamPropertyObj = audioStreamPropertyClass->newInstance(
                "(Ljava/lang/String;Ljava/lang/String;)V",
                JString(properties[j].getKey()).get(),
                JString(properties[j].getValue()).get());
            ThrowIfNull(audioStreamPropertyObj, "createAudioStreamPropertyFailed");
            ThrowIfNot(arr.setAt(j, audioStreamPropertyObj), "insertArrayElementFailed");
        }

        return arr.get();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioStream_getProperties", ex.what());
        return nullptr;
    }
}
}
