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

#ifndef AACE_ENGINE_CONTACTUPLOADER_REST_AGENT_H
#define AACE_ENGINE_CONTACTUPLOADER_REST_AGENT_H

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <queue>

#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpGet.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpDelete.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpResponseCodes.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPResponse.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>

namespace aace {
namespace engine {
namespace contactUploader {

class ContactUploaderRESTAgent {
private:
    ContactUploaderRESTAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet> httpGet,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost> httpPost,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete> httpDelete,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo
    );

public:
    using HTTPResponse = alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse;

    static std::shared_ptr<ContactUploaderRESTAgent> create (
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo
    );

    virtual ~ContactUploaderRESTAgent() = default;

    enum class CommsProvisionStatus {
        /*
         * Invalid state
         */
        INVALID,
        /*
         * Status is new or unknown.
         */
        UNKNOWN,
        /*
         * Account is provisioned.
         */
        PROVISIONED,
        /*
         * Account is deprovisioned.
         */
        DEPROVISIONED,
        /*
         * Account is auto provisioned.
         */
        AUTO_PROVISIONED,
    };

    struct AlexaAccountInfo {
        std::string commsId;
        std::string directedId;
        CommsProvisionStatus provisionStatus;
    };

    AlexaAccountInfo getAlexaAccountInfo();
    std::string getPceId( const std::string& commsId );

    std::string createAndGetAddressBookId( const std::string& sourceAddressBookId, const std::string& pceId );
    std::string getAddressBookId( const std::string& sourceAddressBookId, const std::string& pceId );
    bool deleteAddressBookId( const std::string& addressBookId, const std::string& pceId );
    bool doAccountAutoProvision( const std::string& directedId );

    HTTPResponse uploadContactToAddressBook( const std::vector<std::string>& poppedContacts, const std::string& addressBookId, const std::string& pceId );
    bool parseCreateAddressBookEntryForFailedStatus( const HTTPResponse& response, std::queue<std::string>& failedResponse );
    std::string buildFailedContactsJson( std::queue<std::string>& failedContact );

    std::string getHTTPErrorString( const HTTPResponse& response );

private:
    std::vector<std::string> buildCommonHTTPHeader();
    bool parseCommonHTTPResponse( const HTTPResponse& response );

    std::string buildCreateAddressBookDataJson( const std::string& sourceAddressBookId );
    std::string buildContactsJson( const std::vector<std::string>& contacts );
    std::string buildAutoAccountProvisionJson();

    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet> m_httpGet;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost> m_httpPost;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete> m_httpDelete;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
};

} // aace::engine::contactUploader
} // aace::engine
} // aace

#endif
