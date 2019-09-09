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

#include <rapidjson/prettywriter.h> 
#include <rapidjson/stringbuffer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/AddressBook/AddressBookCloudUploaderRESTAgent.h>

namespace aace {
namespace engine {
namespace addressBook {

// String to identify log entries originating from this file.
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
const std::string AUTO_SDK_CONTACT_ADDRESS_BOOK_TYPE ="automotive";

/// Address Book Type to be used for Navigation Address as defined @c ACMS
const std::string AUTO_SDK_NAVIGATION_ADDRESS_ADDRESS_BOOK_TYPE ="automotivePostalAddress";

/// Default value for the Address Book Name @c ACMS
const std::string AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME ="AutoSDK";

/// Default value for the HTTP request timeout.
static const std::chrono::seconds DEFAULT_HTTP_TIMEOUT = std::chrono::seconds(60);

/// Default value for the HTTP retry on Network Error.
static const int  HTTP_RETRY_COUNT = 3;

// ACMS End Point 
static const std::string ACMS_ENDPOINT = "https://alexa-comms-mobile-service-na.amazon.com";

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

AddressBookCloudUploaderRESTAgent::AddressBookCloudUploaderRESTAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ):
    m_authDelegate( authDelegate ),
    m_deviceInfo( deviceInfo ) {
}

std::shared_ptr<AddressBookCloudUploaderRESTAgent> AddressBookCloudUploaderRESTAgent::create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ) {
    try {
        std::shared_ptr<AddressBookCloudUploaderRESTAgent> addressBookCloudRESTAgent = std::shared_ptr<AddressBookCloudUploaderRESTAgent>( new AddressBookCloudUploaderRESTAgent( authDelegate, deviceInfo ) );
        return addressBookCloudRESTAgent;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"create").d("reason", ex.what() ) );
        return nullptr;
    }
}

bool AddressBookCloudUploaderRESTAgent::isAccountProvisioned() {
    if( !isPceIdValid() ) {
        auto alexaAccount = getAlexaAccountInfo();
        if( !alexaAccount.commsId.empty() ) {
            m_pceId = getPceIdFromIndentity( alexaAccount.commsId );
        }
    }
    return isPceIdValid();
}

bool AddressBookCloudUploaderRESTAgent::autoProvisionAccount() {
    try {
        auto alexaAccount = getAlexaAccountInfo();
        ThrowIf( alexaAccount.provisionStatus == CommsProvisionStatus::INVALID, "getAlexaAccountDetailsFailed" );

        if( alexaAccount.provisionStatus == CommsProvisionStatus::UNKNOWN || alexaAccount.provisionStatus == CommsProvisionStatus::DEPROVISIONED ) {
            // Auto-provision the accountA
            ThrowIfNot( doAccountAutoProvision( alexaAccount.directedId ), "accountAutoProvisionFailed" );
        
            alexaAccount = getAlexaAccountInfo(); // get commsId after auto provisioning.
            ThrowIf( alexaAccount.commsId.empty(), "commsIdEmptyAfterProvisioning" );
        }

        ThrowIf( alexaAccount.commsId.empty(), "commsIdEmptyForProvisionedAccount" );

        m_pceId = getPceIdFromIndentity( alexaAccount.commsId );
        ThrowIf( m_pceId.empty(), "pceIdEmptyForProvisionedAccount" );
        return true;

    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"autoProvisionAccount").d( "reason", ex.what() ) );
        return false;
    }
}

