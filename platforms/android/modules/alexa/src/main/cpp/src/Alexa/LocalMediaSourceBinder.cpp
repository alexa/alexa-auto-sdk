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

#include <AACE/JNI/Alexa/LocalMediaSourceBinder.h>
#include <AACE/JNI/Alexa/ExternalMediaAdapterBinder.h>
#include <AACE/JNI/Audio/AudioOutputBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.LocalMediaSourceBinder";

// type aliases
using SupportedPlaybackOperation = aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation;
using Source = aace::jni::alexa::LocalMediaSourceHandler::Source;
using ContentSelector = aace::jni::alexa::LocalMediaSourceHandler::ContentSelector;

namespace aace {
namespace jni {
namespace alexa {

//
// LocalMediaSourceBinder
//

LocalMediaSourceBinder::LocalMediaSourceBinder(jobject obj, Source source) {
    m_localMediaSourceHandler = std::shared_ptr<LocalMediaSourceHandler>(new LocalMediaSourceHandler(obj, source));
}

//
// LocalMediaSourceHandler
//

LocalMediaSourceHandler::LocalMediaSourceHandler(jobject obj, Source source) :
        LocalMediaSource(source), m_obj(obj, "com/amazon/aace/alexa/LocalMediaSource") {
}

bool LocalMediaSourceHandler::play(ContentSelector selector, const std::string& payload) {
    try_with_context {
        jboolean result;
        jobject selectorTypeObj;
        ThrowIfNot(JContentSelector::checkType(selector, &selectorTypeObj), "invalidContentSelectorType");
        ThrowIfNot(
            m_obj.invoke(
                "play",
                "(Lcom/amazon/aace/alexa/LocalMediaSource$ContentSelector;Ljava/lang/String;)Z",
                &result,
                selectorTypeObj,
                JString(payload).get()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "play", ex.what());
        return false;
    }
}

bool LocalMediaSourceHandler::playControl(PlayControlType controlType) {
    try_with_context {
        jboolean result;
        jobject controlTypeObj;
        ThrowIfNot(JLocalPlayControlType::checkType(controlType, &controlTypeObj), "invalidControlType");
        ThrowIfNot(
            m_obj.invoke(
                "playControl", "(Lcom/amazon/aace/alexa/LocalMediaSource$PlayControlType;)Z", &result, controlTypeObj),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "playControl", ex.what());
        return false;
    }
}

bool LocalMediaSourceHandler::seek(std::chrono::milliseconds offset) {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("seek", "(J)Z", &result, offset.count()), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "seek", ex.what());
        return false;
    }
}

bool LocalMediaSourceHandler::adjustSeek(std::chrono::milliseconds deltaOffset) {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("adjustSeek", "(J)Z", &result, deltaOffset.count()), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "adjustSeek", ex.what());
        return false;
    }
}

LocalMediaSourceHandler::LocalMediaSourceState LocalMediaSourceHandler::getState() {
    aace::alexa::LocalMediaSource::LocalMediaSourceState state;

    try_with_context {
        jvalue result;
        ThrowIfNot(
            m_obj.invoke("getState", "()Lcom/amazon/aace/alexa/LocalMediaSource$LocalMediaSourceState;", &result.l),
            "invokeMethodFailed");

        JObject localMediaSourceState(result.l, "com/amazon/aace/alexa/LocalMediaSource$LocalMediaSourceState");

        //
        // SessionState
        //

        ThrowIfNot(
            localMediaSourceState.get(
                "sessionState", "Lcom/amazon/aace/alexa/LocalMediaSource$SessionState;", &result.l),
            "getFieldFailed");
        JObject sessionStateObj(result.l, "com/amazon/aace/alexa/LocalMediaSource$SessionState");

        ThrowIfNot(sessionStateObj.get("endpointId", &state.sessionState.endpointId), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("loggedIn", &state.sessionState.loggedIn), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("userName", &state.sessionState.userName), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("isGuest", &state.sessionState.isGuest), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("launched", &state.sessionState.launched), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("active", &state.sessionState.active), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("accessToken", &state.sessionState.accessToken), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("spiVersion", &state.sessionState.spiVersion), "getFieldFailed");
        ThrowIfNot(
            sessionStateObj.get(
                "supportedContentSelectors", "[Lcom/amazon/aace/alexa/LocalMediaSource$ContentSelector;", &result.l),
            "getFieldFailed");

        JObjectArray supportedContentSelectorArr((jobjectArray)result.l);
        std::vector<aace::alexa::LocalMediaSource::ContentSelector> supportedContentSelectors;
        ContentSelector selector;

        for (int j = 0; j < supportedContentSelectorArr.size(); j++) {
            ThrowIfNot(supportedContentSelectorArr.getAt(j, &result.l), "getArrayElementFailed");
            ThrowIfNot(JContentSelector::checkType(result.l, &selector), "invalidContentSelectorType");
            supportedContentSelectors.push_back(selector);
        }
        state.sessionState.supportedContentSelectors = supportedContentSelectors;

        ThrowIfNot(sessionStateObj.get("tokenRefreshInterval", &result.j), "getFieldFailed");
        state.sessionState.tokenRefreshInterval = std::chrono::milliseconds(result.j);

        //
        // PlaybackState
        //

        ThrowIfNot(
            localMediaSourceState.get(
                "playbackState", "Lcom/amazon/aace/alexa/LocalMediaSource$PlaybackState;", &result.l),
            "getFieldFailed");
        JObject playbackStateObj(result.l, "com/amazon/aace/alexa/LocalMediaSource$PlaybackState");

        ThrowIfNot(playbackStateObj.get("state", &state.playbackState.state), "getFieldFailed");
        ThrowIfNot(
            playbackStateObj.get(
                "supportedOperations",
                "[Lcom/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation;",
                &result.l),
            "getFieldFailed");
        if (result.l == nullptr) {
            AACE_JNI_WARN(
                TAG,
                "LocalMediaSourceHandler::getState",
                "Provided supportedOperations is null, defaulting to empty array");
        } else {
            JObjectArray supportedOpsArr((jobjectArray)result.l);
            std::vector<aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation> supportedOperations;
            SupportedPlaybackOperation op;

            for (int j = 0; j < supportedOpsArr.size(); j++) {
                ThrowIfNot(supportedOpsArr.getAt(j, &result.l), "getArrayElementFailed");
                ThrowIfNot(JLocalSupportedPlaybackOperation::checkType(result.l, &op), "invalidPlaybackOperationType");
                supportedOperations.push_back(op);
            }

            state.playbackState.supportedOperations = supportedOperations;
        }

        ThrowIfNot(playbackStateObj.get("trackOffset", &result.j), "getFieldFailed");
        state.playbackState.trackOffset = std::chrono::milliseconds(result.j);

        ThrowIfNot(playbackStateObj.get("shuffleEnabled", &state.playbackState.shuffleEnabled), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("repeatEnabled", &state.playbackState.repeatEnabled), "getFieldFailed");

        ThrowIfNot(
            playbackStateObj.get("favorites", "Lcom/amazon/aace/alexa/LocalMediaSource$Favorites;", &result.l),
            "getFieldFailed");
        ThrowIfNot(JLocalFavorites::checkType(result.l, &state.playbackState.favorites), "invalidFavoritesType");

        ThrowIfNot(playbackStateObj.get("type", &state.playbackState.type), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("playbackSource", &state.playbackState.playbackSource), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("playbackSourceId", &state.playbackState.playbackSourceId), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("trackName", &state.playbackState.trackName), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("trackId", &state.playbackState.trackId), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("artistName", &state.playbackState.artistName), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("artistId", &state.playbackState.artistId), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("albumName", &state.playbackState.albumName), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("albumId", &state.playbackState.albumId), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("tinyURL", &state.playbackState.tinyURL), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("smallURL", &state.playbackState.smallURL), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("mediumURL", &state.playbackState.mediumURL), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("largeURL", &state.playbackState.largeURL), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("coverId", &state.playbackState.coverId), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("mediaProvider", &state.playbackState.mediaProvider), "getFieldFailed");

        ThrowIfNot(
            playbackStateObj.get("mediaType", "Lcom/amazon/aace/alexa/LocalMediaSource$MediaType;", &result.l),
            "getFieldFailed");
        ThrowIfNot(JLocalMediaType::checkType(result.l, &state.playbackState.mediaType), "invalidMediaType");

        ThrowIfNot(playbackStateObj.get("duration", &result.j), "getFieldFailed");
        state.playbackState.duration = std::chrono::milliseconds(result.j);

        return state;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getState", ex.what());
        return state;
    }
}

