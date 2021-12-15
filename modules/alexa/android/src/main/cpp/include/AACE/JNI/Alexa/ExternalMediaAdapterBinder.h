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

#ifndef AACE_JNI_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H
#define AACE_JNI_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H

#include <AACE/Alexa/ExternalMediaAdapter.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class ExternalMediaAdapterHandler : public aace::alexa::ExternalMediaAdapter {
public:
    ExternalMediaAdapterHandler(jobject obj);

    // aace::alexa::ExternalMediaAdapter
    bool login(
        const std::string& localPlayerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) override;
    bool logout(const std::string& localPlayerId) override;
    bool play(
        const std::string& localPlayerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        bool preload,
        Navigation navigation) override;
    bool playControl(const std::string& localPlayerId, PlayControlType requestType) override;
    bool seek(const std::string& localPlayerId, std::chrono::milliseconds offset) override;
    bool adjustSeek(const std::string& localPlayerId, std::chrono::milliseconds deltaOffset) override;
    bool authorize(const std::vector<AuthorizedPlayerInfo>& authorizedPlayers) override;
    std::chrono::milliseconds getOffset(const std::string& localPlayerId) override;
    bool getState(const std::string& localPlayerId, ExternalMediaAdapterState& state) override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    JObject m_obj;
};

class ExternalMediaAdapterBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    ExternalMediaAdapterBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_externalMediaAdapterHandler;
    }

    std::shared_ptr<ExternalMediaAdapterHandler> getExternalMediaAdapter() {
        return m_externalMediaAdapterHandler;
    }

private:
    std::shared_ptr<ExternalMediaAdapterHandler> m_externalMediaAdapterHandler;
};

//
// JNavigation
//

class JNavigationConfig : public EnumConfiguration<ExternalMediaAdapterHandler::Navigation> {
public:
    using T = ExternalMediaAdapterHandler::Navigation;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/ExternalMediaAdapter$Navigation";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DEFAULT, "DEFAULT"}, {T::FOREGROUND, "FOREGROUND"}, {T::NONE, "NONE"}};
    }
};

using JNavigation = JEnum<ExternalMediaAdapterHandler::Navigation, JNavigationConfig>;

//
// JPlayControlType
//

class JPlayControlTypeConfig : public EnumConfiguration<ExternalMediaAdapterHandler::PlayControlType> {
public:
    using T = ExternalMediaAdapterHandler::PlayControlType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/ExternalMediaAdapter$PlayControlType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::RESUME, "RESUME"},
                {T::PAUSE, "PAUSE"},
                {T::STOP, "STOP"},
                {T::NEXT, "NEXT"},
                {T::PREVIOUS, "PREVIOUS"},
                {T::START_OVER, "START_OVER"},
                {T::FAST_FORWARD, "FAST_FORWARD"},
                {T::REWIND, "REWIND"},
                {T::ENABLE_REPEAT_ONE, "ENABLE_REPEAT_ONE"},
                {T::ENABLE_REPEAT, "ENABLE_REPEAT"},
                {T::DISABLE_REPEAT, "DISABLE_REPEAT"},
                {T::ENABLE_SHUFFLE, "ENABLE_SHUFFLE"},
                {T::DISABLE_SHUFFLE, "DISABLE_SHUFFLE"},
                {T::FAVORITE, "FAVORITE"},
                {T::UNFAVORITE, "UNFAVORITE"},
                // internal
                {T::PAUSE_RESUME_TOGGLE, "PAUSE_RESUME_TOGGLE"}};
    }
};

using JPlayControlType = JEnum<ExternalMediaAdapterHandler::PlayControlType, JPlayControlTypeConfig>;

//
// JSupportedPlaybackOperation
//

