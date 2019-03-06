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

#include "AACE/Engine/ContactUploader/ContactUploaderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace contactUploader {

// String to identify log entries originating from this file.
static const std::string TAG("aace.contactuploader.ContactUploaderEngineImpl");

/// Contact Batch Upload Size
static const int MAX_BATCH_SIZE = 100;

/// Max allowed phonenumbers per contact
static const int MAX_ALLOWED_PHONENUMBER_PER_CONTACT = 30;

/// Max allowed Characters
static const int MAX_ALLOWED_CHARACTERS = 1000;

/// Max allowed Characters
static const int MAX_ALLOWED_PHONENUMBER_CHARACTERS = 100;

/// Empty String
static const std::string EMPTY_STRING = "";

ContactUploaderEngineImpl::ContactUploaderEngineImpl(
    std::shared_ptr<aace::contactUploader::ContactUploader> contactUploaderPlatformInterface ) : 
    alexaClientSDK::avsCommon::utils::RequiresShutdown( TAG ),
    m_contactUploaderPlatformInterface( contactUploaderPlatformInterface ),
    m_contactUploadState( ContactUploaderInternalState::IDLE ),
    m_pceId( "" ),
    m_addressBookId( "" ),
    m_isStopping( false ),
    m_isAuthRefreshed( false ),
    m_deleteAddressBookOnEngineStart( false ) {
}

bool ContactUploaderEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ) {
    try {
        m_authDelegate = authDelegate;
        m_deviceInfo = deviceInfo;

        m_authDelegate->addAuthObserver( shared_from_this() );

        m_contactUploaderRESTAgent = ContactUploaderRESTAgent::create( m_authDelegate, m_deviceInfo );
        ThrowIfNull( m_contactUploaderRESTAgent, "nullContactUploaderRESTAgent" );

        m_contactUploaderPlatformInterface->setEngineInterface( std::dynamic_pointer_cast<aace::contactUploader::ContactUploaderEngineInterface>( shared_from_this() ) );

        m_deleteAddressBookOnEngineStart = true;
        return true;

    }
    catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"initialize").d("reason", ex.what() ) );
        return false;
    }
}

std::shared_ptr<ContactUploaderEngineImpl> ContactUploaderEngineImpl::create(
    std::shared_ptr<aace::contactUploader::ContactUploader> contactUploaderPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo ) {

    try {
        ThrowIfNull( authDelegate, "nullAuthDelegateInterface" );

        std::shared_ptr<ContactUploaderEngineImpl> contactUploaderEngineImpl = std::shared_ptr<ContactUploaderEngineImpl>( new ContactUploaderEngineImpl( contactUploaderPlatformInterface ) );

        ThrowIfNot( contactUploaderEngineImpl->initialize( authDelegate, deviceInfo ), "initializeContactUploaderEngineImplFailed" );

        return contactUploaderEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"create").d("reason", ex.what() ) );
        return nullptr;
    }
}

void ContactUploaderEngineImpl::doShutdown() {
    m_executor.shutdown();
    if( m_contactUploaderPlatformInterface != nullptr ) {
        m_contactUploaderPlatformInterface->setEngineInterface( nullptr ); 
    }
}

