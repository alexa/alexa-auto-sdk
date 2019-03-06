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

#include "AACE/Engine/ContactUploader/ContactUploaderRESTAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/prettywriter.h> 
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace engine {
namespace contactUploader {

// String to identify log entries originating from this file.
static const std::string TAG("aace.contactuploader.ContactUploaderRESTAgent");

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
const std::string DEFAULT_USER_AGENT_VALUE = "AutoSDK/ContactUploader/1.0";

/// Default value for the Address Book Type @c ACMS
const std::string AUTO_SDK_DEFAULT_ADDRESS_BOOK_TYPE ="automotive";

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
static const std::string GET_INDENTITYV2_QUERY = "/identities?includeUserName=false";

/// Path suffix for URL used in ACMS Get Address Book Ids
static const std::string GET_ADDRESS_BOOK_QUERY = "?addressBookSourceIds=";

ContactUploaderRESTAgent::ContactUploaderRESTAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet> httpGet,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost> httpPost,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete> httpDelete,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ):
    m_httpGet( httpGet ),
    m_httpPost( httpPost ),
    m_httpDelete( httpDelete ),
    m_authDelegate( authDelegate ),
    m_deviceInfo( deviceInfo ) {
}

std::shared_ptr<ContactUploaderRESTAgent> ContactUploaderRESTAgent::create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ) {
    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet> httpGet;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost> httpPost;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete> httpDelete;

    try {
        // Create HTTP Get handler
        httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
        ThrowIfNull( httpGet, "nullHttpGet" );

        // Create HTTP Post handler
        httpPost = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost::create();
        ThrowIfNull( httpPost, "nullHttpPost" );

        // Create HTTP Delete handler
        httpDelete = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpDelete::create();
        ThrowIfNull( httpDelete, "nullHttpDelete" );

        std::shared_ptr<ContactUploaderRESTAgent> contactUploaderRESTAgent = std::shared_ptr<ContactUploaderRESTAgent>( new ContactUploaderRESTAgent( httpGet, httpPost, httpDelete, authDelegate, deviceInfo ) );
        return contactUploaderRESTAgent;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"create").d("reason", ex.what() ) );
        return nullptr;
    }
}