std::vector<std::string> AddressBookCloudUploaderRESTAgent::buildCommonHTTPHeader() {
    std::vector<std::string> httpHeaderData;
    std::string requestUUID = alexaClientSDK::avsCommon::utils::uuidGeneration::generateUUID();
    httpHeaderData = {
        AUTHORIZATION_HTTP_HEADER + COLON_SEPARATOR + SPACE_SEPARATOR + AUTHORIZATION_HTTP_HEADER_POSTFIX + SPACE_SEPARATOR + m_authDelegate->getAuthToken(),
        ACCEPT_PFM + COLON_SEPARATOR + SPACE_SEPARATOR + DEFAULT_PFM,
        X_AMZN_REQUESTID + COLON_SEPARATOR + SPACE_SEPARATOR + requestUUID,
        X_AZN_CLIENTID + COLON_SEPARATOR + SPACE_SEPARATOR + m_deviceInfo->getDeviceSerialNumber(),
        USER_AGENT + COLON_SEPARATOR + SPACE_SEPARATOR + DEFAULT_USER_AGENT_VALUE
    };

    return httpHeaderData;
}

bool AddressBookCloudUploaderRESTAgent::parseCommonHTTPResponse( const HTTPResponse& response ) {
    if( HTTPResponseCode::SUCCESS_OK == response.code ) {
        //TBD Check the content-type in Response Header
        return true;
    }
    return false;
}

AddressBookCloudUploaderRESTAgent::HTTPResponse AddressBookCloudUploaderRESTAgent::doPost( const std::string& url, const std::vector<std::string> headerLines, const std::string& data, std::chrono::seconds timeout ) {
    try {
        // Creating the HttpPost on every doPost is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpPost = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost::create();
        ThrowIfNull( httpPost, "nullHttpPost" );

        return httpPost->doPost( url, headerLines, data, timeout );
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"doPost").d("reason", ex.what() ) );
        return AddressBookCloudUploaderRESTAgent::HTTPResponse();
    }
}

AddressBookCloudUploaderRESTAgent::HTTPResponse AddressBookCloudUploaderRESTAgent::doGet( const std::string& url, const std::vector<std::string>& headers ) {
    try {
        // Creating the HttpGet on every doGet is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
        ThrowIfNull( httpGet, "nullHttpGet" );

        return httpGet->doGet( url, headers );
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"doGet").d("reason", ex.what() ) );
        return AddressBookCloudUploaderRESTAgent::HTTPResponse();
    }
}

AddressBookCloudUploaderRESTAgent::HTTPResponse AddressBookCloudUploaderRESTAgent::doDelete( const std::string& url, const std::vector<std::string>& headers ) {
    try {
        // Creating the HttpDelete on every doDelete is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpDelete = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete::create();
        ThrowIfNull( httpDelete, "nullHttpDelete" );

        return httpDelete->doDelete( url, headers );
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"doDelete").d("reason", ex.what() ) );
        return AddressBookCloudUploaderRESTAgent::HTTPResponse();
    }
}

