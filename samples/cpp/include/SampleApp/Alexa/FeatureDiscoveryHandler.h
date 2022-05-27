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

#ifndef SAMPLEAPP_ALEXA_FEATUREDISCOVERYHANDLER_H_
#define SAMPLEAPP_ALEXA_FEATUREDISCOVERYHANDLER_H_

#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include "AACE/Core/MessageBroker.h"
#include "AASB/Message/Alexa/FeatureDiscovery/GetFeaturesMessage.h"

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FeatureDiscoveryHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class FeatureDiscoveryHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    FeatureDiscoveryHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<FeatureDiscoveryHandler> {
        return std::shared_ptr<FeatureDiscoveryHandler>(new FeatureDiscoveryHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the GetFeaturesReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetFeaturesReplyMessage(const std::string& message);

    /**
     * Parses the discovery responses received from the engine.
     *
     * @param [in] A string of the escaped JSON array representing the feature discovery responses
     * */
    void parseDiscoveryResponses(const std::string& discoveryResponses);

    /**
     * Notify the Engine (by publishing GetFeaturesMessage) to retrieve the Alexa features from the
     * feature discovery endpoint.
     * The response of the event will be reported through GetFeaturesReplyMessage.
     */
    void getFeatures(const std::string& domain);

private:
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_FEATUREDISCOVERYHANDLER_H_
