/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include <AACE/Engine/Alexa/FeatureDiscoveryRESTAgent.h>
#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.featureDiscovery.FeatureDiscoveryRESTAgent");

/// The HTTP header to pass the Amazon access token
static const std::string AUTHORIZATION_HEADER = "Authorization";
/// The HTTP header to pass the accept language
static const std::string ACCEPT_LANGUAGE_HEADER = "Accept-Language";
/// The prefix of the Amazon access token
static const std::string TOKEN_PREFIX = "Bearer";
/// Colon Separator for HTTP
static const std::string COLON_SEPARATOR = ":";
/// Space Separator for HTTP
static const std::string SPACE_SEPARATOR = " ";
/// Default value for the HTTP request timeout.
static const std::chrono::seconds DEFAULT_HTTP_TIMEOUT = std::chrono::seconds(60);
/// Default value for the HTTP retry on Network Error.
static const int HTTP_RETRY_COUNT = 3;
/// REST URL to discovery features
static const std::string DEFAULT_AHS_ENDPOINT = "https://api.amazonalexa.com";
/// Question mark used in URL
static const std::string QUESTION_MARK_SEPARATOR = "?";
/// And sign used in URL
static const std::string AND_SIGN = "&";
/// Equal sign
static const std::string EQUAL_SIGN = "=";
/// Path suffix for GetHints requests
static const std::string HINTS_PATH = "v1/hints";
/// Forward slash separator used in URL
static const std::string FORWARD_SLASH = "/";

/// Forward slash separator used in URL
static const std::string PARAM_DOMAIN = "domain";
static const std::string PARAM_EVENT_TYPE = "eventType";
static const std::string PARAM_LIMIT = "limit";
static const std::string PARAM_TAG = "hintTags";

static const std::string PAYLOAD_EMBEDDED = "_embedded";
static const std::string PAYLOAD_HINTS = "hints";
static const std::string PAYLOAD_LOCALIZED_CONTENT = "localizedContent";
static const std::string PAYLOAD_UTTERANCE = "utteranceText";
static const std::string PAYLOAD_DESCRIPTION = "descriptionText";

using LocalizedFeature = FeatureDiscoveryRESTAgent::LocalizedFeature;
using json = nlohmann::json;

FeatureDiscoveryRESTAgent::FeatureDiscoveryRESTAgent(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate) :
        m_authDelegate(authDelegate), m_featureDiscoveryEndpoint(DEFAULT_AHS_ENDPOINT) {
}