std::string AddressBookCloudUploaderRESTAgent::getHTTPErrorString( const HTTPResponse& response ) {
    switch( response.code ) {
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


bool AddressBookCloudUploaderRESTAgent::isPceIdValid() {
    return( !m_pceId.empty() );
}

AddressBookCloudUploaderRESTAgent::AlexaAccountInfo AddressBookCloudUploaderRESTAgent::getAlexaAccountInfo() {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    AlexaAccountInfo alexaAccount;

    // Initialize to default values.
    alexaAccount.commsId = "";
    alexaAccount.directedId = "";
    alexaAccount.provisionStatus = CommsProvisionStatus::INVALID;

    auto httpHeaderData = buildCommonHTTPHeader();
    bool validFlag = false;
    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = doGet( 
                ACMS_ENDPOINT + FORWARD_SLASH + ACCOUNTS_PATH, 
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoGetFailed" + getHTTPErrorString( httpResponse ) );

        if( document.Parse(httpResponse.body.c_str()).HasParseError() ) {
            Throw( "jsonParseError" );
        }
        if( !document.IsArray() ) {
            Throw( "jsonResponseNotValidArray" );
        }

        // Example Response: Array of Alexa Accounts.
        // [
        //   {
        //     "commsId": "string",
        //     "directedId": "string",
        //     "phoneCountryCode": "string",
        //     "phoneNumber": "string",
        //     "firstName": "string",
        //     "lastName": "string",
        //     "signedInUser": false,
        //     "commsProvisioned": false,
        //     "commsProvisionStatus":Enum String,
        //     "isChild": boolean
        //     "speakerProvisioned": false
        //   }
        // ]
        for( rapidjson::Value::ConstValueIterator itr = document.Begin(); itr != document.End(); itr++ ) {
            if( (*itr)[ "signedInUser" ].IsBool() && (*itr)[ "signedInUser" ].GetBool() ) { // Check if signedInUser is true
                if( (*itr).HasMember( "commsId" ) ) {
                    if( (*itr)[ "commsId" ].IsString() ) {
                        alexaAccount.commsId = (*itr)[ "commsId" ].GetString();
                    }
                } else {
                    Throw( "commsIdNotPresent" );
                }

                if( (*itr).HasMember( "directedId" ) ) {
                    if( (*itr)[ "directedId" ].IsString() ) {
                        alexaAccount.directedId = (*itr)[ "directedId" ].GetString();
                    }
                } else {
                    Throw( "directedIdNotPresent" );
                }

                if( (*itr).HasMember( "commsProvisionStatus" ) ) {
                    if( (*itr)[ "commsProvisionStatus" ].IsString() ) {
                        auto tempCommsProvisionStatus = (*itr)[ "commsProvisionStatus" ].GetString();
                        if( strcmp("UNKNOWN", tempCommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::UNKNOWN;
                        } else if( strcmp("PROVISIONED", tempCommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::PROVISIONED;
                        } else if( strcmp("DEPROVISIONED", tempCommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::DEPROVISIONED;
                        } else if( strcmp("AUTO_PROVISIONED", tempCommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::AUTO_PROVISIONED;
                        }
                    }
                } else {
                    Throw( "commsProvisionStatusNotValid" );
                }
                break; // Found the singedInUser, so break.
            }
        }
        return alexaAccount;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"getAlexaAccountInfo").d("reason", ex.what() ) );
        return alexaAccount;
    }
}

bool AddressBookCloudUploaderRESTAgent::doAccountAutoProvision( const std::string& directedId ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );

    auto autoProvisionJson = buildAutoAccountProvisionJson();
    try {
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = doPost(
                ACMS_ENDPOINT + FORWARD_SLASH + ACCOUNTS_PATH + FORWARD_SLASH + directedId + FORWARD_SLASH + USERS_PATH,
                httpHeaderData, autoProvisionJson, DEFAULT_HTTP_TIMEOUT );
            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoPostFailed" + getHTTPErrorString( httpResponse ) );
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"doAccountAutoProvision").d( "reason", ex.what() ) );
        return false;
    }
}

std::string AddressBookCloudUploaderRESTAgent::buildAutoAccountProvisionJson() {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember( "autoProvision", rapidjson::Value().SetBool(true), document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return( std::string( buffer.GetString() ) );
}

std::string AddressBookCloudUploaderRESTAgent::getPceIdFromIndentity( const std::string& commsId ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    std::string pceId = "";
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();

    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = doGet( 
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + commsId + GET_INDENTITY_V2_QUERY,
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            } 
        }

        ThrowIfNot( validFlag, "httpDoGetFailed" + getHTTPErrorString( httpResponse ) );

        // Example Response
        // getIdentityV2 Response.
        // {
        //     "commsId": "string",
        //     "pceId": "string",
        //     "homeGroupId": "string",
        //     "aor": "string",
        //     "hashedCommsId": "string",
        //     "name": null,
        //     "commsProvisionStatus": "AUTO_PROVISIONED"
        // }

        if( document.Parse(httpResponse.body.c_str()).HasParseError() ) {
            Throw( "jsonParseError" );
        }

        auto it = document.FindMember( "pceId" );
        if( it != document.MemberEnd() && it->value.IsString() ) {
            pceId = it->value.GetString();
        } else {
             Throw( "notValidPceId" );
        }
        return pceId;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"getPceIdFromIndentity").d("reason", ex.what() ) );
        return "";
    }
}

