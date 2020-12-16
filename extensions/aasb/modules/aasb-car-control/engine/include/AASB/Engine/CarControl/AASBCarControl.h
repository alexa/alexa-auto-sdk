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

#ifndef AASB_ENGINE_CAR_CONTROL_AASB_CAR_CONTROL_H
#define AASB_ENGINE_CAR_CONTROL_AASB_CAR_CONTROL_H

#include <AACE/CarControl/CarControl.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <future>
#include <utility>
#include <unordered_map>
#include <string>

namespace aasb {
namespace engine {
namespace carControl {

class AASBCarControl
        : public aace::carControl::CarControl
        , public std::enable_shared_from_this<AASBCarControl> {
private:
    using CarControlPromise = std::promise<bool>;
    AASBCarControl(uint32_t asyncReplyTimeout);

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<CarControlPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    bool waitForAsyncReply(const std::string& messageId);
    std::shared_ptr<CarControlPromise> getReplyMessagePromise(const std::string& messageId);

public:
    static std::shared_ptr<AASBCarControl> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        uint32_t asyncReplyTimeout);

    // aace::carControl
    bool turnPowerControllerOn(const std::string& endpointId) override;
    bool turnPowerControllerOff(const std::string& endpointId) override;
    bool isPowerControllerOn(const std::string& endpointId, bool& isOn) override;

    bool turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) override;
    bool turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) override;
    bool isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn) override;

    bool setRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double value) override;
    bool adjustRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double delta)
        override;
    bool getRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double& value)
        override;

    bool setModeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        const std::string& value) override;
    bool adjustModeControllerValue(const std::string& endpointId, const std::string& controllerId, int delta) override;
    bool getModeControllerValue(const std::string& endpointId, const std::string& controllerId, std::string& value)
        override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
    uint32_t m_replyMessageTimeout;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<CarControlPromise>> m_promiseMap;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aasb

#endif
