/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "AACE/Engine/PhoneCallController/PhoneCallControllerRESTAgent.h"

#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpGet.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpResponseCodes.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPResponse.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace phoneCallController {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.phoneCallController.PhoneCallControllerRESTAgent");

/// The HTTP authorization header to pass the LWA Auth token
static const std::string AUTHORIZATION_HTTP_HEADER = "Authorization";

/// The HTTP authorization header prefix to pass the LWA Auth token
static const std::string AUTHORIZATION_HTTP_HEADER_POSTFIX = "Bearer";

/// The HTTP header to pass the Amazon RequestId
static const std::string X_AMZN_REQUESTID = "X-Amzn-RequestId";

// The HTTP header to pass the Amazon ClientId
static const std::string X_AZN_CLIENTID = "X-Amzn-ClientId";

/// The HTTP header to pass the User Agent
static const std::string USER_AGENT = "User-Agent";

/// The HTTP header to pass the the Preferred Market Place
static const std::string ACCEPT_PFM = "Accept-PFM";

/// Colon Separator for HTTP
static const std::string COLON_SEPARATOR = ":";

/// Space Separator for HTTP
static const std::string SPACE_SEPARATOR = " ";

/// The HTTP header to pass the Content-Type as application/json
static const std::string CONTENT_TYPE_APPLICATION_JSON = "content-type: application/json";

/// Default value for ACCEPT_PFM HTTP header
const std::string DEFAULT_PFM = "US";

/// Default value for User Agent HTTP header
const std::string DEFAULT_USER_AGENT_VALUE = "AutoSDK/PhoneCallController/1.0";

/// Default value for the HTTP request timeout.
static const std::chrono::seconds DEFAULT_HTTP_TIMEOUT = std::chrono::seconds(60);

/// ACMS Endpoint
static const std::string DEFAULT_ACMS_ENDPOINT = "https://alexa-comms-mobile-service-na.amazon.com";

/// Path suffix for ACMS URL accounts
static const std::string ACCOUNTS_PATH = "accounts";

/// Path suffix for ACMS URL users
static const std::string USERS_PATH = "users";

/// Forward slash separator used in URL
static const std::string FORWARD_SLASH = "/";

/**
 * Helper function to build the common HTTP header used for network calls.
 *
 * @param deviceSerialNumber The serial number of the device required in the HTTP header.
 * @param authToken The LWA token required in the HTTP header.
 * @return Returns a vector of string with the HTTP header.
 */
static std::vector<std::string> buildCommonHTTPHeader(
    const std::string& deviceSerialNumber,
    const std::string& authToken) {
    std::vector<std::string> httpHeaderData;
    std::string requestUUID = alexaClientSDK::avsCommon::utils::uuidGeneration::generateUUID();
    httpHeaderData = {AUTHORIZATION_HTTP_HEADER + COLON_SEPARATOR + SPACE_SEPARATOR +
                          AUTHORIZATION_HTTP_HEADER_POSTFIX + SPACE_SEPARATOR + authToken,
                      ACCEPT_PFM + COLON_SEPARATOR + SPACE_SEPARATOR + DEFAULT_PFM,
                      X_AMZN_REQUESTID + COLON_SEPARATOR + SPACE_SEPARATOR + requestUUID,
                      X_AZN_CLIENTID + COLON_SEPARATOR + SPACE_SEPARATOR + deviceSerialNumber,
                      USER_AGENT + COLON_SEPARATOR + SPACE_SEPARATOR + DEFAULT_USER_AGENT_VALUE};

    return httpHeaderData;
}

/**
 * Helper function to verify if HTTP response received.
 *
 * @param response The @c HTTPResponse containing the HTTP response of network call.
 * @return Returns @c true if the response is success, otherwise @c false.
 */
static bool parseCommonHTTPResponse(const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response) {
    if (HTTPResponseCode::SUCCESS_OK == response.code) {
        return true;
    }
    return false;
}

/**
 * Helper function to do HTTP POST operation.
 *
 * @param url The endpoint url.
 * @param headerLines The HTTP header passed as part of HTTP POST.
 * @param data The HTTP body passed as part of HTTP POST.
 * @param timeout The timeout for HTTP POST operation.
 * @return On success returns @c HTTPResponse received from server, otherwise empty object of @c HTTPResponse.
 */
static alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse doPost(
    const std::string& url,
    const std::vector<std::string> headerLines,
    const std::string& data,
    std::chrono::seconds timeout) {
    try {
        // Creating the HttpPost on every doPost is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpPost = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost::create();
        ThrowIfNull(httpPost, "nullHttpPost");

        return httpPost->doPost(url, headerLines, data, timeout);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "doPost").d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse();
    }
}

/**
 * Helper function to do HTTP GET operation.
 *
 * @param url The endpoint url.
 * @param headerLines The HTTP header passed as part of HTTP GET.
 * @return On success returns @c HTTPResponse received from server, otherwise empty object of @c HTTPResponse.
 */
static alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse doGet(
    const std::string& url,
    const std::vector<std::string>& headers) {
    try {
        // Creating the HttpGet on every doGet is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
        ThrowIfNull(httpGet, "nullHttpGet");

        return httpGet->doGet(url, headers, DEFAULT_HTTP_TIMEOUT);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "doGet").d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse();
    }
}

/**
 * Helper function to convert HTTP response error to strings used for logging.
 *
 * @param response The @c HTTPResponse.
 * @return Returns the string representation of the HTTP error codes.
 */
static std::string convertHTTPErrorStringForLogging(
    const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response) {
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

/**
 * Helper function to build the HTTP body of auto provisioning.
 *
 * @return Returns the string json.
 */
static std::string buildAutoAccountProvisionJson() {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember("autoProvision", rapidjson::Value().SetBool(true), document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return (std::string(buffer.GetString()));
}

/**
 * Gets the ACMS endpoint.
 * 
 * @param alexaEndpoints Reference to @c AlexaEndpointInterface, used to get the ACMS endpoint provided in engine configuration.
 * @return Returns the ACMS endpoint if provideed in the engine configuration otherwise default value. 
 */
static std::string getACMSEndpoint(std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    auto acmsEndpoint = alexaEndpoints->getACMSEndpoint();
    if (!acmsEndpoint.empty()) {
        AACE_INFO(LX(TAG).m("usingACMSEndpointFromConfiguration"));
        AACE_DEBUG(LX(TAG).d("acmsEndpoint", acmsEndpoint));
        return acmsEndpoint;
    }
    return DEFAULT_ACMS_ENDPOINT;
}

AlexaAccountInfo getAlexaAccountInfo(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    rapidjson::Document document;
    AlexaAccountInfo alexaAccount;

    auto httpHeaderData = buildCommonHTTPHeader(deviceInfo->getDeviceSerialNumber(), authDelegate->getAuthToken());
    try {
        auto httpResponse = doGet(getACMSEndpoint(alexaEndpoints) + FORWARD_SLASH + ACCOUNTS_PATH, httpHeaderData);

        ThrowIfNot(
            parseCommonHTTPResponse(httpResponse),
            "parseCommonHTTPResponseFailed" + convertHTTPErrorStringForLogging(httpResponse));

        if (document.Parse(httpResponse.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }

        if (!document.IsArray()) {
            Throw("jsonResponseNotValidArray");
        }

        for (rapidjson::Value::ConstValueIterator itr = document.Begin(); itr != document.End(); itr++) {
            if ((*itr)["signedInUser"].IsBool() && (*itr)["signedInUser"].GetBool()) {
                if ((*itr).HasMember("directedId")) {
                    if ((*itr)["directedId"].IsString()) {
                        alexaAccount.directedId = (*itr)["directedId"].GetString();
                    }
                } else {
                    Throw("directedIdNotPresent");
                }

                if ((*itr).HasMember("commsProvisionStatus")) {
                    if ((*itr)["commsProvisionStatus"].IsString()) {
                        auto tempCommsProvisionStatus = (*itr)["commsProvisionStatus"].GetString();
                        if (strcmp("UNKNOWN", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = AlexaAccountInfo::AccountProvisionStatus::UNKNOWN;
                        } else if (strcmp("PROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = AlexaAccountInfo::AccountProvisionStatus::PROVISIONED;
                        } else if (strcmp("DEPROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = AlexaAccountInfo::AccountProvisionStatus::DEPROVISIONED;
                        } else if (strcmp("AUTO_PROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = AlexaAccountInfo::AccountProvisionStatus::AUTO_PROVISIONED;
                        }
                    }
                } else {
                    Throw("commsProvisionStatusNotPresent");
                }

                break;  // Found the signed in user, so break.
            }
        }
        return alexaAccount;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "getAlexaAccountInfo").d("reason", ex.what()));
        return alexaAccount;
    }
}

bool doAccountAutoProvision(
    const AlexaAccountInfo& alexaAccountInfo,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    auto httpHeaderData = buildCommonHTTPHeader(deviceInfo->getDeviceSerialNumber(), authDelegate->getAuthToken());
    httpHeaderData.insert(httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON);

    auto autoProvisionJson = buildAutoAccountProvisionJson();
    try {
        auto httpResponse = doPost(
            getACMSEndpoint(alexaEndpoints) + FORWARD_SLASH + ACCOUNTS_PATH + FORWARD_SLASH +
                alexaAccountInfo.directedId + FORWARD_SLASH + USERS_PATH,
            httpHeaderData,
            autoProvisionJson,
            DEFAULT_HTTP_TIMEOUT);

        ThrowIfNot(
            parseCommonHTTPResponse(httpResponse), "httpDoPostFailed" + convertHTTPErrorStringForLogging(httpResponse));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "doAccountAutoProvision").d("reason", ex.what()));
        return false;
    }
}

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aace
