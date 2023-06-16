/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_CLOUD_UPLOADER_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_CLOUD_UPLOADER_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/HTTP/HttpResponseCode.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/DeviceInfo.h>

#include <AACE/Network/NetworkInfoProvider.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>
#include <AACE/Engine/Network/NetworkInfoObserver.h>
#include <AACE/Engine/Network/NetworkObservableInterface.h>

#include "AddressBookObserver.h"
#include "AddressBookServiceInterface.h"
#include "AddressBookCloudUploaderRESTAgent.h"

namespace aace {
namespace engine {
namespace addressBook {

using AddressBookEntity = aace::engine::addressBook::AddressBookEntity;

class Event {
public:
    enum class Type { INVALID, ADD, REMOVE };

    static const Event& INVALID() {
        static Event e(Event::Type::INVALID, nullptr);
        return e;
    }

    Event(Type type, std::shared_ptr<AddressBookEntity> addressBookEntity) :
            m_type(type), m_retryCounter(0), m_addressBookEntity(std::move(addressBookEntity)) {
        // Cache the create time for metrics
        m_createTime = std::chrono::system_clock::now();
    }

    Type getType() {
        return m_type;
    }

    std::shared_ptr<AddressBookEntity> getAddressBookEntity() {
        return m_addressBookEntity;
    }

    unsigned int getRetryCount() {
        return m_retryCounter;
    }

    void incrementRetryCount() {
        m_retryCounter += 1;
    }

    std::chrono::system_clock::time_point getEventCreateTime() {
        return m_createTime;
    }

private:
    //Event Type
    Type m_type;

    // Network Retry Counter
    unsigned int m_retryCounter;

    // AddressBookEntity
    std::shared_ptr<AddressBookEntity> m_addressBookEntity;

    // Creation time
    std::chrono::system_clock::time_point m_createTime;
};

enum class AddressBookOperationResultCode {
    SUCCESS,
    ERROR_ACCOUNT_NOT_PROVISIONED,
    ERROR_DELETE_ADDRESS_BOOK_FAILED,
    ERROR_CREATE_ADDRESS_BOOK_FAILED,
    ERROR_HTTP_PARSE_RESPONSE_FAILED,
    ERROR_HTTP_RESPONSE_MISSING_CONTENT,
    ERROR_HTTP_UNHANDLED_REDIRECTION_RESPONSE,
    ERROR_HTTP_BAD_REQUEST,
    ERROR_HTTP_FORBIDDEN,
    ERROR_HTTP_THROTTLED,
    ERROR_HTTP_SERVER_ERROR,
    ERROR_UNKNOWN
};

inline std::ostream& operator<<(std::ostream& stream, const AddressBookOperationResultCode& code) {
    switch (code) {
        case AddressBookOperationResultCode::SUCCESS:
            stream << "SUCCESS";
            break;
        case AddressBookOperationResultCode::ERROR_ACCOUNT_NOT_PROVISIONED:
            stream << "ERROR_ACCOUNT_NOT_PROVISIONED";
            break;
        case AddressBookOperationResultCode::ERROR_DELETE_ADDRESS_BOOK_FAILED:
            stream << "ERROR_DELETE_ADDRESS_BOOK_FAILED";
            break;
        case AddressBookOperationResultCode::ERROR_CREATE_ADDRESS_BOOK_FAILED:
            stream << "ERROR_CREATE_ADDRESS_BOOK_FAILED";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_PARSE_RESPONSE_FAILED:
            stream << "ERROR_HTTP_PARSE_RESPONSE_FAILED";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_RESPONSE_MISSING_CONTENT:
            stream << "ERROR_HTTP_RESPONSE_MISSING_CONTENT";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_UNHANDLED_REDIRECTION_RESPONSE:
            stream << "ERROR_HTTP_UNHANDLED_REDIRECTION_RESPONSE";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_BAD_REQUEST:
            stream << "ERROR_HTTP_BAD_REQUEST";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_FORBIDDEN:
            stream << "ERROR_HTTP_FORBIDDEN";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_THROTTLED:
            stream << "ERROR_HTTP_THROTTLED";
            break;
        case AddressBookOperationResultCode::ERROR_HTTP_SERVER_ERROR:
            stream << "ERROR_HTTP_SERVER_ERROR";
            break;
        case AddressBookOperationResultCode::ERROR_UNKNOWN:
            stream << "ERROR_UNKNOWN";
            break;
    }
    return stream;
}

class AddressBookCloudUploader
        : public aace::engine::addressBook::AddressBookObserver
        , public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public aace::engine::network::NetworkInfoObserver
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AddressBookCloudUploader> {
private:
    AddressBookCloudUploader();

    bool initialize(
        std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        NetworkInfoObserver::NetworkStatus networkStatus,
        std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder,
        bool cleanAllAddressBooksAtStart);

public:
    static std::shared_ptr<AddressBookCloudUploader> create(
        std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        NetworkInfoObserver::NetworkStatus networkStatus,
        std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder,
        bool cleanAllAddressBooksAtStart);

    // AddressBookObserver
    bool addressBookAdded(std::shared_ptr<AddressBookEntity> addressBookEntity) override;
    bool addressBookRemoved(std::shared_ptr<AddressBookEntity> addressBookEntity) override;

    // AuthObserverInterface
    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;

    // aace::engine::network::NetworkInfoObserver
    void onNetworkInfoChanged(NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength) override;
    void onNetworkInterfaceChangeStatusChanged(
        const std::string& networkInterface,
        NetworkInfoObserver::NetworkInterfaceChangeStatus status) override;

protected:
    // RequiresShutdown
    void doShutdown() override;

private:
    using HTTPResponse = AddressBookCloudUploaderRESTAgent::HTTPResponse;
    using HTTPResponseCode = alexaClientSDK::avsCommon::utils::http::HTTPResponseCode;

    void eventLoop(bool cleanAllAddressBooksAtStart);  // Infinite loop
    const Event popNextEventFromQ();

    /**
     * Upload the specified address book
     * @param [in] addressBookEntity Address book to upload
     * @param [out] numBatches int reference to update with the number of
     *        batches it took to complete the upload attempt
     * @param [out] result AddressBookOperationResultCode reference to update
     *        with result of the upload
     * @return @c true if the upload was successful, false otherwise
     */
    bool handleUpload(
        std::shared_ptr<AddressBookEntity> addressBookEntity,
        int& numBatches,
        AddressBookOperationResultCode& result);
    bool handleRemove(std::shared_ptr<AddressBookEntity> addressBookEntity, AddressBookOperationResultCode& result);

    bool checkAndAutoProvisionAccount();
    AddressBookOperationResultCode prepareForUpload(
        std::shared_ptr<AddressBookEntity> addressBookEntity,
        std::string& cloudAddressBookId);
    AddressBookOperationResultCode uploadEntries(
        const std::string& cloudAddressBookId,
        std::shared_ptr<rapidjson::Document> document);

    std::string createAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity);
    bool deleteAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity);