std::vector<std::string> ContactUploaderRESTAgent::buildCommonHTTPHeader() {
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

bool ContactUploaderRESTAgent::parseCommonHTTPResponse( const HTTPResponse& response ) {
    if( HTTPResponseCode::SUCCESS_OK == response.code ) {
        //TBD Check the content-type in Response Header
        return true;
    }
    return false;
}

std::string ContactUploaderRESTAgent::getHTTPErrorString( const HTTPResponse& response ) {
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

ContactUploaderRESTAgent::AlexaAccountInfo ContactUploaderRESTAgent::getAlexaAccountInfo() {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
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
            httpResponse = m_httpGet->doGet( 
                ACMS_ENDPOINT + FORWARD_SLASH + ACCOUNTS_PATH, 
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoGetFailed" + getHTTPErrorString( httpResponse ) );

        if( document.Parse(httpResponse.body.c_str()).HasParseError() ) {
            Throw( "jsonPraseError" );
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
                        auto tempcommsProvisionStatus = (*itr)[ "commsProvisionStatus" ].GetString();
                        if( strcmp("UNKNOWN", tempcommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::UNKNOWN;
                        } else if( strcmp("PROVISIONED", tempcommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::PROVISIONED;
                        } else if( strcmp("DEPROVISIONED", tempcommsProvisionStatus ) == 0 ) {
                            alexaAccount.provisionStatus = CommsProvisionStatus::DEPROVISIONED;
                        } else if( strcmp("AUTO_PROVISIONED", tempcommsProvisionStatus ) == 0 ) {
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
        AACE_ERROR( LX(TAG,"getAlexaAccountDetails").d("reason", ex.what() ) );
        return alexaAccount;
    }
}

bool ContactUploaderRESTAgent::doAccountAutoProvision( const std::string& directedId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );

    auto autoProvisionJson = buildAutoAccountProvisionJson();
    try {
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = m_httpPost->doPost(
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

std::string ContactUploaderRESTAgent::buildAutoAccountProvisionJson() {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember( "autoProvision", rapidjson::Value().SetBool(true), document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return( std::string( buffer.GetString() ) );
}

std::string ContactUploaderRESTAgent::getPceId( const std::string& commsId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    std::string pceId = "";
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();

    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = m_httpGet->doGet( 
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + commsId + GET_INDENTITYV2_QUERY, 
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
        AACE_ERROR( LX(TAG,"getPceId").d("reason", ex.what() ) );
        return "";
    }
}

std::string ContactUploaderRESTAgent::createAndGetAddressBookId( const std::string& sourceAddressBookId, const std::string& pceId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    std::string addressBookId = "";
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );
    auto addressDataJson = buildCreateAddressBookDataJson( sourceAddressBookId );

    try {
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
        httpResponse = m_httpPost->doPost(
            ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + pceId + FORWARD_SLASH + ADDRESSBOOK_PATH,
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
        AACE_ERROR( LX(TAG,"createAndGetAddressBookId").d("reason", ex.what() ) );
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
std::string ContactUploaderRESTAgent::buildCreateAddressBookDataJson( const std::string& sourceAddressBookId ) {
    rapidjson::Document document;
    document.SetObject();

    document.AddMember( "addressBookSourceId", rapidjson::Value().SetString( sourceAddressBookId.c_str(), sourceAddressBookId.length() ), document.GetAllocator() );
    document.AddMember( "addressBookName", rapidjson::Value().SetString( AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.c_str(), AUTO_SDK_DEFAULT_ADDRESS_BOOK_NAME.length() ) , document.GetAllocator() );
    document.AddMember( "addressBookType", rapidjson::Value().SetString( AUTO_SDK_DEFAULT_ADDRESS_BOOK_TYPE.c_str(), AUTO_SDK_DEFAULT_ADDRESS_BOOK_TYPE.length() )  , document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);
    return( std::string( buffer.GetString() ) );
}

std::string ContactUploaderRESTAgent::getAddressBookId( const std::string& sourceAddressBookId, const std::string& pceId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    std::string addressBookId = "";
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = m_httpGet->doGet(
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + GET_ADDRESS_BOOK_QUERY + sourceAddressBookId,
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
            if( addressBooks->value.Size() == 1 ) {
                for( rapidjson::Value::ConstValueIterator itr = addressBooks->value.Begin(); itr != addressBooks->value.End(); itr++ ) {
                    if( (*itr)[ "addressBookType" ].IsString() && ( strcmp( "automotive", (*itr)[ "addressBookType" ].GetString() ) == 0 ) ) {
                        addressBookId = (*itr)[ "addressBookId" ].GetString();
                        break;
                    } else {
                        Throw( "addressBooksReturnedNotAutomotiveType" );
                    }
                }
            }
        } else {
            Throw( "addressBooksNotValid" );
        }
        return addressBookId;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"getAddresssBookId").d("reason", ex.what() ) );
        return "";
    }
}

ContactUploaderRESTAgent::HTTPResponse ContactUploaderRESTAgent::uploadContactToAddressBook( const std::vector<std::string>& poppedContacts, const std::string& addressBookId, const std::string& pceId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;

    auto httpHeaderData = buildCommonHTTPHeader( );
    httpHeaderData.insert( httpHeaderData.end(), CONTENT_TYPE_APPLICATION_JSON );
    
    auto contactsJson = buildContactsJson( poppedContacts );

    for( int retryCount = 0; retryCount < HTTP_RETRY_COUNT; retryCount++  ) {
        httpResponse = m_httpPost->doPost(
            ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + FORWARD_SLASH + addressBookId + FORWARD_SLASH + ENTRIES_PATH,
            httpHeaderData, contactsJson, DEFAULT_HTTP_TIMEOUT );

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
std::string ContactUploaderRESTAgent::buildContactsJson( const std::vector<std::string>& contacts ) {
    rapidjson::Document srcDocument, destDocument;
    rapidjson::Value entries;
    
    destDocument.SetObject();
    entries.SetArray();
    try {
        for( auto contact : contacts ) {
            if( srcDocument.Parse( contact.c_str() ).HasParseError() ) {
                Throw( "buildACMSContactJsonError" );
            }

            rapidjson::Value contactDetails;
            contactDetails.SetObject();

            rapidjson::Value addresses;
            addresses.SetArray();

            rapidjson::Value data;
            data.SetObject();

            rapidjson::Value destName;
            destName.SetObject();

            if( srcDocument.HasMember( "id" ) ) {
                contactDetails.AddMember( "entrySourceId",rapidjson::Value().SetString( srcDocument[ "id" ].GetString(), srcDocument[ "id" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
            } else {
                Throw( "idNotValid" );
            }

            if( srcDocument.HasMember( "firstName" ) ) {
                destName.AddMember( "firstName",rapidjson::Value().SetString( srcDocument[ "firstName" ].GetString(), srcDocument[ "firstName" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
            }
            if( srcDocument.HasMember( "lastName" ) ) {
                destName.AddMember( "lastName",rapidjson::Value().SetString( srcDocument[ "lastName" ].GetString(), srcDocument[ "lastName" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
            }
            if( srcDocument.HasMember( "nickName" ) ) {
                destName.AddMember( "nickName",rapidjson::Value().SetString( srcDocument[ "nickName" ].GetString(), srcDocument[ "nickName" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
            }

            auto srcAddresses = srcDocument.FindMember( "addresses" );
            if( srcAddresses != srcDocument.MemberEnd() && srcAddresses->value.IsArray() ) {
                for( rapidjson::Value::ConstValueIterator itr = srcAddresses->value.Begin(); itr != srcAddresses->value.End(); itr++ ) {
                    rapidjson::Value address;
                    address.SetObject();

                    if( (*itr).HasMember( "type" ) && (*itr)[ "type" ].IsString() ) {
                        address.AddMember( "addressType", rapidjson::Value().SetString( (*itr)[ "type" ].GetString(), (*itr)[ "type" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
                    } else {
                        Throw( "typeNotValid" );
                    }
                    if( (*itr).HasMember( "value" ) && (*itr)[ "value" ].IsString() ) {
                        address.AddMember( "value", rapidjson::Value().SetString( (*itr)[ "value" ].GetString(), (*itr)[ "value" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
                    } else {
                        Throw( "valueNotValid" );
                    }
                    if( (*itr).HasMember( "label" ) && (*itr)[ "label" ].IsString() ) {
                        address.AddMember( "rawType", rapidjson::Value().SetString( (*itr)[ "label" ].GetString(), (*itr)[ "label" ].GetStringLength(), destDocument.GetAllocator() ), destDocument.GetAllocator() );
                    }
                    addresses.PushBack( address, destDocument.GetAllocator() );
                }
            } else {
                Throw( "addressesNotValid" );
            }

            data.AddMember( "name", destName, destDocument.GetAllocator() );
            data.AddMember( "addresses", addresses, destDocument.GetAllocator() );

            if( srcDocument.HasMember( "company" ) ) {
                data.AddMember( "company", rapidjson::Value().SetString( srcDocument[ "company" ].GetString(), srcDocument[ "company" ].GetStringLength(), destDocument.GetAllocator() ),destDocument.GetAllocator() );
            }

            contactDetails.AddMember( "data", data, destDocument.GetAllocator() );

            entries.PushBack( contactDetails, destDocument.GetAllocator() );
        }

        destDocument.AddMember( "entries", entries, destDocument.GetAllocator() );

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

        destDocument.Accept( writer );

        return std::string( buffer.GetString() );
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"buildContactsJson").d("reason", ex.what() ) );
        return "";
    }

}

bool ContactUploaderRESTAgent::parseCreateAddressBookEntryForFailedStatus( const HTTPResponse& response, std::queue<std::string>& failedContact  ) {
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
                        failedContact.push( (*itr)[ "entrySourceId" ].GetString() );
                    }
                }
            }
        } else {
            Throw( "referencesNotFound" );
        }
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"parseCreateAddressBookEntryForFailedContacts").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderRESTAgent::deleteAddressBookId( const std::string& addressBookId, const std::string& pceId ) {
    ContactUploaderRESTAgent::HTTPResponse httpResponse;
    rapidjson::Document document;
    bool validFlag = false;

    auto httpHeaderData = buildCommonHTTPHeader();
    try { 
        for( int retry = 0; retry < HTTP_RETRY_COUNT; retry++ ) {
            httpResponse = m_httpDelete->doDelete(
                ACMS_ENDPOINT + FORWARD_SLASH + USERS_PATH + FORWARD_SLASH + pceId + FORWARD_SLASH + ADDRESSBOOK_PATH + FORWARD_SLASH + addressBookId,
                httpHeaderData );

            if( parseCommonHTTPResponse( httpResponse ) ) {
                validFlag = true;
                break;
            }
        }
        ThrowIfNot( validFlag, "httpDoDeleteFailed" + getHTTPErrorString( httpResponse ) );
        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"deleteAddressBookId").d("reason", ex.what() ) );
        return false;
    }
}

std::string ContactUploaderRESTAgent::buildFailedContactsJson( std::queue<std::string>& failedList ){
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

} // aace::engine::contactUploader
} // aace::engine
} // aace