std::shared_ptr<FeatureDiscoveryRESTAgent> FeatureDiscoveryRESTAgent::create(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        std::shared_ptr<FeatureDiscoveryRESTAgent> featureDiscoveryRESTAgent =
            std::shared_ptr<FeatureDiscoveryRESTAgent>(new FeatureDiscoveryRESTAgent(authDelegate));
        ThrowIfNot(featureDiscoveryRESTAgent->initialize(alexaEndpoints), "initializeFeatureDiscoveryRESTAgentFailed");

        return featureDiscoveryRESTAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool FeatureDiscoveryRESTAgent::initialize(
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    auto featureDiscoveryEndpoint = alexaEndpoints->getFeatureDiscoveryEndpoint();
    if (!featureDiscoveryEndpoint.empty()) {
        m_featureDiscoveryEndpoint = featureDiscoveryEndpoint;
        AACE_INFO(LX(TAG).m("usingFeatureDiscoveryEndpointFromConfiguration"));
        AACE_DEBUG(LX(TAG).d("featureDiscoveryEndpoint", m_featureDiscoveryEndpoint));
    }

    return true;
}

std::vector<std::string> FeatureDiscoveryRESTAgent::buildCommonHTTPHeader(const std::string& locale) {
    return {AUTHORIZATION_HEADER + COLON_SEPARATOR + SPACE_SEPARATOR + TOKEN_PREFIX + SPACE_SEPARATOR +
                m_authDelegate->getAuthToken(),
            ACCEPT_LANGUAGE_HEADER + COLON_SEPARATOR + SPACE_SEPARATOR + locale};
}

bool FeatureDiscoveryRESTAgent::parseCommonHTTPResponse(const HTTPResponse& response) {
    return (HTTPResponseCode::SUCCESS_OK == response.code);
}

std::string FeatureDiscoveryRESTAgent::createUrlFromParameters(
    const std::string& domain,
    const std::string& eventType,
    int limit,
    const std::string& tag) {
    std::string url = m_featureDiscoveryEndpoint + FORWARD_SLASH + HINTS_PATH + QUESTION_MARK_SEPARATOR;
    if (domain.empty() || eventType.empty() || limit < 1) {
        AACE_ERROR(LX(TAG).m("parameterIsInvalid"));
        return "";
    }
    std::vector<std::pair<std::string, std::string> > queryParams = {
        {PARAM_DOMAIN, domain},
        {PARAM_EVENT_TYPE, eventType},
        {PARAM_LIMIT, std::to_string(limit)},
    };
    if (!tag.empty()) {
        queryParams.emplace_back(std::make_pair(PARAM_TAG, tag));
    }

    for (auto& param : queryParams) {
        if (url.back() != QUESTION_MARK_SEPARATOR[0]) {
            url.append(AND_SIGN);
        }
        url.append(param.first);
        url.append(EQUAL_SIGN);
        url.append(param.second);
    }
    return url;
}

FeatureDiscoveryRESTAgent::HTTPResponse FeatureDiscoveryRESTAgent::doGet(
    const std::string& url,
    const std::vector<std::string>& headers) {
    try {
        // Creating the HttpGet on every doGet is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
        ThrowIfNull(httpGet, "nullHttpGet");

        return httpGet->doGet(url, headers, DEFAULT_HTTP_TIMEOUT);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return FeatureDiscoveryRESTAgent::HTTPResponse();
    }
}

std::string FeatureDiscoveryRESTAgent::getHTTPErrorString(const HTTPResponse& response) {
    switch (response.code) {
        case HTTPResponseCode::SUCCESS_NO_CONTENT:
            return ".successWithNoContent";
        case HTTPResponseCode::BAD_REQUEST:
            return ".badRequest";
        case HTTPResponseCode::FORBIDDEN:
            return ".authenticationFailed";
        case HTTPResponseCode::SERVER_INTERNAL_ERROR:
            return ".internalServiceError";
        default:
            return ".httpRequestFailed ";
    }
}

std::vector<LocalizedFeature> FeatureDiscoveryRESTAgent::getFeaturesFromHTTPResponse(
    const HTTPResponse& httpResponse,
    const std::string& locale) {
    try {
        std::vector<LocalizedFeature> localizedFeatures;
        ThrowIf(httpResponse.body.empty(), "ResponseBodyIsEmpty");
        const auto& document = json::parse(httpResponse.body);
        const auto& embedded = document.at(PAYLOAD_EMBEDDED);
        for (const auto& hint : embedded.at(PAYLOAD_HINTS)) {
            const auto& localizedContent = hint.at(PAYLOAD_LOCALIZED_CONTENT);
            const auto& hints = localizedContent.at(convertLocaleFormat(locale));
            LocalizedFeature feature;
            feature.utteranceText = hints.at(PAYLOAD_UTTERANCE).get<std::string>();
            if (hints.contains(PAYLOAD_DESCRIPTION)) {
                feature.descriptionText = hints[PAYLOAD_DESCRIPTION].get<std::string>();
            }
            localizedFeatures.push_back(feature);
        }
        return localizedFeatures;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return {};
    }
}

FeatureDiscoveryRESTAgent::HTTPResponse FeatureDiscoveryRESTAgent::getHTTPResponseFromCloud(
    const std::string& url,
    const std::string& locale) {
    FeatureDiscoveryRESTAgent::HTTPResponse httpResponse;
    bool validFlag = false;
    try {
        ThrowIf(locale.empty(), "localeIsEmpty");
        auto httpHeaderData = buildCommonHTTPHeader(locale);
        for (int retry = 0; retry < HTTP_RETRY_COUNT; retry++) {
            httpResponse = doGet(url, httpHeaderData);
            if (parseCommonHTTPResponse(httpResponse)) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot(validFlag, "httpDoGetFailed" + getHTTPErrorString(httpResponse));
        return httpResponse;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return {};
    }
}

std::string FeatureDiscoveryRESTAgent::convertLocaleFormat(const std::string& locale) {
    if (locale.empty() || locale.size() < 5) {
        return "";
    }
    std::string language = locale.substr(0, 2);
    std::string country = locale.substr(3, 2);
    std::transform(language.begin(), language.end(), language.begin(), ::tolower);
    std::transform(country.begin(), country.end(), country.begin(), ::toupper);
    return language + "_" + country;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
