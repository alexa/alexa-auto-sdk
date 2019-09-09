/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_ALEXA_CARDRENDERER_HANDLER_H
#define AASB_ALEXA_CARDRENDERER_HANDLER_H

#include <memory>

#include <AACE/Gloria/CardRenderer.h>
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace gloria {

class CardRendererHandler : public aace::gloria::CardRenderer {
public:
    static std::shared_ptr<CardRendererHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    /// @name aace::gloria::CardRenderer
    /// @{
    void renderCard(const std::string& payload) override;
    void renderPlayerInfo(const std::string& payload) override;
    void readCard(const std::string& payload) override;
    /// @}

private:
    CardRendererHandler(std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_CARDRENDERER_HANDLER_H