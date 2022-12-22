/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_WAKEWORD_AASB_WAKEWORD_H
#define AASB_ENGINE_WAKEWORD_AASB_WAKEWORD_H

#include <AACE/Wakeword/WakewordManager.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace wakeword {

class AASBWakeword
        : public aace::wakeword::WakewordManager
        , public std::enable_shared_from_this<AASBWakeword> {
public:
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;

private:
    AASBWakeword() = default;

    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBWakeword> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    void onWakewordDetected(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex) override;

private:
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
};

}  // namespace wakeword
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_WAKEWORD_AASB_WAKEWORD_H
