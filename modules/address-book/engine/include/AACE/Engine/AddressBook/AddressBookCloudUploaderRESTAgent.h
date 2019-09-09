
/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESSBOOK_CLOUD_UPLOADER_REST_AGENT_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESSBOOK_CLOUD_UPLOADER_REST_AGENT_H

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
namespace addressBook {

class AddressBookCloudUploaderRESTAgent {
private:
    AddressBookCloudUploaderRESTAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo
    );

public:
    using HTTPResponse = alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse;

    static std::shared_ptr<AddressBookCloudUploaderRESTAgent> create (
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo
    );

    virtual ~AddressBookCloudUploaderRESTAgent() = default;

    bool isAccountProvisioned();
    bool autoProvisionAccount();

    std::string createAndGetCloudAddressBook( const std::string& addressBookSourceId, const std::string& addressBookType );
    bool getCloudAddressBookId( const std::string& addressBookSourceId, const std::string& addressBookType, std::string& cloudAddressBookId );
    bool deleteCloudAddressBook( const std::string& cloudAddressBookId );
    
    HTTPResponse uploadDocumentToCloud( std::shared_ptr<rapidjson::Document> document, const std::string& cloudAddressBookId );
    bool parseCreateAddressBookEntryResponse( const HTTPResponse& response, std::queue<std::string>& failedEntries );
    std::string buildFailedEntriesJson( std::queue<std::string>& failedContact );

    std::string getHTTPErrorString( const HTTPResponse& response );

private:
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

    bool isPceIdValid();
    std::string getPceId();
    void setPceId( const std::string& pceId );

    AlexaAccountInfo getAlexaAccountInfo();
    std::string getPceIdFromIndentity( const std::string& commsId );
    bool doAccountAutoProvision( const std::string& directedId );

    std::vector<std::string> buildCommonHTTPHeader();
    bool parseCommonHTTPResponse( const HTTPResponse& response );

    HTTPResponse doPost( const std::string& url, const std::vector<std::string> headerLines, const std::string& data, std::chrono::seconds timeout );
    HTTPResponse doGet( const std::string& url, const std::vector<std::string>& headers );
    HTTPResponse doDelete( const std::string& url, const std::vector<std::string>& headers );


    std::string buildCreateAddressBookDataJson( const std::string& addressBookSourceId, const std::string& addressBookType );
    std::string buildEntriesJsonString( std::shared_ptr<rapidjson::Document> document );
    std::string buildAutoAccountProvisionJson();

private:
    std::string m_pceId;
    std::string m_addressBookId;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
};

} // aace::engine::addressBook
} // aace::engine
} // aace

#endif //AACE_ENGINE_ADDRESS_BOOK_ADDRESSBOOK_CLOUD_UPLOADER_REST_AGENT_H
