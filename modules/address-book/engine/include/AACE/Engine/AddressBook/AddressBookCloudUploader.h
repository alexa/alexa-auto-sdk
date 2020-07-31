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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_CLOUD_UPLOADER_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_CLOUD_UPLOADER_H

#include <atomic>
#include <thread>
#include <deque>
#include <unordered_map>
#include <condition_variable>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/DeviceInfo.h>

#include <AACE/Network/NetworkInfoProvider.h>
#include <AACE/Engine/Network/NetworkInfoObserver.h>
#include <AACE/Engine/Network/NetworkObservableInterface.h>
#include <AACE/Engine/Metrics/MetricEvent.h>

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
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        m_createTimeMs = now_ms.count();
    }

    Type getType() {
        return m_type;
    }

    std::shared_ptr<AddressBookEntity> getAddressBookEntity() {
        return m_addressBookEntity;
    }

    int getRetryCount() {
        return m_retryCounter;
    }

    void incrementRetryCount() {
        m_retryCounter += 1;
    }

    double getEventCreateTime() {
        return m_createTimeMs;
    }

private:
    //Event Type
    Type m_type;

    // Network Retry Counter
    int m_retryCounter;

    // AddressBookEntity
    std::shared_ptr<AddressBookEntity> m_addressBookEntity;

    // For metrics
    double m_createTimeMs;
};

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
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

public:
    static std::shared_ptr<AddressBookCloudUploader> create(
        std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        NetworkInfoObserver::NetworkStatus networkStatus,
        std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

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

    void eventLoop();  // Infinite loop
    const Event popNextEventFromQ();

    bool handleUpload(std::shared_ptr<AddressBookEntity> addressBookEntity);
    bool handleRemove(std::shared_ptr<AddressBookEntity> addressBookEntity);

    bool checkAndAutoProvisionAccount();
    std::string prepareForUpload(std::shared_ptr<AddressBookEntity> addressBookEntity);
    bool upload(const std::string& cloudAddressBookId, std::shared_ptr<rapidjson::Document>);
    bool uploadEntries(const std::string& cloudAddressBookId, std::shared_ptr<rapidjson::Document> document);

    std::string createAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity);
    bool deleteAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity);

    bool cleanAllCloudAddressBooks();

    bool isEventEnqueuedLocked(Event::Type type, std::shared_ptr<AddressBookEntity> addressBookEntity);
    void removeMatchingAddEventFromQueueLocked(std::shared_ptr<AddressBookEntity> addressBookEntity);

    enum class UploadFlowState { POST, PARSE, ERROR, FINISH };

    UploadFlowState handleUploadEntries(
        const std::string& addressBookId,
        std::shared_ptr<rapidjson::Document> document,
        HTTPResponse& httpResponse);
    UploadFlowState handleParseHTTPResponse(const HTTPResponse& httpResponse);
    UploadFlowState handleError(const std::string& addressBookId);

    void logNetworkMetrics(const HTTPResponse& httpResponse);

    void emitCounterMetrics(const std::string& methodName, const std::string& key, const int value);
    void emitTimerMetrics(const std::string& methodName, const std::string& key, const double value);
    double getCurrentTimeInMs();

    friend std::ostream& operator<<(std::ostream& stream, const UploadFlowState& state);

private:
    /// Serialize access to address book event queue
    std::mutex m_mutex;

    /// reference to AddressBookService
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> m_addressBookService;

    // Mapping source address book id to AddressBookEntity.
    std::unordered_map<std::string, std::shared_ptr<AddressBookEntity>> m_addressBooks;

    /// Indicates whether the internal main loop should keep running.
    std::atomic<bool> m_isShuttingDown;

    /// Condition Variable to wait on external actions
    std::condition_variable m_waitStatusChange;

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
