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
#ifndef AASB_ALEXA_LISTRENDERER_HANDLER_H
#define AASB_ALEXA_LISTRENDERER_HANDLER_H

#include <memory>

#include <AACE/Gloria/ListRenderer.h>
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace gloria {

class ListRendererHandler : public aace::gloria::ListRenderer {
public:
    static std::shared_ptr<ListRendererHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    /// @name aace::gloria::ListRenderer
    /// @{
    void handleDirective(
        const std::string& ns,
        const std::string& name,
        const std::string& messageId,
        const std::string& dialogRequestId,
        const std::string& payload) override;
    /// @}

    /**
     * Process incoming events from AASB client meant for topic @c TOPIC_PHONECALL_CONTROLLER
     *
     * @param action Type of event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    ListRendererHandler(std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    /**
     * Notifies the Engine of the event that occurred in app
     *
     * @param payload Payload containing data.
     */
    void sendEvent(const std::string& payload);

    /**
     * Notifies the Engine that state of the list rendered earlier has changed.
     *
     * @param payload Payload containing data.
     */
    void renderedListStateChanged(const std::string& payload);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_LISTRENDERER_HANDLER_H