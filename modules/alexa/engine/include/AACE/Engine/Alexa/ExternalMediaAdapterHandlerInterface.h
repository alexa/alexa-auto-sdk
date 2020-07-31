/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_INTERFACE_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_INTERFACE_H

#include <chrono>
#include <string>
#include <vector>

#include "ExternalMediaAdapterInterface.h"
#include "ExternalMediaPlayerInterface.h"
#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/Alexa/ExternalMediaAdapter.h"

namespace aace {
namespace engine {
namespace alexa {

static const std::string VALIDATION_SIGNING_CERTIFICATE = "SIGNING_CERTIFICATE";
static const std::string VALIDATION_GENERATED_CERTIFICATE = "GENERATED_CERTIFICATE";
static const std::string VALIDATION_NONE = "NONE";

class PlayerInfo;

class ExternalMediaAdapterHandlerInterface : public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    ExternalMediaAdapterHandlerInterface(const std::string& name);
    virtual ~ExternalMediaAdapterHandlerInterface() = default;

public:
    virtual std::vector<aace::engine::alexa::PlayerInfo> authorizeDiscoveredPlayers(
        const std::vector<aace::engine::alexa::PlayerInfo>& authorizedPlayerList) = 0;
    virtual bool login(
        const std::string& playerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) = 0;
    virtual bool logout(const std::string& playerId) = 0;
    virtual bool play(
        const std::string& playerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        const std::string& skillToken,
        const std::string& playbackSessionId,
        const std::string& navigation,
        bool preload,
        const alexaClientSDK::avsCommon::avs::PlayRequestor& playRequestor) = 0;
    virtual bool playControl(const std::string& playerId, aace::engine::alexa::RequestType requestType) = 0;
    virtual bool seek(const std::string& playerId, std::chrono::milliseconds offset) = 0;
    virtual bool adjustSeek(const std::string& playerId, std::chrono::milliseconds deltaOffset) = 0;
    virtual std::vector<aace::engine::alexa::AdapterState> getAdapterStates(bool all = true) = 0;
    virtual std::chrono::milliseconds getOffset(const std::string& playerId) = 0;
};

inline ExternalMediaAdapterHandlerInterface::ExternalMediaAdapterHandlerInterface(const std::string& name) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(name) {
}

class PlayerInfo {
public:
    PlayerInfo(const std::string& localPlayerId = "", const std::string& spiVersion = "", bool authorized = false);

public:
    std::string localPlayerId;
    std::string spiVersion;
    std::string playerId;
    std::string skillToken;
    std::string playbackSessionId;
    bool authorized;
};

inline PlayerInfo::PlayerInfo(const std::string& localPlayerId, const std::string& spiVersion, bool authorized) :
        localPlayerId(localPlayerId), spiVersion(spiVersion), authorized(authorized) {
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_INTERFACE_H
