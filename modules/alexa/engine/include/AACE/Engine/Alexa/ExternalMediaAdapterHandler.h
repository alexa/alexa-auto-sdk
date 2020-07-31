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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ExternalMediaAdapterInterface.h"
#include "ExternalMediaPlayerInterface.h"
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/ExternalMediaAdapter.h"

#include "DiscoveredPlayerSenderInterface.h"
#include "ExternalMediaAdapterHandlerInterface.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace alexa {

class FocusHandlerInterface;

class ExternalMediaAdapterHandler
        : public alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface
        , public ExternalMediaAdapterHandlerInterface
        , public std::enable_shared_from_this<ExternalMediaAdapterHandler> {
protected:
    ExternalMediaAdapterHandler(
        std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
        std::shared_ptr<FocusHandlerInterface> focusHandler);

    bool initializeAdapterHandler(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager);

    bool validatePlayer(const std::string& localPlayerId, bool checkAuthorized = true);
    bool setFocus(const std::string& localPlayerId, bool focusAcquire);

    std::string createExternalMediaPlayerEvent(
        const std::string& localPlayerId,
        const std::string& event,
        bool includePlaybackSessionId = false,
        std::function<void(rapidjson::Value::Object&, rapidjson::Value::AllocatorType&)> createPayload =
            [](rapidjson::Value::Object& v, rapidjson::Value::AllocatorType& a) {});

    void reportDiscoveredPlayers(
        const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers);
    bool removeDiscoveredPlayer(const std::string& localPlayerId);

    // ExternalMediaAdapterHandler interface
    virtual bool handleAuthorization(
        const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList) = 0;
    virtual bool handleLogin(
        const std::string& playerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) = 0;
    virtual bool handleLogout(const std::string& playerId) = 0;
    virtual bool handlePlay(
        const std::string& playerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        bool preload,
        aace::alexa::ExternalMediaAdapter::Navigation navigation,
        const std::string& playbackSessionId,
        const std::string& skillToken) = 0;
    virtual bool handlePlayControl(
        const std::string& playerId,
        aace::alexa::ExternalMediaAdapter::PlayControlType playControlType) = 0;
    virtual bool handleSeek(const std::string& playerId, std::chrono::milliseconds offset) = 0;
    virtual bool handleAdjustSeek(const std::string& playerId, std::chrono::milliseconds deltaOffset) = 0;
    virtual bool handleGetAdapterState(const std::string& playerId, aace::engine::alexa::AdapterState& state) = 0;
    virtual std::chrono::milliseconds handleGetOffset(const std::string& playerId) = 0;

    virtual bool handleSetVolume(int8_t volume) = 0;
    virtual bool handleSetMute(bool mute) = 0;

    // alexaClientSDK::avsCommon::utils::RequiresShutdown
    virtual void doShutdown() override;

public:
    std::vector<PlayerInfo> authorizeDiscoveredPlayers(const std::vector<PlayerInfo>& authorizedPlayerList) override;
    bool login(
        const std::string& playerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) override;
    bool logout(const std::string& playerId) override;
    bool play(
        const std::string& playerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        const std::string& skillToken,
        const std::string& playbackSessionId,
        const std::string& navigation,
        bool preload,
        const alexaClientSDK::avsCommon::avs::PlayRequestor& playRequestor) override;
    bool playControl(const std::string& playerId, aace::engine::alexa::RequestType requestType) override;
    bool seek(const std::string& playerId, std::chrono::milliseconds offset) override;
    bool adjustSeek(const std::string& playerId, std::chrono::milliseconds deltaOffset) override;
    std::vector<AdapterState> getAdapterStates(bool all) override;
    std::chrono::milliseconds getOffset(const std::string& playerId) override;

    //
    // alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface
    //
    bool startDucking() override;
    bool stopDucking() override;
    bool setUnduckedVolume(int8_t volume) override;
    bool setMute(bool mute) override;
    bool getSpeakerSettings(
        alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) const override;
    alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type getSpeakerType() const override;

private:
    std::weak_ptr<DiscoveredPlayerSenderInterface> m_discoveredPlayerSender;
    std::weak_ptr<FocusHandlerInterface> m_focusHandler;

    std::unordered_map<std::string, PlayerInfo> m_playerInfoMap;
    std::unordered_map<std::string, std::string> m_alexaToLocalPlayerIdMap;

    bool m_muted;
    int8_t m_volume;

    /**
     * Serializes generic access. Used for delaying focus state change.
     */
    std::mutex m_mutex;
    /**
     * Generic executor. Used for delaying focus state change.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
    /**
     * Serializes generic condition. Used for delaying focus state change.
     */
    std::condition_variable m_attemptedSetFocusPlayerInFocusCondition;
};

class FocusHandlerInterface {
public:
    virtual void setFocus(const std::string& playerId, bool focusAcquire) = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H
