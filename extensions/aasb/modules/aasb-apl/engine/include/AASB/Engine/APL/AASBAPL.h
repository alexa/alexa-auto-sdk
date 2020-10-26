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

#ifndef AASB_ENGINE_APL_AASB_APL_H
#define AASB_ENGINE_APL_AASB_APL_H

#include <AACE/APL/APL.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace apl {

class AASBAPL
        : public aace::apl::APL
        , public std::enable_shared_from_this<AASBAPL> {
private:
    AASBAPL() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBAPL> create(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::apl
    std::string getVisualContext() override;
    void renderDocument(const std::string& payload, const std::string& token, const std::string& windowId) override;
    void clearDocument() override;
    void executeCommands(const std::string& payload, const std::string& token) override;
    void interruptCommandSequence() override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace apl
}  // namespace engine
}  // namespace aasb

#endif