class JSupportedPlaybackOperationConfig
        : public EnumConfiguration<ExternalMediaAdapterHandler::SupportedPlaybackOperation> {
public:
    using T = ExternalMediaAdapterHandler::SupportedPlaybackOperation;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/ExternalMediaAdapter$SupportedPlaybackOperation";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::PLAY, "PLAY"},
                {T::PAUSE, "PAUSE"},
                {T::STOP, "STOP"},
                {T::NEXT, "NEXT"},
                {T::PREVIOUS, "PREVIOUS"},
                {T::START_OVER, "START_OVER"},
                {T::FAST_FORWARD, "FAST_FORWARD"},
                {T::REWIND, "REWIND"},
                {T::ENABLE_REPEAT, "ENABLE_REPEAT"},
                {T::ENABLE_REPEAT_ONE, "ENABLE_REPEAT_ONE"},
                {T::DISABLE_REPEAT, "DISABLE_REPEAT"},
                {T::ENABLE_SHUFFLE, "ENABLE_SHUFFLE"},
                {T::DISABLE_SHUFFLE, "DISABLE_SHUFFLE"},
                {T::FAVORITE, "FAVORITE"},
                {T::UNFAVORITE, "UNFAVORITE"},
                {T::SEEK, "SEEK"},
                {T::ADJUST_SEEK, "ADJUST_SEEK"}};
    }
};

using JSupportedPlaybackOperation =
    JEnum<ExternalMediaAdapterHandler::SupportedPlaybackOperation, JSupportedPlaybackOperationConfig>;

//
// JFavorites
//

class JFavoritesConfig : public EnumConfiguration<ExternalMediaAdapterHandler::Favorites> {
public:
    using T = ExternalMediaAdapterHandler::Favorites;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/ExternalMediaAdapter$Favorites";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::FAVORITED, "FAVORITED"}, {T::UNFAVORITED, "UNFAVORITED"}, {T::NOT_RATED, "NOT_RATED"}};
    }
};

using JFavorites = JEnum<ExternalMediaAdapterHandler::Favorites, JFavoritesConfig>;

//
// JMediaType
//

class JMediaTypeConfig : public EnumConfiguration<ExternalMediaAdapterHandler::MediaType> {
public:
    using T = ExternalMediaAdapterHandler::MediaType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/ExternalMediaAdapter$MediaType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::TRACK, "TRACK"},
                {T::PODCAST, "PODCAST"},
                {T::STATION, "STATION"},
                {T::AD, "AD"},
                {T::SAMPLE, "SAMPLE"},
                {T::OTHER, "OTHER"}};
    }
};

using JMediaType = JEnum<ExternalMediaAdapterHandler::MediaType, JMediaTypeConfig>;

//
// JPlayRequestor
//

// class JPlayRequestorConfig : public EnumConfiguration<ExternalMediaAdapterHandler::PlayRequestor> {
// public:
//     using T = ExternalMediaAdapterHandler::PlayRequestor;

//     const char* getClassName() override {
//         return "com/amazon/aace/alexa/ExternalMediaAdapter$PlayRequestor";
//     }

//     std::vector<std::pair<T,std::string>> getConfiguration() override {
//         return {
//             {T::USER,"USER"},
//             {T::ALERT,"ALERT"}
//         };
//     }
// };

// using JPlayRequestor = JEnum<ExternalMediaAdapterHandler::PlayRequestor,JPlayRequestorConfig>;

//
// JMutedState
//
/*
    class JMutedStateConfig : public EnumConfiguration<ExternalMediaAdapterHandler::MutedState> {
    public:
        using T = ExternalMediaAdapterHandler::MutedState;

        const char* getClassName() override {
            return "com/amazon/aace/alexa/ExternalMediaAdapter$MutedState";
        }

        std::vector<std::pair<T,std::string>> getConfiguration() override {
            return {
                {T::MUTED,"MUTED"},
                {T::UNMUTED,"UNMUTED"}
            };
        }
    };

    using JMutedState = JEnum<ExternalMediaAdapterHandler::MutedState,JMutedStateConfig>;
*/
}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_EXTERNAL_MEDIA_ADAPTER_BINDER_H
