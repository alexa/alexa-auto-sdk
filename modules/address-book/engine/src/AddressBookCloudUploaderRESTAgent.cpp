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

#include <AVSCommon/Utils/HTTP/HttpResponseCode.h>
#include <AVSCommon/Utils/RetryTimer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Core/EngineVersion.h>
#include <AACE/Engine/AddressBook/AddressBookCloudUploaderRESTAgent.h>

#include "zlib.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace aace {
namespace engine {
namespace addressBook {

using namespace alexaClientSDK::avsCommon::utils::http;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBookCloudService.AddressBookCloudUploaderRESTAgent");

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

/// The HTTP header to pass the the Preferred Market Place(PFM)
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
const std::string DEFAULT_USER_AGENT_VALUE = "AutoSDK/AddressBook/1.0";

/// Address Book Type to be used for Contacts as defined @c ACMS
const std::string AUTO_SDK_CONTACT_ADDRESS_BOOK_TYPE = "automotive";

/// Address Book Type to be used for Navigation Address as defined @c ACMS
const std::string AUTO_SDK_NAVIGATION_ADDRESS_ADDRESS_BOOK_TYPE = "automotivePostalAddress";

/// Default value for the Address Book Name @c ACMS
const std::string AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME = "AutoSDK";

/// Default value for the HTTP request timeout.
static const std::chrono::seconds DEFAULT_HTTP_TIMEOUT = std::chrono::seconds(60);

/// Default value for the HTTP retry on Network Error.
static const int HTTP_RETRY_COUNT = 2;

/// REST URL to upload the address book
static const std::string DEFAULT_ACMS_ENDPOINT = "https://alexa-comms-mobile-service-na.amazon.com";

/// Path suffix for ACMS URL accounts
static const std::string ACCOUNTS_PATH = "accounts";

/// Path suffix for ACMS URL users
static const std::string USERS_PATH = "users";

/// Path suffix for ACMS URL addressbooks
static const std::string ADDRESSBOOK_PATH = "addressbooks";

/// Path suffix for ACMS URL entries
static const std::string ENTRIES_PATH = "entries";

/// Forward slash separator used in URL
static const std::string FORWARD_SLASH = "/";

/// Path suffix for URL used in ACMS Get IndentityV2 Request
static const std::string GET_INDENTITY_V2_QUERY = "/identities?includeUserName=false";

/// Path suffix for URL used in ACMS Get Address Book Ids
static const std::string GET_ADDRESS_BOOK_QUERY = "?addressBookSourceIds=";

/// Valid hours for uploaded address book
static const int ADDRESS_BOOK_VALID_HOURS = 30 * 24;

/// HTTP Gateway Timeout
static const int HTTP_ERROR_GATEWAY_TIMEOUT = 504;

/**
 * Function to convert the number of times we have already retried to the time to perform the next retry.
 *
 * @param retryCount The number of times we have retried
 * @return The time that the next retry should be attempted
 */
static std::chrono::steady_clock::time_point calculateTimeToRetry(int retryCount) {
    /**
     * Table of retry backoff values
     */
    const static std::vector<int> retryBackoffTimes = {
        1000,  // Retry 1:  1.00s range with 50% randomization: [ 0.5s,  1.5s]
        3000,  // Retry 2:  3.00s range with 50% randomization: [ 1.5s,  4.5s]
    };

    // Retry Timer Object.
    alexaClientSDK::avsCommon::utils::RetryTimer RETRY_TIMER(retryBackoffTimes);

    return std::chrono::steady_clock::now() + RETRY_TIMER.calculateTimeToRetry(retryCount);
}

enum class HTTPResponseResult {
    // Request Successful
    SUCCESS,

    // Retry Request
    RETRY,

