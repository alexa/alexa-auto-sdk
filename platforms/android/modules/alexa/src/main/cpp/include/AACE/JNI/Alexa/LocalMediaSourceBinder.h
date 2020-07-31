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

#ifndef AACE_JNI_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H
#define AACE_JNI_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H

#include <AACE/Alexa/LocalMediaSource.h>
#include <AACE/Alexa/ExternalMediaAdapter.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>
#include "ExternalMediaAdapterBinder.h"

namespace aace {
namespace jni {
namespace alexa {

class LocalMediaSourceHandler : public aace::alexa::LocalMediaSource {
public:
    LocalMediaSourceHandler(jobject obj, Source source);

    // aace::alexa::LocalMediaSource
    bool play(ContentSelector selector, const std::string& payload) override;
    bool playControl(PlayControlType controlType) override;
    bool seek(std::chrono::milliseconds offset) override;
    bool adjustSeek(std::chrono::milliseconds deltaOffset) override;
    LocalMediaSourceState getState() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    JObject m_obj;
};

class LocalMediaSourceBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    LocalMediaSourceBinder(jobject obj, LocalMediaSourceHandler::Source source);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_localMediaSourceHandler;
    }

    std::shared_ptr<LocalMediaSourceHandler> getLocalMediaSource() {
        return m_localMediaSourceHandler;
    }

private:
    std::shared_ptr<LocalMediaSourceHandler> m_localMediaSourceHandler;
};

//
// JLocalPlayControlType
//

class JLocalPlayControlTypeConfig : public EnumConfiguration<LocalMediaSourceHandler::PlayControlType> {
public:
    using T = LocalMediaSourceHandler::PlayControlType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$PlayControlType";
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

using JLocalPlayControlType = JEnum<LocalMediaSourceHandler::PlayControlType, JLocalPlayControlTypeConfig>;

//
// JSource
//

class JSourceConfig : public EnumConfiguration<LocalMediaSourceHandler::Source> {
public:
    using T = LocalMediaSourceHandler::Source;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$Source";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::BLUETOOTH, "BLUETOOTH"},
                {T::USB, "USB"},
                {T::FM_RADIO, "FM_RADIO"},
                {T::AM_RADIO, "AM_RADIO"},
                {T::SATELLITE_RADIO, "SATELLITE_RADIO"},
                {T::LINE_IN, "LINE_IN"},
                {T::COMPACT_DISC, "COMPACT_DISC"},
                {T::SIRIUS_XM, "SIRIUS_XM"},
                {T::DAB, "DAB"}};
    }
};

using JSource = JEnum<LocalMediaSourceHandler::Source, JSourceConfig>;

//
// JContentSelector
//

class JContentSelectorConfig : public EnumConfiguration<LocalMediaSourceHandler::ContentSelector> {
public:
    using T = LocalMediaSourceHandler::ContentSelector;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$ContentSelector";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::FREQUENCY, "FREQUENCY"}, {T::CHANNEL, "CHANNEL"}, {T::PRESET, "PRESET"}};
    }
};

using JContentSelector = JEnum<LocalMediaSourceHandler::ContentSelector, JContentSelectorConfig>;

//
// JLocalSupportedPlaybackOperation
//

class JLocalSupportedPlaybackOperationConfig
        : public EnumConfiguration<ExternalMediaAdapterHandler::ExternalMediaAdapter::SupportedPlaybackOperation> {
public:
    using T = ExternalMediaAdapterHandler::SupportedPlaybackOperation;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$SupportedPlaybackOperation";
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

using JLocalSupportedPlaybackOperation =
    JEnum<ExternalMediaAdapterHandler::SupportedPlaybackOperation, JLocalSupportedPlaybackOperationConfig>;

//
// JLocalFavorites
//

class JLocalFavoritesConfig : public EnumConfiguration<ExternalMediaAdapterHandler::Favorites> {
public:
    using T = ExternalMediaAdapterHandler::Favorites;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$Favorites";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::FAVORITED, "FAVORITED"}, {T::UNFAVORITED, "UNFAVORITED"}, {T::NOT_RATED, "NOT_RATED"}};
    }
};

using JLocalFavorites = JEnum<ExternalMediaAdapterHandler::Favorites, JLocalFavoritesConfig>;

//
// JLocalMediaType
//

class JLocalMediaTypeConfig : public EnumConfiguration<ExternalMediaAdapterHandler::MediaType> {
public:
    using T = ExternalMediaAdapterHandler::MediaType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/LocalMediaSource$MediaType";
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

using JLocalMediaType = JEnum<ExternalMediaAdapterHandler::MediaType, JLocalMediaTypeConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_LOCAL_MEDIA_SOURCE_BINDER_H
