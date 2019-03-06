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

#ifndef AACE_ENGINE_CONTACTUPLOADER_CONTACTUPLOADER_ENGINE_IMPL_H
#define AACE_ENGINE_CONTACTUPLOADER_CONTACTUPLOADER_ENGINE_IMPL_H

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <queue>
#include <chrono>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include <AVSCommon/Utils/DeviceInfo.h>

#include <AACE/ContactUploader/ContactUploader.h>
#include <AACE/ContactUploader/ContactUploaderEngineInterface.h>
#include "ContactUploaderRESTAgent.h"

namespace aace {
namespace engine {
namespace contactUploader {

class ContactUploaderEngineImpl : 
    public aace::contactUploader::ContactUploaderEngineInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface,
    public std::enable_shared_from_this<ContactUploaderEngineImpl> {

private:
    ContactUploaderEngineImpl( std::shared_ptr<aace::contactUploader::ContactUploader> contactUploaderPlatformInterface );

    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo
    );

public:
    static std::shared_ptr<ContactUploaderEngineImpl> create(
        std::shared_ptr<aace::contactUploader::ContactUploader> contactUploaderPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo );
    
    using HTTPResponse = ContactUploaderRESTAgent::HTTPResponse;
    using AlexaAccountInfo = ContactUploaderRESTAgent::AlexaAccountInfo;
    using CommsProvisionStatus = ContactUploaderRESTAgent::CommsProvisionStatus;

    // ContactUploaderEngineInterface
    bool onAddContactsBegin() override;
    bool onAddContactsEnd() override;
    bool onAddContactsCancel() override;
    bool onAddContact( const std::string& contact ) override;
    bool onRemoveUploadedContacts() override;

    using ContactUploaderStatus = aace::contactUploader::ContactUploaderEngineInterface::ContactUploaderStatus;

    void contactsUploaderStatusChanged( ContactUploaderStatus status, const std::string& info );

    // AuthObserverInterface
    void onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error ) override;

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::contactUploader::ContactUploader> m_contactUploaderPlatformInterface;
    std::shared_ptr<ContactUploaderRESTAgent> m_contactUploaderRESTAgent;

    bool fetchPceId();
    bool createAddressBook( const std::string& sourceAddressBookId );
    bool deleteAddressBook( const std::string& sourceAddressBookId );
    bool validateContactJson( const std::string& contact );

    void emptyContactQueue();
    void emptyFailedContactListQueue();
    void executeAsyncUploadContactsTask( const std::vector<std::string>& poppedContacts, const bool finalBatch );
    void executeAsyncRemoveAddressBookTask( const std::string& sourceAddressBookId );

    bool isPceIdValid();
    std::string getPceId();
    void setPceId( const std::string& pceId );

    bool isAddressBookIdValid();
    std::string getAddressBookId();
    void setAddressBookId( const std::string& pceId );

   enum class ContactUploaderInternalState {
        IDLE,
        START_TRIGGERED,
        UPLOADING,
        CANCEL_TRIGGERED,
        REMOVE_TRIGGERED   
    };
    ContactUploaderInternalState m_contactUploadState;

    bool isUploadInProgress();
    bool isCancelInProgress();
    bool isRemoveInProgress();
    void setContactUploadProgressState( ContactUploaderInternalState state );
    ContactUploaderInternalState getContactUploadProgressState();

    friend std::ostream& operator<<(std::ostream& stream, const ContactUploaderInternalState& state);

    bool isStopping();
    void notifyToStartAsyncUploadTask();
    void resetCanceledState();

    enum class FlowState {
        POST,
        PARSE,
        NOTIFY,
        ERROR,
        FINISH
    };

    FlowState handleUploadContacts( const std::vector<std::string>& poppedContacts, HTTPResponse& httpResponse );
    FlowState handleParse( const HTTPResponse& httpResponse );
    FlowState handleNotification( const bool finalBatch );
    FlowState handleError();
    FlowState handleStopping();

    friend std::ostream& operator<<(std::ostream& stream, const FlowState& state);

    std::string m_pceId;
    std::string m_addressBookId;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;

    /// Contacts Queue
    std::queue<std::string> m_contactsQueue;

    /// Whether or not excector thread is stopping.
    bool m_isStopping;

    /// Mutex used to serialize access to various data members.
    std::mutex m_mutex;

    /// Failed contacts queue
    std::queue<std::string> m_failedContactQueue;

    /// Auth Token refresh state.
    bool m_isAuthRefreshed;

    /// Flag to delete the address book on first start of engine.
    bool m_deleteAddressBookOnEngineStart;

    /**
     * @note This declaration needs to come *after* the Executor Thread Variables above so that the thread shuts down
     *     before the Executor Thread Variables are destroyed.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

inline std::ostream& operator<<(std::ostream& stream, const ContactUploaderEngineImpl::FlowState& state) {
    switch( state ) {
        case ContactUploaderEngineImpl::FlowState::POST:
            stream << "POST";
            break;
        case ContactUploaderEngineImpl::FlowState::PARSE:
            stream << "PARSE";
            break;
        case ContactUploaderEngineImpl::FlowState::NOTIFY:
            stream << "NOTIFY";
            break;
        case ContactUploaderEngineImpl::FlowState::ERROR:
            stream << "ERROR";
            break;
        case ContactUploaderEngineImpl::FlowState::FINISH:
            stream << "FINISH";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const ContactUploaderEngineImpl::ContactUploaderInternalState& state) {
    switch( state ) {
        case ContactUploaderEngineImpl::ContactUploaderInternalState::IDLE:
            stream << "IDLE";
            break;
        case ContactUploaderEngineImpl::ContactUploaderInternalState::START_TRIGGERED:
            stream << "START_TRIGGERED";
            break;
        case ContactUploaderEngineImpl::ContactUploaderInternalState::UPLOADING:
            stream << "UPLOADING";
            break;
        case ContactUploaderEngineImpl::ContactUploaderInternalState::CANCEL_TRIGGERED:
            stream << "CANCEL_TRIGGERED";
            break;
        case ContactUploaderEngineImpl::ContactUploaderInternalState::REMOVE_TRIGGERED:
            stream << "REMOVE_TRIGGERED";
            break;
    }
    return stream;
}

} // aace::engine::contactUploader
} // aace::engine
} // aace

#endif