// ContactUploaderEngineInterface
bool ContactUploaderEngineImpl::onAddContactsBegin() {
    try {
        ThrowIf( isUploadInProgress(), "uploadAlreadyInProgress");
        ThrowIf( isCancelInProgress(), "cancelInProgress");
        ThrowIf( isRemoveInProgress(), "removeInProgress");
        ThrowIfNot( m_isAuthRefreshed, "authTokenNotValid");

        if( !isPceIdValid() ) {
            // First time fetch pceId
            ThrowIfNot( fetchPceId(), "fetchPceIdFailed");  
        }

        ThrowIfNot( isPceIdValid(), "invalidPceId");

        auto sourceAddressBookId = m_deviceInfo->getDeviceSerialNumber();
        AACE_INFO( LX(TAG,"onAddContactsBegin").d("sourceAddressBookId", sourceAddressBookId ) );

        m_executor.waitForSubmittedTasks();

        // Remove any previous Address book.
        ThrowIfNot( deleteAddressBook( sourceAddressBookId ),"deleteAddressBookFailed" );

        ThrowIfNot( createAddressBook( sourceAddressBookId ),"addressBookCreateFailed" );

        emptyContactQueue(); //Empty the queue to remove any stale contants.
        emptyFailedContactListQueue(); //Empty the queue to remove any stale failed contacts.

        setContactUploadProgressState( ContactUploaderInternalState::START_TRIGGERED );

        return true;

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "onAddContactsBegin").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineImpl::onAddContactsEnd() {
    try {
        ThrowIfNot( isUploadInProgress(), "noUploadInProgress" );
        ThrowIf( isCancelInProgress(), "cancelInProgress");
        ThrowIf( isRemoveInProgress(), "removeInProgress");

        if( m_contactsQueue.size() > 0 ) { //Check for remaining contacts and send.
            std::vector<std::string> poppedContacts;

            while( !m_contactsQueue.empty() ) {
                poppedContacts.insert(poppedContacts.end(), m_contactsQueue.front() );
                m_contactsQueue.pop();
            }
            AACE_INFO( LX(TAG,"onAddContactsEnd").d( "uploadLastBatchOfContacts", poppedContacts.size() ) );

            m_executor.waitForSubmittedTasks();

            if( !isCancelInProgress() ) { // Check if Cancel is triggered
                m_executor.submit( [this, poppedContacts] {
                    notifyToStartAsyncUploadTask();
                    executeAsyncUploadContactsTask( poppedContacts, true );
                });
            }

        } else {
            AACE_INFO( LX(TAG,"onAddContactsEnd").m( "noPendingContactsToUpload" ) );
            m_executor.waitForSubmittedTasks();

            if( !isCancelInProgress() ) { // Check if Cancel is triggered
                auto response = m_contactUploaderRESTAgent->buildFailedContactsJson( m_failedContactQueue );
                contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_COMPLETED, response );
                setContactUploadProgressState( ContactUploaderInternalState::IDLE );
            }
        }

        return true;

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "onAddContactsEnd").d("reason", ex.what()));
        return false;
    }
}

bool ContactUploaderEngineImpl::onAddContactsCancel() {
    try {
        ThrowIfNot( isUploadInProgress(), "noUploadInProgress" );
        ThrowIf( isRemoveInProgress(), "removeInProgress" );

        if( getContactUploadProgressState() != ContactUploaderInternalState::START_TRIGGERED ) {
            AACE_INFO( LX(TAG,"onAddContactsCancel").m( "uploadingInProgress" ) );
            setContactUploadProgressState( ContactUploaderInternalState::CANCEL_TRIGGERED );

            m_executor.waitForSubmittedTasks();

            ThrowIfNot( deleteAddressBook( m_deviceInfo->getDeviceSerialNumber() ), "deleteAddressBookFailed" );

            contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_CANCELED, EMPTY_STRING );
            setContactUploadProgressState( ContactUploaderInternalState::IDLE );

        } else {
            AACE_INFO( LX(TAG,"onAddContactsCancel").m( "noUploadingInProgress" ) );
            contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_CANCELED, EMPTY_STRING );
            setContactUploadProgressState( ContactUploaderInternalState::IDLE );
        }

        return true;

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "onAddContactsCancel").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineImpl::onAddContact( const std::string& contact ) {
    try {
        ThrowIfNot( isUploadInProgress(), "noUploadInProgress" );
        ThrowIf( isCancelInProgress(), "cancelInProgress" );

        ThrowIfNot( isAddressBookIdValid(), "invalidAddressBookId");

        ThrowIfNot( validateContactJson( contact ), "validateContactJsonFailed");
        m_contactsQueue.push( contact );

        if( m_contactsQueue.size() == MAX_BATCH_SIZE ) {
            std::vector<std::string> poppedContacts;

            while( !m_contactsQueue.empty() ) {
                poppedContacts.insert(poppedContacts.end(), m_contactsQueue.front() );
                m_contactsQueue.pop();
            }
            AACE_INFO( LX(TAG,"onAddContact").m( "uploadingBatch" ) );

            //Wait for the previous upload to get finished if any.  
            m_executor.waitForSubmittedTasks();
            
            if( !isCancelInProgress() ) { // Check if Cancel is triggered.
                m_executor.submit( [this, poppedContacts] {
                    notifyToStartAsyncUploadTask();
                    executeAsyncUploadContactsTask( poppedContacts, false );
                });
            }
        }

        return true;

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "onAddContact").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineImpl::onRemoveUploadedContacts() {
    try {
        ThrowIf( isUploadInProgress(), "uploadInProgress" );
        ThrowIf( isRemoveInProgress(), "removeAlreadyInProgress" );
        ThrowIf( isCancelInProgress(), "cancelAlreadyInProgress" );
        ThrowIfNot( m_isAuthRefreshed, "authTokenNotValid");

        setContactUploadProgressState( ContactUploaderInternalState::REMOVE_TRIGGERED );

        if( !isPceIdValid() ) {
            ThrowIfNot( fetchPceId(), "fetchPceIdFailed");  
        }

        ThrowIfNot( isPceIdValid(), "invalidPceId");

        auto sourceAddressBookId = m_deviceInfo->getDeviceSerialNumber();
        AACE_INFO( LX(TAG,"onRemoveUploadedContacts").d( "removeContacts", sourceAddressBookId  ) );

        m_executor.submit( [this, sourceAddressBookId] {
            executeAsyncRemoveAddressBookTask( sourceAddressBookId );
        });

        return true;

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "onRemoveUploadedContacts").d("reason", ex.what() ) );
        return false;
    }
}