bool LocalMediaSourceHandler::volumeChanged(float volume) {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("volumeChanged", "(F)Z", &result, volume), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "volumeChanged", ex.what());
        return false;
    }
}

bool LocalMediaSourceHandler::mutedStateChanged(MutedState state) {
    try_with_context {
        jobject mutedStateObj;
        ThrowIfNot(aace::jni::audio::JMutedState::checkType(state, &mutedStateObj), "invalidMutedState");

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "mutedStateChanged", "(Lcom/amazon/aace/audio/AudioOutput$MutedState;)Z", &result, mutedStateObj),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "mutedStateChanged", ex.what());
        return false;
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define LOCAL_MEDIA_SOURCE_BINDER(ref) reinterpret_cast<aace::jni::alexa::LocalMediaSourceBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_createBinder(JNIEnv* env, jobject obj, jobject source) {
    try {
        Source sourceObj;
        ThrowIfNot(aace::jni::alexa::JSource::checkType(source, &sourceObj), "invalidSourceType");
        return reinterpret_cast<long>(new aace::jni::alexa::LocalMediaSourceBinder(obj, sourceObj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_LocalMediaSource_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto localMediaSourceBinder = LOCAL_MEDIA_SOURCE_BINDER(ref);
        ThrowIfNull(localMediaSourceBinder, "invalidLocalMediaSourceBinder");
        delete localMediaSourceBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_LocalMediaSource_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_LocalMediaSource_playerEvent(JNIEnv* env, jobject /* this */, jlong ref, jstring eventName) {
    try {
        auto localMediaSourceBinder = LOCAL_MEDIA_SOURCE_BINDER(ref);
        ThrowIfNull(localMediaSourceBinder, "invalidLocalMediaSourceBinder");

        localMediaSourceBinder->getLocalMediaSource()->playerEvent(JString(eventName).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_LocalMediaSource_playerEvent", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_LocalMediaSource_playerError(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring errorName,
    jlong code,
    jstring description,
    jboolean fatal) {
    try {
        auto localMediaSourceBinder = LOCAL_MEDIA_SOURCE_BINDER(ref);
        ThrowIfNull(localMediaSourceBinder, "invalidLocalMediaSourceBinder");

        localMediaSourceBinder->getLocalMediaSource()->playerError(
            JString(errorName).toStdStr(), code, JString(description).toStdStr(), fatal);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_LocalMediaSource_playerError", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_LocalMediaSource_setFocus(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jboolean focusAcquire) {
    try {
        auto localMediaSourceBinder = LOCAL_MEDIA_SOURCE_BINDER(ref);
        ThrowIfNull(localMediaSourceBinder, "invalidLocalMediaSourceBinder");

        localMediaSourceBinder->getLocalMediaSource()->setFocus(focusAcquire);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_LocalMediaSource_setFocus", ex.what());
    }
}
}