    bool isEventEnqueuedLocked(Event::Type type, std::shared_ptr<AddressBookEntity> addressBookEntity);
    void removeMatchingAddEventFromQueueLocked(std::shared_ptr<AddressBookEntity> addressBookEntity);

    enum class UploadFlowState { POST, PARSE, ERROR, FINISH };

    UploadFlowState handleUploadEntries(
        const std::string& addressBookId,
        std::shared_ptr<rapidjson::Document> document,
        HTTPResponse& httpResponse);
    UploadFlowState handleParseHTTPResponse(const HTTPResponse& httpResponse);
    UploadFlowState handleError(const std::string& addressBookId);

    static AddressBookOperationResultCode httpResponseCodeToResult(HTTPResponseCode code);

    /// Get AddressBookOperationResultCode as a string for metrics dimensions
    static std::string getResultString(const AddressBookOperationResultCode& result);

    /**
     * Records latency, success count, and error count metrics with various
     * dimensions for the result of address book requests. Uses the default
     * context for @c AddressBookCloudUploader.
     */
    static void submitAddressBookRequestResultMetrics(
        const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& recorder,
        Event::Type eventType,
        std::chrono::milliseconds latency,
        unsigned int retryCount,
        unsigned int numBatchesInRequest,
        AddressBookType addressBookType,
        AddressBookOperationResultCode result);

    friend std::ostream& operator<<(std::ostream& stream, const UploadFlowState& state);

private:
    /// Serialize access to member variables  m_authDelegate, m_networkStatus and m_addressBookEventQ
    std::mutex m_mutex;

    /// reference to AddressBookService
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> m_addressBookService;

    /// Mapping source address book id to AddressBookEntity
    std::unordered_map<std::string, std::shared_ptr<AddressBookEntity>> m_addressBooks;

    /// The metric recorder
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> m_metricRecorder;

    /// Indicates whether the internal main loop should keep running.
    std::atomic<bool> m_isShuttingDown;

    /// Condition Variable to wait on external events
    std::condition_variable m_waitForEvent;

    /// Queue holding the address book events.
    std::deque<Event> m_addressBookEventQ;

    std::shared_ptr<aace::engine::network::NetworkObservableInterface> m_networkObserver;
    std::shared_ptr<AddressBookCloudUploaderRESTAgent> m_addressBookCloudUploaderRESTAgent;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;

    bool m_isAuthRefreshed = false;
    NetworkInfoObserver::NetworkStatus m_networkStatus;

    std::thread m_eventThread;
};

inline std::ostream& operator<<(std::ostream& stream, const AddressBookCloudUploader::UploadFlowState& state) {
    switch (state) {
        case AddressBookCloudUploader::UploadFlowState::POST:
            stream << "POST";
            break;
        case AddressBookCloudUploader::UploadFlowState::PARSE:
            stream << "PARSE";
            break;
        case AddressBookCloudUploader::UploadFlowState::ERROR:
            stream << "ERROR";
            break;
        case AddressBookCloudUploader::UploadFlowState::FINISH:
            stream << "FINISH";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const Event::Type& type) {
    switch (type) {
        case Event::Type::INVALID:
            stream << "INVALID";
            break;
        case Event::Type::ADD:
            stream << "ADD";
            break;
        case Event::Type::REMOVE:
            stream << "REMOVE";
            break;
    }
    return stream;
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_CLOUD_UPLOADER_H
