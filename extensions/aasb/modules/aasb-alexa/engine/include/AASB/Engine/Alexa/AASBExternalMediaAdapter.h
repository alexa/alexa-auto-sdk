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

#ifndef AASB_EXTERNAL_MEDIA_ADAPTER_H
#define AASB_EXTERNAL_MEDIA_ADAPTER_H

#include <AACE/Alexa/ExternalMediaAdapter.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace alexa {

class AASBExternalMediaAdapter
        : public aace::alexa::ExternalMediaAdapter
        , public std::enable_shared_from_this<AASBExternalMediaAdapter> {
private:
    AASBExternalMediaAdapter() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBExternalMediaAdapter> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

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
    bool playControl(const std::string& localPlayerId, PlayControlType controlType) override;
    bool seek(const std::string& localPlayerId, std::chrono::milliseconds offset) override;
    bool adjustSeek(const std::string& localPlayerId, std::chrono::milliseconds deltaOffset) override;
    bool authorize(const std::vector<AuthorizedPlayerInfo>& authorizedPlayers) override;
    bool getState(const std::string& localPlayerId, ExternalMediaAdapterState& state) override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

#endif  //AASB_EXTERNAL_MEDIA_ADAPTER_H