void ContactUploaderEngineImpl::emptyContactQueue() {
    while( !m_contactsQueue.empty() ) {
        m_contactsQueue.pop();
    }
}

void ContactUploaderEngineImpl::emptyFailedContactListQueue() {
    while( !m_failedContactQueue.empty() ) {
        m_failedContactQueue.pop();
    }
}

void ContactUploaderEngineImpl::executeAsyncUploadContactsTask( const std::vector<std::string>& poppedContacts, const bool finalBatch ) {
    HTTPResponse httpResponse;
    auto flowState = FlowState::POST;

    while( !isStopping() ) {
        auto nextFlowState = FlowState::FINISH;

        if( ( isCancelInProgress() || m_executor.isShutdown() ) && flowState != FlowState::FINISH ) {
            flowState = FlowState::FINISH;
        }

        AACE_DEBUG( LX(TAG,"executeAsyncUploadContactsTask").d( "flowState", flowState ) );
        switch( flowState ) {
            case FlowState::POST:
                nextFlowState = handleUploadContacts( poppedContacts, httpResponse );
                break;
            case FlowState::PARSE:
                nextFlowState = handleParse( httpResponse );
                break;
            case FlowState::NOTIFY:
                nextFlowState = handleNotification( finalBatch );
                break;
            case FlowState::ERROR:
                nextFlowState = handleError();
                contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_ERROR, EMPTY_STRING );
                break;
            case FlowState::FINISH:
                nextFlowState = handleStopping();
                break;
        }
        flowState = nextFlowState;
    }
}

ContactUploaderEngineImpl::FlowState ContactUploaderEngineImpl::handleStopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_isStopping = true;

    return FlowState::FINISH;
}

ContactUploaderEngineImpl::FlowState ContactUploaderEngineImpl::handleError() {
    try {
        emptyFailedContactListQueue();
        ThrowIfNot( deleteAddressBook( m_deviceInfo->getDeviceSerialNumber() ), "deleteAddressBookFailed" );

    } catch( std::exception &ex ) {
        AACE_ERROR( LX(TAG, "handleError").d("reason", ex.what() ) );
    }

    return FlowState::FINISH;
}

ContactUploaderEngineImpl::FlowState ContactUploaderEngineImpl::handleUploadContacts( const std::vector<std::string>& poppedContacts, HTTPResponse& httpResponse ) {
    try {
        httpResponse = m_contactUploaderRESTAgent->uploadContactToAddressBook( poppedContacts, getAddressBookId(), getPceId() );

        switch( httpResponse.code ) {
            case HTTPResponseCode::SUCCESS_OK:
                return FlowState::PARSE;
            case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
            case HTTPResponseCode::SUCCESS_NO_CONTENT:
            case HTTPResponseCode::REDIRECTION_START_CODE:
            case HTTPResponseCode::REDIRECTION_END_CODE:
            case HTTPResponseCode::BAD_REQUEST:  
            case HTTPResponseCode::FORBIDDEN:
            case HTTPResponseCode::SERVER_INTERNAL_ERROR:
                Throw( "handleUploadContactsFailed" + m_contactUploaderRESTAgent->getHTTPErrorString( httpResponse ) );
                break; 
        }
        return FlowState::ERROR;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleUploadContacts").d("reason", ex.what() ) );
        return FlowState::ERROR; //Some Exception, return ERROR
    }
}

