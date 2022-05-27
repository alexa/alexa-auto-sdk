
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

#ifndef AACE_ENGINE_FEATURE_DISCOVERY_REST_AGENT_H
#define AACE_ENGINE_FEATURE_DISCOVERY_REST_AGENT_H

#include <queue>

#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpGet.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpDelete.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpResponseCodes.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPResponse.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AACE/Engine/Alexa/AlexaEndpointInterface.h>
#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include "AACE/Engine/Utils/JSON/JSON.h"

namespace aace {
namespace engine {
namespace alexa {

class FeatureDiscoveryRESTAgent {
private:
    FeatureDiscoveryRESTAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate);

    bool initialize(std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

public:
    struct LocalizedFeature {
        std::string utteranceText;
        std::string descriptionText;
    };

    using HTTPResponse = alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse;

    static std::shared_ptr<FeatureDiscoveryRESTAgent> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

    virtual ~FeatureDiscoveryRESTAgent() = default;

    std::string createUrlFromParameters(
        const std::string& domain,
        const std::string& eventType,
        int limit,
        const std::string& tag = "");
    HTTPResponse getHTTPResponseFromCloud(const std::string& url, const std::string& locale);
    std::vector<LocalizedFeature> getFeaturesFromHTTPResponse(const HTTPResponse& response, const std::string& locale);
    static std::string convertLocaleFormat(const std::string& locale);

private:
    std::vector<std::string> buildCommonHTTPHeader(const std::string& locale);
    bool parseCommonHTTPResponse(const HTTPResponse& response);
    std::string getHTTPErrorString(const HTTPResponse& response);

    HTTPResponse doGet(const std::string& url, const std::vector<std::string>& headers);
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    std::string m_featureDiscoveryEndpoint;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_FEATURE_DISCOVERY_REST_AGENT_H