std::string AddressBookCloudUploaderRESTAgent::createAndGetCloudAddressBook( const std::string& addressBookSourceId, const std::string& addressBookType ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    std::string addressBookId = "";
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );
    auto addressDataJson = buildCreateAddressBookDataJson( addressBookSourceId, addressBookType );

    try {
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
        httpResponse = doPost(
            ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + m_pceId + FORWARD_SLASH + ADDRESSBOOK_PATH,
            httpHeaderData, addressDataJson, DEFAULT_HTTP_TIMEOUT );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }

        ThrowIfNot( validFlag, "httpDoPostFailed" + getHTTPErrorString( httpResponse ) );

        if( document.Parse( httpResponse.body.c_str()).HasParseError() ) {
            Throw( "jsonParseError" );
        }

        // Example Response
        // {
        //     addressBookId : "id"
        // }
        auto it = document.FindMember( "addressBookId" );
        if( it != document.MemberEnd() && it->value.IsString() ) {
            addressBookId = it->value.GetString();
        } else {
            Throw( "notValidAddressBookId" );
        }

        return addressBookId;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"createAndGetCloudAddressBook").d("reason", ex.what() ) );
        return "";
    }
}

// Sample JSON Request Format
// POST /users/{pceId}/addressbooks
// {
//     addressBookSourceId : "srcId",
//     addressBookName : "Test",
//     addressBookType : "phone"
// }
std::string AddressBookCloudUploaderRESTAgent::buildCreateAddressBookDataJson( const std::string& addressBookSourceId, const std::string& addressBookType ) {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember( "addressBookSourceId", rapidjson::Value().SetString( addressBookSourceId.c_str(), addressBookSourceId.length() ), document.GetAllocator() );
    document.AddMember( "addressBookName", rapidjson::Value().SetString( AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.c_str(), AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.length() ) , document.GetAllocator() );
    document.AddMember( "addressBookType", rapidjson::Value().SetString( addressBookType.c_str(), addressBookType.length() )  , document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return( std::string( buffer.GetString() ) );
}

bool AddressBookCloudUploaderRESTAgent::getCloudAddressBookId( const std::string& addressBookSourceId, const std::string& addressBookType, std::string& cloudAddressBookId ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = doGet(
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + m_pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + GET_ADDRESS_BOOK_QUERY + addressBookSourceId,
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoGetFailed" + getHTTPErrorString( httpResponse ) );

        if( document.Parse( httpResponse.body.c_str() ).HasParseError() ) {
            Throw( "jsonParseError" );
        }

        // Example Response
        // {
        //     "addressBooks": [
        //         {
        //             "addressBookId": "string",
        //             "addressBookType": "automotive",
        //             "addressBookSourceId": "string",
        //             "addressBookName": "string"
        //         }
        //     ]
        // }
        auto addressBooks = document.FindMember( "addressBooks" );
        if( addressBooks != document.MemberEnd() && addressBooks->value.IsArray() ) {
            if( addressBooks->value.Size() >= 1 ) {
                for( rapidjson::Value::ConstValueIterator itr = addressBooks->value.Begin(); itr != addressBooks->value.End(); itr++ ) {
                    if( (*itr)[ "addressBookType" ].IsString() && ( strcmp( addressBookType.c_str(), (*itr)[ "addressBookType" ].GetString() ) == 0 ) ) {
                        cloudAddressBookId = (*itr)[ "addressBookId" ].GetString();
                        break;
                    }
                }
            }
        } else {
            Throw( "addressBooksNotValid" );
        }
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"getCloudAddressBookId").d("reason", ex.what() ) );
        return false;
    }
}