ContactUploaderEngineImpl::FlowState ContactUploaderEngineImpl::handleParse( const HTTPResponse& httpResponse ) {
    try {
        ThrowIfNot( m_contactUploaderRESTAgent->parseCreateAddressBookEntryForFailedStatus( httpResponse, m_failedContactQueue ), "responseJsonParseFailed" );
        return FlowState::NOTIFY;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleParse").d("reason", ex.what() ) );
        return FlowState::FINISH;
    }
}

ContactUploaderEngineImpl::FlowState ContactUploaderEngineImpl::handleNotification( const bool finalBatch ) {
    if( !isCancelInProgress() ) {
        if( ContactUploaderInternalState::START_TRIGGERED == getContactUploadProgressState() ) {
            //Notify STARTED for the first batch upload
            contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_STARTED, EMPTY_STRING );
            setContactUploadProgressState( ContactUploaderInternalState::UPLOADING );
        } else if( ContactUploaderInternalState::UPLOADING == getContactUploadProgressState() ) {
            //Notify UPLOADLING for the subsequent batch uploads.
            contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_UPLOADING, EMPTY_STRING );
        }
        if( finalBatch ) {
            auto response = m_contactUploaderRESTAgent->buildFailedContactsJson( m_failedContactQueue );
            contactsUploaderStatusChanged( ContactUploaderStatus::UPLOAD_CONTACTS_COMPLETED, response );
            setContactUploadProgressState( ContactUploaderInternalState::IDLE );
        }
    }

    return( FlowState::FINISH );
}

bool ContactUploaderEngineImpl::isStopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isStopping;
}

void ContactUploaderEngineImpl::notifyToStartAsyncUploadTask() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_isStopping = false;
}

void ContactUploaderEngineImpl::contactsUploaderStatusChanged( ContactUploaderStatus status, const std::string& info ) {
    if( m_contactUploaderPlatformInterface != nullptr ) {
        return m_contactUploaderPlatformInterface->contactsUploaderStatusChanged( status, info );
    }
}

void ContactUploaderEngineImpl::onAuthStateChange( AuthObserverInterface::State newState, AuthObserverInterface::Error error ) {
    m_isAuthRefreshed = ( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED == newState );
    AACE_DEBUG( LX(TAG,"onAuthStateChange").d( "m_isAuthRefreshed", m_isAuthRefreshed ) );

    // Since Engine start, at the first callback of AuthObserverInterface::State::REFRESHED delete previous address book.
    if( m_deleteAddressBookOnEngineStart &&  m_isAuthRefreshed  ) {
        m_deleteAddressBookOnEngineStart = false;
        AACE_INFO( LX(TAG,"onAuthStateChange").m( "deletingAddressBookAtFirstAuthStateChange" ) );

        auto alexaAccount = m_contactUploaderRESTAgent->getAlexaAccountInfo();
        // Delete Address book to be done for only the provisioned/auto-provisioned accounts.
        if( alexaAccount.provisionStatus == CommsProvisionStatus::PROVISIONED || 
            alexaAccount.provisionStatus == CommsProvisionStatus::AUTO_PROVISIONED ) {
            auto sourceAddressBookId = m_deviceInfo->getDeviceSerialNumber();
            m_executor.waitForSubmittedTasks();
            m_executor.submit( [this, sourceAddressBookId] {
                try {
                    ThrowIfNot( fetchPceId(), "fetchPceIdFailed");
                    if( isPceIdValid() ) {
                        deleteAddressBook( sourceAddressBookId );
                    }
                    return;
                } catch( std::exception& ex ) {
                    AACE_ERROR(LX(TAG,"onAuthStateChange").d("reason", ex.what() ) );
                    return;
                }
            });
        }
    }
}

