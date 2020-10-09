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

#include <typeinfo>

#include "AACE/Engine/Navigation/NavigationEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace navigation {

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.NavigationEngineService");

// register the service
REGISTER_SERVICE(NavigationEngineService);

NavigationEngineService::NavigationEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool NavigationEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        rapidjson::IStreamWrapper isw(*configuration);
        rapidjson::Document document;

        document.ParseStream(isw);

        ThrowIf(document.HasParseError(), GetParseError_En(document.GetParseError()));
        ThrowIfNot(document.IsObject(), "invalidConfigurationStream");

        auto root = document.GetObject();

        if (root.HasMember("providerName") && root["providerName"].IsString()) {
            m_navigationProviderName = root["providerName"].GetString();
            AACE_DEBUG(LX(TAG, "configure").d("providerName", m_navigationProviderName));
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

bool NavigationEngineService::shutdown() {
    if (m_navigationEngineImpl != nullptr) {
        m_navigationEngineImpl->shutdown();
    }

    return true;
}

bool NavigationEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::navigation::Navigation>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool NavigationEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::navigation::Navigation> navigation) {
    try {
        ThrowIfNotNull(m_navigationEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the navigation engine implementation
        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto defaultEndpointBuilder = alexaComponents->getDefaultEndpointBuilder();
        ThrowIfNull(defaultEndpointBuilder, "defaultEndpointBuilderInvalid");

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull(exceptionSender, "exceptionSenderInvalid");

        auto messageSender = alexaComponents->getMessageSender();
        ThrowIfNull(messageSender, "messageSenderInvalid");

        auto contextManager = alexaComponents->getContextManager();
        ThrowIfNull(contextManager, "contextManagerInvalid");

        m_navigationEngineImpl = aace::engine::navigation::NavigationEngineImpl::create(
            navigation,
            defaultEndpointBuilder,
            exceptionSender,
            messageSender,
            contextManager,
            m_navigationProviderName);
        ThrowIfNull(m_navigationEngineImpl, "createNavigationEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Navigation>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace navigation
}  // namespace engine
}  // namespace aace
