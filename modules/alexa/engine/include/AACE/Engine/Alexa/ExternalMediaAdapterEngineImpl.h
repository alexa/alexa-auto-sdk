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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_ENGINE_IMPL_H

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ExternalMediaAdapterInterface.h"
#include "ExternalMediaPlayerInterface.h"
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/ExternalMediaAdapter.h"

#include "ExternalMediaAdapterHandler.h"

namespace aace {
namespace engine {
namespace alexa {

class ExternalMediaAdapterEngineImpl
        : public ExternalMediaAdapterHandler
        , public aace::alexa::ExternalMediaAdapterEngineInterface {
private:
    ExternalMediaAdapterEngineImpl(
        std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter,
        std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
        std::shared_ptr<FocusHandlerInterface> focusHandler);

    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager);

public:
    static std::shared_ptr<ExternalMediaAdapterEngineImpl> create(
        std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter,
        std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
        std::shared_ptr<FocusHandlerInterface> focusHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager);

    // aace::alexa::ExternalMediaAdapterEngineInterface
    void onReportDiscoveredPlayers(const std::vector<DiscoveredPlayerInfo>& discoveredPlayers) override;
    void onRequestToken(const std::string& localPlayerId) override;
    void onLoginComplete(const std::string& localPlayerId) override;
    void onLogoutComplete(const std::string& localPlayerId) override;
    void onPlayerEvent(const std::string& localPlayerId, const std::string& eventName) override;
    void onPlayerError(
        const std::string& localPlayerId,
        const std::string& errorName,
        long code,
        const std::string& description,
        bool fatal) override;
    void onSetFocus(const std::string& localPlayerId) override;
    void onRemoveDiscoveredPlayer(const std::string& localPlayerId) override;

protected:
    // ExternalMediaAdapterHandler
    bool handleAuthorization(
        const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList) override;
    bool handleLogin(
        const std::string& localPlayerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) override;
    bool handleLogout(const std::string& localPlayerId) override;
    bool handlePlay(
        const std::string& localPlayerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        bool preload,
        aace::alexa::ExternalMediaAdapter::Navigation navigation,
        const std::string& playbackSessionId,
        const std::string& skillToken) override;
    bool handlePlayControl(
        const std::string& localPlayerId,
        aace::alexa::ExternalMediaAdapter::PlayControlType playControlType) override;
    bool handleSeek(const std::string& localPlayerId, std::chrono::milliseconds offset) override;
    bool handleAdjustSeek(const std::string& localPlayerId, std::chrono::milliseconds deltaOffset) override;
    bool handleGetAdapterState(const std::string& localPlayerId, AdapterState& state) override;
    std::chrono::milliseconds handleGetOffset(const std::string& playerId) override;

    bool handleSetVolume(int8_t volume) override;
    bool handleSetMute(bool mute) override;

    void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> m_platformMediaAdapter;
    std::weak_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_ENGINE_IMPL_H