bool ContactUploaderEngineImpl::isUploadInProgress() {
    std::lock_guard<std::mutex> lock( m_mutex );
    if( ( m_contactUploadState == ContactUploaderInternalState::START_TRIGGERED ||
        m_contactUploadState == ContactUploaderInternalState::UPLOADING ) ) {
        return true;
    }
    return false;
}

bool ContactUploaderEngineImpl::isCancelInProgress() {
    std::lock_guard<std::mutex> lock( m_mutex );
    if( m_contactUploadState == ContactUploaderInternalState::CANCEL_TRIGGERED ) {
        return true;
    }
    return false;
}

bool ContactUploaderEngineImpl::isRemoveInProgress() {
    std::lock_guard<std::mutex> lock( m_mutex );
    if( m_contactUploadState == ContactUploaderInternalState::REMOVE_TRIGGERED ) {
        return true;
    }
    return false;
}

void ContactUploaderEngineImpl::setContactUploadProgressState( ContactUploaderInternalState state ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    m_contactUploadState = state;
}

ContactUploaderEngineImpl::ContactUploaderInternalState ContactUploaderEngineImpl::getContactUploadProgressState() {
    std::lock_guard<std::mutex> lock( m_mutex );
    return m_contactUploadState;
}

bool ContactUploaderEngineImpl::isPceIdValid() {
    return( !m_pceId.empty() );
}

void ContactUploaderEngineImpl::setPceId( const std::string& pceId ) {
    m_pceId = pceId;
}
void ContactUploaderEngineImpl::setAddressBookId( const std::string& addressBookId ) {
    m_addressBookId = addressBookId;
}

bool ContactUploaderEngineImpl::isAddressBookIdValid() {
    return( !m_addressBookId.empty() );
}

std::string ContactUploaderEngineImpl::getPceId() {
    return m_pceId;
}

std::string ContactUploaderEngineImpl::getAddressBookId() {
    return m_addressBookId;
}

bool ContactUploaderEngineImpl::fetchPceId() {
    try {
        auto alexaAccount = m_contactUploaderRESTAgent->getAlexaAccountInfo();
        ThrowIf( alexaAccount.provisionStatus == CommsProvisionStatus::INVALID, "getAlexaAccountDetailsFailed" );

        if( alexaAccount.provisionStatus == CommsProvisionStatus::UNKNOWN || alexaAccount.provisionStatus == CommsProvisionStatus::DEPROVISIONED ) {
            // Auto-provision the account
            ThrowIfNot( m_contactUploaderRESTAgent->doAccountAutoProvision( alexaAccount.directedId ), "AccountAutoPrvovisionFailed" );
            alexaAccount = m_contactUploaderRESTAgent->getAlexaAccountInfo(); // get commsId after auto provisioning.
            ThrowIf( alexaAccount.commsId.empty(), "commsIdEmptyAfterProvisioning" );
        }

        ThrowIf( alexaAccount.commsId.empty(), "commsIdEmptyForProvisionedAccount" );

        auto pceId = m_contactUploaderRESTAgent->getPceId( alexaAccount.commsId );
        ThrowIf( pceId.empty(), "pceIdEmptyForProvisionedAccount" );

        setPceId( pceId );

        return true;

    } catch( std::exception& ex ) {
        AACE_ERROR( LX(TAG,"fetchPceId").d("reason", ex.what()));
        return false;
    }
}

bool ContactUploaderEngineImpl::createAddressBook( const std::string& sourceAddressBookId ) {
    try {
        auto addressBookId = m_contactUploaderRESTAgent->createAndGetAddressBookId( sourceAddressBookId, getPceId() );

        if( addressBookId.empty() ) {
            Throw( "addressBookIdEmpty" );
        }
        setAddressBookId( addressBookId );

        return true;

    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"createAddressBook").d("reason", ex.what()));
        return false;
    }
}