    // Request Failed
    FAILED
};

std::ostream& operator<<(std::ostream& os, const HTTPResponseResult& result) {
    switch (result) {
        case HTTPResponseResult::SUCCESS:
            return os << "SUCCESS";
        case HTTPResponseResult::RETRY:
            return os << "RETRY";
        case HTTPResponseResult::FAILED:
            return os << "FAILED";
    }
    return os << "UnknownResult: " << static_cast<int>(result);
}

/**
 * Funtion to parse the HTTP response.
 *
 * @param code The HTTP response code.
 * @return Returns SUCCESS when HTTP resonse is 200 which the expected values for the API used here.
 * And, RETRY for the retriable HTTP code, and FAILED for the permanent failures. 
 */
static HTTPResponseResult parseHTTPResponseCode(long code) {
    HTTPResponseResult status = HTTPResponseResult::FAILED;
    switch (code) {
        case HTTPResponseCode::SUCCESS_OK:
            status = HTTPResponseResult::SUCCESS;
            break;
        case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
        case HTTPResponseCode::CLIENT_ERROR_FORBIDDEN:
        case HTTPResponseCode::CLIENT_ERROR_THROTTLING_EXCEPTION:
        case HTTPResponseCode::SERVER_ERROR_INTERNAL:
        case HTTPResponseCode::SERVER_UNAVAILABLE:
        case HTTPResponseCode::SERVER_ERROR_NOT_IMPLEMENTED:
        case HTTP_ERROR_GATEWAY_TIMEOUT:
            status = HTTPResponseResult::RETRY;
            break;
        case HTTPResponseCode::SUCCESS_CREATED:
        case HTTPResponseCode::SUCCESS_ACCEPTED:
        case HTTPResponseCode::SUCCESS_NO_CONTENT:
        case HTTPResponseCode::SUCCESS_PARTIAL_CONTENT:
            // For the API's that Auto SDK uses these are not the expected response, so treating
            // them as FAILED. Change this logic if any new API in the future return these codes.
        default:
            status = HTTPResponseResult::FAILED;
            break;
    }
    AACE_DEBUG(LX(TAG).d("code", code).d("status", status));
    return status;
}

AddressBookCloudUploaderRESTAgent::AddressBookCloudUploaderRESTAgent(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_isShuttingDown(false),
        m_authDelegate(authDelegate),
        m_deviceInfo(deviceInfo),
        m_acmsEndpoint(DEFAULT_ACMS_ENDPOINT) {
}

std::shared_ptr<AddressBookCloudUploaderRESTAgent> AddressBookCloudUploaderRESTAgent::create(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        std::shared_ptr<AddressBookCloudUploaderRESTAgent> addressBookCloudRESTAgent =
            std::shared_ptr<AddressBookCloudUploaderRESTAgent>(
                new AddressBookCloudUploaderRESTAgent(authDelegate, deviceInfo));
        ThrowIfNot(
            addressBookCloudRESTAgent->initialize(alexaEndpoints), "initializeAddressBookCloudUploaderRESTAgentFailed");

        return addressBookCloudRESTAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AddressBookCloudUploaderRESTAgent::initialize(
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    auto acmsendpoint = alexaEndpoints->getACMSEndpoint();
    if (!acmsendpoint.empty()) {
        m_acmsEndpoint = acmsendpoint;
        AACE_INFO(LX(TAG).m("usingACMSEndpointFromConfiguration"));
        AACE_DEBUG(LX(TAG).d("acmsEndpoint", m_acmsEndpoint));
    }

    return true;
}

bool AddressBookCloudUploaderRESTAgent::isAccountProvisioned() {
    if (!isPceIdValid()) {
        auto alexaAccount = getAlexaAccountInfo();
        if (!alexaAccount.commsId.empty()) {
            setPceId(getPceIdFromIdentity(alexaAccount.commsId));
        }
    }
    return isPceIdValid();
}

std::vector<std::string> AddressBookCloudUploaderRESTAgent::buildCommonHTTPHeader() {
    std::vector<std::string> httpHeaderData;
    std::string requestUUID = alexaClientSDK::avsCommon::utils::uuidGeneration::generateUUID();
    auto token = m_authDelegate->getAuthToken();
    auto userAgent =
        DEFAULT_USER_AGENT_VALUE + FORWARD_SLASH + aace::engine::core::version::getEngineVersion().toString();
    if (!token.empty()) {
        httpHeaderData = {AUTHORIZATION_HTTP_HEADER + COLON_SEPARATOR + SPACE_SEPARATOR +
                              AUTHORIZATION_HTTP_HEADER_POSTFIX + SPACE_SEPARATOR + token,
                          ACCEPT_PFM + COLON_SEPARATOR + SPACE_SEPARATOR + DEFAULT_PFM,
                          X_AMZN_REQUESTID + COLON_SEPARATOR + SPACE_SEPARATOR + requestUUID,
                          X_AZN_CLIENTID + COLON_SEPARATOR + SPACE_SEPARATOR + m_deviceInfo->getDeviceSerialNumber(),
                          USER_AGENT + COLON_SEPARATOR + SPACE_SEPARATOR + userAgent};
    }

    return httpHeaderData;
}

std::pair<bool, AddressBookCloudUploaderRESTAgent::HTTPResponse> AddressBookCloudUploaderRESTAgent::doPost(
    const std::string& url,
    const std::vector<std::string> headerLines,
    const std::string& data,
    std::chrono::seconds timeout) {
    AACE_DEBUG(LX(TAG));
    try {
        int retry = 0;
        do {
            // Creating the HttpPost on every doPost ensures most updated curl options set in libCurlUtils are used.
            auto httpPost = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost::create();
            ThrowIfNull(httpPost, "nullHttpPost");

            auto httpResponse = httpPost->doPost(url, headerLines, data, timeout);

            auto status = parseHTTPResponseCode(httpResponse.code);

            ThrowIf(status == HTTPResponseResult::FAILED, "doPostFailed");

            if (status == HTTPResponseResult::SUCCESS) {
                return std::make_pair(true, httpResponse);
            }

            if (retry < HTTP_RETRY_COUNT) {
                std::unique_lock<std::mutex> lock(m_wakeMutex);
                m_wake.wait_until(
                    lock, calculateTimeToRetry(retry++), [this] { return m_isShuttingDown ? true : false; });
                lock.unlock();

                if (m_isShuttingDown) {
                    AACE_INFO(LX(TAG).m("exitDueToShutdown"));
                    return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
                }
            } else {
                break;
            }
        } while (true);

        AACE_INFO(LX(TAG).m("exceededRetries"));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    }
}

std::pair<bool, AddressBookCloudUploaderRESTAgent::HTTPResponse> AddressBookCloudUploaderRESTAgent::doGet(
    const std::string& url,
    const std::vector<std::string>& headers) {
    AACE_DEBUG(LX(TAG));
    try {
        int retry = 0;
        do {
            // Creating the HttpGet on every doGet ensures most updated curl options set in libCurlUtils are used.
            auto httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
            ThrowIfNull(httpGet, "nullHttpGet");

            auto httpResponse = httpGet->doGet(url, headers, DEFAULT_HTTP_TIMEOUT);

            auto status = parseHTTPResponseCode(httpResponse.code);

            ThrowIf(status == HTTPResponseResult::FAILED, "doGetFailed");

            if (status == HTTPResponseResult::SUCCESS) {
                return std::make_pair(true, httpResponse);
            }

            if (retry < HTTP_RETRY_COUNT) {
                std::unique_lock<std::mutex> lock(m_wakeMutex);
                m_wake.wait_until(
                    lock, calculateTimeToRetry(retry++), [this] { return m_isShuttingDown ? true : false; });
                lock.unlock();

                if (m_isShuttingDown) {
                    AACE_INFO(LX(TAG).m("exitDueToShutdown"));
                    return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
                }
            } else {
                break;
            }
        } while (true);

        AACE_INFO(LX(TAG).m("exceededRetries"));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    }
}

std::pair<bool, AddressBookCloudUploaderRESTAgent::HTTPResponse> AddressBookCloudUploaderRESTAgent::doDelete(
    const std::string& url,
    const std::vector<std::string>& headers) {
    AACE_DEBUG(LX(TAG));
    try {
        int retry = 0;
        do {
            // Creating the HttpDelete on every doDelete ensures most updated curl options set in libCurlUtils are used.
            auto httpDelete = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete::create();
            ThrowIfNull(httpDelete, "nullHttpDelete");

            auto httpResponse = httpDelete->doDelete(url, headers, DEFAULT_HTTP_TIMEOUT);

            auto status = parseHTTPResponseCode(httpResponse.code);

            ThrowIf(status == HTTPResponseResult::FAILED, "doDeleteFailed");

            if (status == HTTPResponseResult::SUCCESS) {
                return std::make_pair(true, httpResponse);
            }

            if (retry < HTTP_RETRY_COUNT) {
                std::unique_lock<std::mutex> lock(m_wakeMutex);
                m_wake.wait_until(
                    lock, calculateTimeToRetry(retry++), [this] { return m_isShuttingDown ? true : false; });
                lock.unlock();

                if (m_isShuttingDown) {
                    AACE_INFO(LX(TAG).m("exitDueToShutdown"));
                    return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
                }
            } else {
                break;
            }
        } while (true);

        AACE_INFO(LX(TAG).m("exceededRetries"));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(false, AddressBookCloudUploaderRESTAgent::HTTPResponse());
    }
}

void AddressBookCloudUploaderRESTAgent::reset() {
    setPceId("");
}

void AddressBookCloudUploaderRESTAgent::doShutdown() {
    m_isShuttingDown = true;
    m_wake.notify_all();
}

bool AddressBookCloudUploaderRESTAgent::isPceIdValid() {
    return (!getPceId().empty());
}

AddressBookCloudUploaderRESTAgent::AlexaAccountInfo AddressBookCloudUploaderRESTAgent::getAlexaAccountInfo() {
    AACE_DEBUG(LX(TAG));
    AlexaAccountInfo alexaAccount;
    // Initialize to default values.
    alexaAccount.commsId = "";
    alexaAccount.provisionStatus = CommsProvisionStatus::INVALID;
    try {
        rapidjson::Document document;

        auto httpHeader = buildCommonHTTPHeader();
        if (httpHeader.size() == 0) {
            // When auth token is empty the size returned is 0.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return alexaAccount;
        }

        auto url = m_acmsEndpoint + FORWARD_SLASH + ACCOUNTS_PATH;

        auto result = doGet(url, httpHeader);

        ThrowIfNot(result.first, "doGetFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        if (document.Parse(result.second.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }
        if (!document.IsArray()) {
            Throw("jsonResponseNotValidArray");
        }

        for (rapidjson::Value::ConstValueIterator itr = document.Begin(); itr != document.End(); itr++) {
            if ((*itr)["signedInUser"].IsBool() && (*itr)["signedInUser"].GetBool()) {
                if ((*itr).HasMember("commsId")) {
                    if ((*itr)["commsId"].IsString()) {
                        alexaAccount.commsId = (*itr)["commsId"].GetString();
                    }
                } else {
                    Throw("commsIdNotPresent");
                }

                if ((*itr).HasMember("commsProvisionStatus")) {
                    if ((*itr)["commsProvisionStatus"].IsString()) {
                        auto tempCommsProvisionStatus = (*itr)["commsProvisionStatus"].GetString();
                        if (strcmp("UNKNOWN", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::UNKNOWN;
                        } else if (strcmp("PROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::PROVISIONED;
                        } else if (strcmp("DEPROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::DEPROVISIONED;
                        } else if (strcmp("AUTO_PROVISIONED", tempCommsProvisionStatus) == 0) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::AUTO_PROVISIONED;
                        }
                    }
                } else {
                    Throw("commsProvisionStatusNotValid");
                }
                break;  // Found the signed in user, so break.
            }
        }
        return alexaAccount;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return alexaAccount;
    }
}

std::string AddressBookCloudUploaderRESTAgent::getPceIdFromIdentity(const std::string& commsId) {
    AACE_DEBUG(LX(TAG));
    try {
        rapidjson::Document document;
        std::string pceId;
        auto url = m_acmsEndpoint + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + commsId + GET_INDENTITY_V2_QUERY;

        auto httpHeader = buildCommonHTTPHeader();
        if (httpHeader.size() == 0) {
            // When auth token is empty the size returned is 0.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return "";
        }

        auto result = doGet(url, httpHeader);

        ThrowIfNot(result.first, "doGetFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        if (document.Parse(result.second.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }

        auto it = document.FindMember("pceId");
        if (it != document.MemberEnd() && it->value.IsString()) {
            pceId = it->value.GetString();
        } else {
            Throw("notValidPceId");
        }
        return pceId;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

std::string AddressBookCloudUploaderRESTAgent::createAndGetCloudAddressBook(
    const std::string& addressBookSourceId,
    const std::string& addressBookType) {
    AACE_DEBUG(LX(TAG));
    try {
        auto httpHeaderData = buildCommonHTTPHeader();
        if (httpHeaderData.size() == 0) {
            // When auth token is empty the size returned is 0.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return "";
        }
        httpHeaderData.insert(httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON);

        rapidjson::Document document;
        auto addressDataJson = buildCreateAddressBookDataJson(addressBookSourceId, addressBookType);
        std::string addressBookId;

        auto url =
            m_acmsEndpoint + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + getPceId() + FORWARD_SLASH + ADDRESSBOOK_PATH;

        auto result = doPost(url, httpHeaderData, addressDataJson, DEFAULT_HTTP_TIMEOUT);

        ThrowIfNot(result.first, "doPostFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        if (document.Parse(result.second.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }

        auto it = document.FindMember("addressBookId");
        if (it != document.MemberEnd() && it->value.IsString()) {
            addressBookId = it->value.GetString();
        } else {
            Throw("invalidAddressBookId");
        }

        return addressBookId;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

static std::string hoursFromNowISO8601(int offsetInHours) {
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now + std::chrono::hours(offsetInHours));
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&t_c), "%FT%TZ");
    return ss.str();
}

std::string AddressBookCloudUploaderRESTAgent::buildCreateAddressBookDataJson(
    const std::string& addressBookSourceId,
    const std::string& addressBookType) {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember(
        "addressBookSourceId",
        rapidjson::Value().SetString(addressBookSourceId.c_str(), addressBookSourceId.length()),
        document.GetAllocator());
    document.AddMember(
        "addressBookName",
        rapidjson::Value().SetString(
            AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.c_str(), AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.length()),
        document.GetAllocator());
    document.AddMember(
        "addressBookType",
        rapidjson::Value().SetString(addressBookType.c_str(), addressBookType.length()),
        document.GetAllocator());
    auto validUntilDate = hoursFromNowISO8601(ADDRESS_BOOK_VALID_HOURS);
    document.AddMember(
        "validUntilDate",
        rapidjson::Value().SetString(validUntilDate.c_str(), validUntilDate.length()),
        document.GetAllocator());

    return aace::engine::utils::json::toString(document);
}

bool AddressBookCloudUploaderRESTAgent::getCloudAddressBookId(
    const std::string& addressBookSourceId,
    const std::string& addressBookType,
    std::string& cloudAddressBookId) {
    try {
        auto url = m_acmsEndpoint + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + getPceId() + FORWARD_SLASH +
                   ADDRESSBOOK_PATH + GET_ADDRESS_BOOK_QUERY + addressBookSourceId;

        auto httpHeader = buildCommonHTTPHeader();
        if (httpHeader.size() == 0) {
            // Size is 0 in cases where auth token is empty.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return false;
        }
        auto result = doGet(url, httpHeader);

        ThrowIfNot(result.first, "doGetFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        rapidjson::Document document;
        if (document.Parse(result.second.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }

        auto addressBooks = document.FindMember("addressBooks");
        if (addressBooks != document.MemberEnd() && addressBooks->value.IsArray()) {
            if (addressBooks->value.Size() >= 1) {
                for (rapidjson::Value::ConstValueIterator itr = addressBooks->value.Begin();
                     itr != addressBooks->value.End();
                     itr++) {
                    if ((*itr)["addressBookType"].IsString() &&
                        (strcmp(addressBookType.c_str(), (*itr)["addressBookType"].GetString()) == 0)) {
                        cloudAddressBookId = (*itr)["addressBookId"].GetString();
                        break;
                    }
                }
            }
        } else {
            Throw("addressBooksNotValid");
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

/**
 * Compress input byte stream with gzip.
 *
 * @param bytes the input byte stream
 * @param gzipped the output compressed stream
 * @return Z_OK if success. zlib error code otherwise.
 */
static int gzip(const std::string& bytes, std::string& gzipped) {
    z_stream zstr;
    zstr.zalloc = Z_NULL;
    zstr.zfree = Z_NULL;
    zstr.opaque = Z_NULL;
    zstr.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(bytes.data()));
    zstr.avail_in = bytes.length();

    int ret = deflateInit2(&zstr, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        deflateEnd(&zstr);
        return ret;
    }

    size_t maxOut = deflateBound(&zstr, bytes.length()) + 12;
    gzipped.resize(maxOut);
    zstr.next_out = reinterpret_cast<Bytef*>(&gzipped[0]);
    zstr.avail_out = gzipped.capacity();
    ret = deflate(&zstr, Z_FINISH);
    if (Z_STREAM_END != ret) {
        deflateEnd(&zstr);
        return ret;
    }

    ret = deflateEnd(&zstr);
    if (Z_OK != ret) {
        return ret;
    }

    gzipped.resize(zstr.total_out);
    return Z_OK;
}

AddressBookCloudUploaderRESTAgent::HTTPResponse AddressBookCloudUploaderRESTAgent::uploadDocumentToCloud(
    std::shared_ptr<rapidjson::Document> document,
    const std::string& cloudAddressBookId) {
    AACE_DEBUG(LX(TAG));
    try {
        auto httpHeaderData = buildCommonHTTPHeader();
        if (httpHeaderData.size() == 0) {
            // When auth token is empty the size returned is 0.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return AddressBookCloudUploaderRESTAgent::HTTPResponse();
        }
        httpHeaderData.insert(httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON);

        auto content = aace::engine::utils::json::toString(*document);
        std::string gzipped;
        int ret = gzip(content, gzipped);
        if (ret == Z_OK) {
            httpHeaderData.insert(httpHeaderData.end(), "Content-Encoding: gzip");
            content = std::move(gzipped);
        } else {
            AACE_ERROR(LX(TAG, "failedToCompressContent").d("error", ret));
        }

        auto url = m_acmsEndpoint + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + getPceId() + FORWARD_SLASH +
                   ADDRESSBOOK_PATH + FORWARD_SLASH + cloudAddressBookId + FORWARD_SLASH + ENTRIES_PATH;

        auto result = doPost(url, httpHeaderData, content, DEFAULT_HTTP_TIMEOUT);

        ThrowIfNot(result.first, "doPostFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        return result.second;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return AddressBookCloudUploaderRESTAgent::HTTPResponse();
    }
}

bool AddressBookCloudUploaderRESTAgent::parseCreateAddressBookEntryResponse(
    const HTTPResponse& response,
    std::queue<std::string>& failedEntries) {
    rapidjson::Document document;
    try {
        if (document.Parse(response.body.c_str()).HasParseError()) {
            Throw("jsonParseError");
        }

        auto references = document.FindMember("references");
        if (references != document.MemberEnd() && references->value.IsArray() && references->value.Size() > 0) {
            for (rapidjson::Value::ConstValueIterator itr = references->value.Begin(); itr != references->value.End();
                 itr++) {
                if ((*itr)["status"].IsString()) {
                    if (strcmp("FAILURE", (*itr)["status"].GetString()) == 0) {
                        failedEntries.push((*itr)["entrySourceId"].GetString());
                    }
                }
            }
        } else {
            Throw("referencesNotFound");
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploaderRESTAgent::deleteCloudAddressBook(const std::string& addressBookId) {
    AACE_DEBUG(LX(TAG));
    try {
        rapidjson::Document document;

        auto url = m_acmsEndpoint + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + getPceId() + FORWARD_SLASH +
                   ADDRESSBOOK_PATH + FORWARD_SLASH + addressBookId;

        auto httpHeader = buildCommonHTTPHeader();
        if (httpHeader.size() == 0) {
            // When auth token is empty the size returned is 0.
            AACE_WARN(LX(TAG).m("httpHeaderEmpty"));
            return false;
        }
        auto result = doDelete(url, httpHeader);

        ThrowIfNot(result.first, "doDeleteFailed:" + responseCodeToString((HTTPResponseCode)result.second.code));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AddressBookCloudUploaderRESTAgent::buildFailedEntriesJson(std::queue<std::string>& failedList) {
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value failedContacts;
    failedContacts.SetArray();

    rapidjson::Value failedContact;
    failedContact.SetObject();

    while (!failedList.empty()) {
        auto tempContact = failedList.front();
        failedList.pop();

        failedContact.AddMember(
            "id", rapidjson::Value().SetString(tempContact.c_str(), tempContact.length()), document.GetAllocator());
        failedContact.AddMember("reason", "Failed to Upload", document.GetAllocator());

        failedContacts.PushBack(failedContact, document.GetAllocator());
    }

    document.AddMember("failedContact", failedContacts, document.GetAllocator());

    return aace::engine::utils::json::toString(document);
}

void AddressBookCloudUploaderRESTAgent::setPceId(const std::string& pceId) {
    std::lock_guard<std::mutex> guard(m_pceIdMutex);
    m_pceId = pceId;
}

std::string AddressBookCloudUploaderRESTAgent::getPceId() {
    std::lock_guard<std::mutex> guard(m_pceIdMutex);
    return m_pceId;
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aace