AddressBookCloudUploaderRESTAgent::HTTPResponse AddressBookCloudUploaderRESTAgent::uploadDocumentToCloud( std::shared_ptr<rapidjson::Document> document, const std::string& cloudAddressBookId  ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;

    auto httpHeaderData = buildCommonHTTPHeader();
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );
    
    auto entriesJson = buildEntriesJsonString( document );

    for( int retryCount = 0; retryCount < HTTP_RETRY_COUNT; retryCount++ ) {
        httpResponse = doPost(
            ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + m_pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + FORWARD_SLASH + cloudAddressBookId + FORWARD_SLASH + ENTRIES_PATH,
            httpHeaderData, entriesJson, DEFAULT_HTTP_TIMEOUT );

        switch( httpResponse.code ) {
            case HTTPResponseCode::SUCCESS_OK:
                return httpResponse;
            case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
            case HTTPResponseCode::SUCCESS_NO_CONTENT:
            case HTTPResponseCode::REDIRECTION_START_CODE:
            case HTTPResponseCode::REDIRECTION_END_CODE:
            case HTTPResponseCode::BAD_REQUEST:  
            case HTTPResponseCode::FORBIDDEN:
            case HTTPResponseCode::SERVER_INTERNAL_ERROR:
                break; //Retry
        }
    }

    return httpResponse;
}

// {
//         "id" : "string",
//         "firstName" : "string",
//         "lastName" : "string",
//         "nickName" : "string",
//         "company" : "string",
//         "addresses" : [
//              {
//                  "type" : "string",
//                  "value" : "string",
//                  "label" : "string"
//              }
//         ]
// }
std::string AddressBookCloudUploaderRESTAgent::buildEntriesJsonString( std::shared_ptr<rapidjson::Document> document ) {
    try {    
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

        document->Accept( writer );

        return std::string( buffer.GetString() );
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"buildEntriesJsonString").d("reason", ex.what() ) );
        return "";
    }

}

bool AddressBookCloudUploaderRESTAgent::parseCreateAddressBookEntryResponse( const HTTPResponse& response, std::queue<std::string>& failedEntries  ) {
    rapidjson::Document document;
    try {
        if( document.Parse( response.body.c_str() ).HasParseError() ) {
            Throw( "jsonParseError" );
        }

        // {
        //     "references": [
        //         {
        //             "entryId": "string",
        //             "entrySourceId": "srcId",
        //             "status": "SUCCESS",
        //             "reason": null
        //         }
        //     ]
        // }
        auto references = document.FindMember( "references" );
        if( references != document.MemberEnd() && references->value.IsArray() && references->value.Size() > 0 ) {
            for( rapidjson::Value::ConstValueIterator itr = references->value.Begin(); itr != references->value.End(); itr++ ){
                if( (*itr)[ "status" ].IsString() ) {
                    if( strcmp( "FAILURE", (*itr)[ "status" ].GetString() ) == 0 ) {
                        failedEntries.push( (*itr)[ "entrySourceId" ].GetString() );
                    }
                }
            }
        } else {
            Throw( "referencesNotFound" );
        }
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"parseCreateAddressBookEntryResponse").d("reason", ex.what() ) );
        return false;
    }
}

bool AddressBookCloudUploaderRESTAgent::deleteCloudAddressBook( const std::string& addressBookId ) {
    AddressBookCloudUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = doDelete(
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + m_pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + FORWARD_SLASH + addressBookId,
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoDeleteFailed" + getHTTPErrorString( httpResponse ) );
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"deleteCloudAddressBook").d("reason", ex.what() ) );
        return false;
    }
}

std::string AddressBookCloudUploaderRESTAgent::buildFailedEntriesJson( std::queue<std::string>& failedList ) {
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value failedContacts;
    failedContacts.SetArray();

    rapidjson::Value failedContact;
    failedContact.SetObject();

    while( !failedList.empty() ) {
        auto tempContact = failedList.front();
        failedList.pop();

        failedContact.AddMember( "id", rapidjson::Value().SetString( tempContact.c_str(), tempContact.length() ), document.GetAllocator() );
        failedContact.AddMember( "reason", "Failed to Upload", document.GetAllocator() );

        failedContacts.PushBack( failedContact, document.GetAllocator() );
    }

    document.AddMember( "failedContact", failedContacts, document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return( std::string( buffer.GetString() ) );
}

} // aace::engine::addressBook
} // aace::engine
} // aace