void ContactUploaderEngineImpl::executeAsyncRemoveAddressBookTask( const std::string& sourceAddressBookId ) {

    contactsUploaderStatusChanged( ContactUploaderStatus::REMOVE_CONTACTS_STARTED, EMPTY_STRING );
    try {
        auto addressBookId = m_contactUploaderRESTAgent->getAddressBookId( sourceAddressBookId, getPceId() );

        if( !addressBookId.empty() ) {
            ThrowIfNot( m_contactUploaderRESTAgent->deleteAddressBookId( addressBookId, getPceId() ), "deleteAddressBookIdFailed" ) ;
        }
        contactsUploaderStatusChanged( ContactUploaderStatus::REMOVE_CONTACTS_COMPLETED, EMPTY_STRING );

    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executeRemoveAddressBook").d("reason", ex.what() ) );
        emptyFailedContactListQueue();
        contactsUploaderStatusChanged( ContactUploaderStatus::REMOVE_CONTACTS_ERROR, EMPTY_STRING );
    }
    setContactUploadProgressState( ContactUploaderInternalState::IDLE );
}

bool ContactUploaderEngineImpl::deleteAddressBook( const std::string& sourceAddressBookId ) {
    try {
        auto addressBookId = m_contactUploaderRESTAgent->getAddressBookId( sourceAddressBookId, getPceId() );

        if( !addressBookId.empty() ) {
            ThrowIfNot( m_contactUploaderRESTAgent->deleteAddressBookId( addressBookId, getPceId() ), "deleteAddressBookIdFailed" ) ;
        }
        return true;

    } catch( std::exception& ex ) {
        AACE_ERROR(LX( TAG,"deleteAddressBook" ).d( "reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineImpl::validateContactJson( const std::string& contact ) {
    rapidjson::Document document;
    try { 
        if( document.Parse( contact.c_str()).HasParseError() ) {
            Throw( "parseError" );
        }
        if( document[ "id" ].IsNull() || document[ "id" ].GetStringLength() == 0 ) {
            Throw( "contactIdNotValid" );
        }

        if( document.HasMember( "firstName" ) && document[ "firstName" ].GetStringLength() > MAX_ALLOWED_CHARACTERS ) {
            Throw( "firstNameNotValid" );
        }

        if( document.HasMember( "lastName" ) && document[ "lastName" ].GetStringLength() > MAX_ALLOWED_CHARACTERS ) {
            Throw( "lastNameNotValid" );
        }

        if( document.HasMember(  "nickName" ) && document[ "nickName" ].GetStringLength() > MAX_ALLOWED_CHARACTERS ) {
            Throw( "nickNameNotValid" );
        }

        if( document.HasMember(  "company" ) && document[ "company" ].GetStringLength() > MAX_ALLOWED_CHARACTERS ) {
            Throw( "companyNotValid" );
        }

        auto addressess = document.FindMember( "addresses" );
        if( addressess == document.MemberEnd() ) {
            Throw( "noAddressesField" );
        } else if( !addressess->value.IsArray() ) {
            Throw( "addressesFieldNotArray" );
        } else if( addressess->value.Size() == 0 ) {
            Throw( "emptyAddressesField" );
        } else if( addressess->value.Size() ) {
            int phonenumberCount = 0;
            for( rapidjson::Value::ConstValueIterator itr = addressess->value.Begin(); itr != addressess->value.End(); itr++ ) {
                if( (*itr).HasMember( "type" ) && (*itr)[ "type" ].IsString() && strcmp( "phonenumber", (*itr)[ "type" ].GetString() ) == 0 ) {
                    if( (*itr).HasMember( "value" ) && (*itr)[ "value" ].IsString() && ( (*itr)[ "value" ].GetStringLength() > 0 && (*itr)[ "value" ].GetStringLength() <= MAX_ALLOWED_PHONENUMBER_CHARACTERS ) )  {
                        phonenumberCount++;
                    } else {
                        Throw( "phoneNumberValueNotValid" );
                    }
                } else {
                    Throw( "phoneNumberFormatInvalid" );
                }
            }
            if( phonenumberCount > MAX_ALLOWED_PHONENUMBER_PER_CONTACT ) {
                Throw( "phoneNumberMaxSize" );
            }
        }

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX( TAG,"validateContactJson" ).d( "reason", ex.what() ) );
        return false;
    }
}

} // aace::engine::contactUploader
} // aace::engine
} // aace
