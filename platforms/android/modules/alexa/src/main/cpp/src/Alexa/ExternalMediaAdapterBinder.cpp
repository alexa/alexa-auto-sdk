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

#include <AACE/JNI/Alexa/ExternalMediaAdapterBinder.h>
#include <AACE/JNI/Audio/AudioOutputBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.ExternalMediaAdapterBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// ExternalMediaAdapterBinder
//

ExternalMediaAdapterBinder::ExternalMediaAdapterBinder(jobject obj) {
    m_externalMediaAdapterHandler = std::shared_ptr<ExternalMediaAdapterHandler>(new ExternalMediaAdapterHandler(obj));
}

//
// ExternalMediaAdapterHandler
//

ExternalMediaAdapterHandler::ExternalMediaAdapterHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/alexa/ExternalMediaAdapter") {
}

bool ExternalMediaAdapterHandler::login(
    const std::string& localPlayerId,
    const std::string& accessToken,
    const std::string& userName,
    bool forceLogin,
    std::chrono::milliseconds tokenRefreshInterval) {
    try_with_context {
        jboolean result;

        ThrowIfNot(
            m_obj.invoke(
                "login",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJ)Z",
                &result,
                JString(localPlayerId).get(),
                JString(accessToken).get(),
                JString(userName).get(),
                forceLogin,
                tokenRefreshInterval.count()),
            "invokeMethodFailed");

        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "login", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::logout(const std::string& localPlayerId) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("logout", "(Ljava/lang/String;)Z", &result, JString(localPlayerId).get()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "logout", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::play(
    const std::string& localPlayerId,
    const std::string& playContextToken,
    int64_t index,
    std::chrono::milliseconds offset,
    bool preload,
    Navigation navigation) {
    try_with_context {
        jobject navigationObj;
        ThrowIfNot(JNavigation::checkType(navigation, &navigationObj), "invalidNavigationType");

        jboolean result;

        ThrowIfNot(
            m_obj.invoke(
                "play",
                "(Ljava/lang/String;Ljava/lang/String;JJZLcom/amazon/aace/alexa/ExternalMediaAdapter$Navigation;)Z",
                &result,
                JString(localPlayerId).get(),
                JString(playContextToken).get(),
                index,
                offset.count(),
                preload,
                navigationObj),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "play", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::playControl(const std::string& localPlayerId, PlayControlType playControlType) {
    try_with_context {
        jobject playControlTypeObj;
        ThrowIfNot(JPlayControlType::checkType(playControlType, &playControlTypeObj), "invalidPlayControlType");

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "playControl",
                "(Ljava/lang/String;Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType;)Z",
                &result,
                JString(localPlayerId).get(),
                playControlTypeObj),
            "invokeMethodFailed");

        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "playControl", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::seek(const std::string& localPlayerId, std::chrono::milliseconds offset) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("seek", "(Ljava/lang/String;J)Z", &result, JString(localPlayerId).get(), offset.count()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "seek", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::adjustSeek(const std::string& localPlayerId, std::chrono::milliseconds deltaOffset) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "adjustSeek", "(Ljava/lang/String;J)Z", &result, JString(localPlayerId).get(), deltaOffset.count()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "adjustSeek", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::authorize(const std::vector<AuthorizedPlayerInfo>& authorizedPlayers) {
    try_with_context {
        JObjectArray arr(authorizedPlayers.size(), "com/amazon/aace/alexa/ExternalMediaAdapter$AuthorizedPlayerInfo");

        for (int j = 0; j < authorizedPlayers.size(); j++) {
            JObject info("com/amazon/aace/alexa/ExternalMediaAdapter$AuthorizedPlayerInfo");

            ThrowIfNot(info.set("localPlayerId", authorizedPlayers[j].localPlayerId), "setFieldFailed");
            ThrowIfNot(info.set("authorized", authorizedPlayers[j].authorized), "setFieldFailed");

            arr.setAt(j, info.get());
        }

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "authorize",
                "([Lcom/amazon/aace/alexa/ExternalMediaAdapter$AuthorizedPlayerInfo;)Z",
                &result,
                arr.get()),
            "invokeMethodFailed");

        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "authorize", ex.what());
        return false;
    }
}

std::chrono::milliseconds ExternalMediaAdapterHandler::getOffset(const std::string& localPlayerId) {
    try_with_context {
        jlong result;
        ThrowIfNot(m_obj.invoke("getOffset", "()J", &result), "invokeMethodFailed");
        return std::chrono::milliseconds(result);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getOffset", ex.what());
        return std::chrono::milliseconds::zero();
    }
}

bool ExternalMediaAdapterHandler::getState(const std::string& localPlayerId, ExternalMediaAdapterState& state) {
    try_with_context {
        JObject stateObj("com/amazon/aace/alexa/ExternalMediaAdapter$ExternalMediaAdapterState");
        jvalue result;

        ThrowIfNot(
            m_obj.invoke(
                "getState",
                "(Ljava/lang/String;Lcom/amazon/aace/alexa/ExternalMediaAdapter$ExternalMediaAdapterState;)Z",
                &result.z,
                JString(localPlayerId).get(),
                stateObj.get()),
            "invokeMethodFailed");
        ReturnIfNot(result.z, false);

        //
        // SessionState
        //

        ThrowIfNot(
            stateObj.get("sessionState", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$SessionState;", &result.l),
            "getFieldFailed")
            JObject sessionStateObj(result.l, "com/amazon/aace/alexa/ExternalMediaAdapter$SessionState");

        ThrowIfNot(sessionStateObj.get("endpointId", &state.sessionState.endpointId), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("loggedIn", &state.sessionState.loggedIn), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("userName", &state.sessionState.userName), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("isGuest", &state.sessionState.isGuest), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("launched", &state.sessionState.launched), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("active", &state.sessionState.active), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("accessToken", &state.sessionState.accessToken), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("spiVersion", &state.sessionState.spiVersion), "getFieldFailed");
        ThrowIfNot(sessionStateObj.get("playerCookie", &state.sessionState.playerCookie), "getFieldFailed");

        ThrowIfNot(sessionStateObj.get("tokenRefreshInterval", &result.j), "getFieldFailed");
        state.sessionState.tokenRefreshInterval = std::chrono::milliseconds(result.j);

        //
        // PlaybackState
        //

        ThrowIfNot(
            stateObj.get("playbackState", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$PlaybackState;", &result.l),
            "getFieldFailed");
        JObject playbackStateObj(result.l, "com/amazon/aace/alexa/ExternalMediaAdapter$PlaybackState");

        ThrowIfNot(playbackStateObj.get("state", &state.playbackState.state), "getFieldFailed");
        ThrowIfNot(
            playbackStateObj.get(
                "supportedOperations",
                "[Lcom/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation;",
                &result.l),
            "getFieldFailed");
        // if supportedOperations is nullptr, log warning and don't iterate
        if (result.l == nullptr) {
            AACE_JNI_WARN(
                TAG,
                "ExternalMediaAdapterHandler::getState",
                "Provided supportedOperations is null, defaulting to empty array");
        } else {
            JObjectArray supportedOpsArr((jobjectArray)result.l);
            std::vector<SupportedPlaybackOperation> supportedOperations;
            SupportedPlaybackOperation op;

            for (int j = 0; j < supportedOpsArr.size(); j++) {
                ThrowIfNot(supportedOpsArr.getAt(j, &result.l), "getArrayElementFailed");
                ThrowIfNot(JSupportedPlaybackOperation::checkType(result.l, &op), "invalidPlaybackOperationType");
                supportedOperations.push_back(op);
            }

            state.playbackState.supportedOperations = supportedOperations;
        }

        ThrowIfNot(playbackStateObj.get("trackOffset", &result.j), "getFieldFailed");
        state.playbackState.trackOffset = std::chrono::milliseconds(result.j);

        ThrowIfNot(playbackStateObj.get("shuffleEnabled", &state.playbackState.shuffleEnabled), "getFieldFailed");
        ThrowIfNot(playbackStateObj.get("repeatEnabled", &state.playbackState.repeatEnabled), "getFieldFailed");

        ThrowIfNot(
            playbackStateObj.get("favorites", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$Favorites;", &result.l),
            "getFieldFailed");
        ThrowIfNot(JFavorites::checkType(result.l, &state.playbackState.favorites), "invalidFavoritesType");

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
            playbackStateObj.get("mediaType", "Lcom/amazon/aace/alexa/ExternalMediaAdapter$MediaType;", &result.l),
            "getFieldFailed");
        ThrowIfNot(JMediaType::checkType(result.l, &state.playbackState.mediaType), "invalidMediaType");

        ThrowIfNot(playbackStateObj.get("duration", &result.j), "getFieldFailed");
        state.playbackState.duration = std::chrono::milliseconds(result.j);

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getState", ex.what());
        return false;
    }
}

bool ExternalMediaAdapterHandler::volumeChanged(float volume) {
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

bool ExternalMediaAdapterHandler::mutedStateChanged(MutedState state) {
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

#define EXTERNAL_MEDIA_ADAPTER_BINDER(ref) reinterpret_cast<aace::jni::alexa::ExternalMediaAdapterBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::ExternalMediaAdapterBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_ExternalMediaAdapter_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        delete externalMediaAdapterBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_reportDiscoveredPlayers(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobjectArray discoveredPlayers) {
    using DiscoveredPlayerInfo = aace::jni::alexa::ExternalMediaAdapterHandler::DiscoveredPlayerInfo;

    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");

        JObjectArray arr(discoveredPlayers);
        std::vector<DiscoveredPlayerInfo> discoveredPlayerInfo;
        jvalue result;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &result.l), "getArrayElementFailed");
            JObject playerInfoObj(result.l, "com/amazon/aace/alexa/ExternalMediaAdapter$DiscoveredPlayerInfo");
            DiscoveredPlayerInfo playerInfo;

            ThrowIfNot(playerInfoObj.get("localPlayerId", &playerInfo.localPlayerId), "getFieldFailed");
            ThrowIfNot(playerInfoObj.get("spiVersion", &playerInfo.spiVersion), "getFieldFailed");
            ThrowIfNot(playerInfoObj.get("validationMethod", &playerInfo.validationMethod), "getFieldFailed");

            ThrowIfNot(playerInfoObj.get("validationData", "[Ljava/lang/String;", &result.l), "getFieldFailed");
            JObjectArray validationDataArr((jobjectArray)result.l);

            for (int i = 0; i < validationDataArr.size(); i++) {
                ThrowIfNot(validationDataArr.getAt(i, &result.l), "getArrayElementFailed");
                playerInfo.validationData.push_back(JString(result.l).toStdStr());
            }

            discoveredPlayerInfo.push_back(playerInfo);
        }

        externalMediaAdapterBinder->getExternalMediaAdapter()->reportDiscoveredPlayers(discoveredPlayerInfo);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_reportDiscoveredPlayers", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_requestToken(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->requestToken(JString(localPlayerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_requestToken", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_loginComplete(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->loginComplete(JString(localPlayerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_loginComplete", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_logoutComplete(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->logoutComplete(JString(localPlayerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_logoutComplete", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId,
    jstring eventName) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->playerEvent(
            JString(localPlayerId).toStdStr(), JString(eventName).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerEvent", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerError(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId,
    jstring errorName,
    jlong code,
    jstring description,
    jboolean fatal) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->playerError(
            JString(localPlayerId).toStdStr(),
            JString(errorName).toStdStr(),
            code,
            JString(description).toStdStr(),
            fatal);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_playerError", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_setFocus(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->setFocus(JString(localPlayerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_setFocus", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_ExternalMediaAdapter_removeDiscoveredPlayer(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring localPlayerId) {
    try {
        auto externalMediaAdapterBinder = EXTERNAL_MEDIA_ADAPTER_BINDER(ref);
        ThrowIfNull(externalMediaAdapterBinder, "invalidExternalMediaAdapterBinder");
        externalMediaAdapterBinder->getExternalMediaAdapter()->removeDiscoveredPlayer(
            JString(localPlayerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_ExternalMediaAdapter_removeDiscoveredPlayer", ex.what());
    }
}
}
