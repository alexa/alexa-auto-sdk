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

#include <typeinfo>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Network/NetworkEngineService.h>
#include <AACE/Engine/AddressBook/AddressBookCloudUploader.h>

namespace aace {
namespace engine {
namespace addressBook {

// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBook.addressBookCloudUploader");

/// Upload entreis batch size
static const int UPLOAD_BATCH_SIZE = 100;

/// Max allowed phonenumbers per entry
static const int MAX_ALLOWED_ADDRESSES_PER_ENTRY = 30;

/// Max allowed characters
static const int MAX_ALLOWED_CHARACTERS = 1000;

/// Max allowed EntryId size
static const int MAX_ALLOWED_ENTRY_ID_SIZE = 200;

/// Max event retry
static const int MAX_EVENT_RETRY = 3;

/// Invalid Address Id
static const std::string INVALID_ADDRESS_BOOK_SOURCE_ID = "INVALID";

/// Program Name prefix for metrics
static const std::string METRIC_PROGRAM_NAME_PREFIX = "AlexaAuto";

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AddressBookCloudUploader";

/// Delimiter
static const std::string DELIMITER = "_";

/// Program Name for metrics
static const std::string METRIC_PROGRAM_NAME = METRIC_PROGRAM_NAME_PREFIX + DELIMITER + METRIC_PROGRAM_NAME_SUFFIX;

/// Metric for adding contact
static const std::string METRIC_ADD_ADDRESS_BOOK_CONTACT = "Add.Contact";

/// Count metric for adding navigation
static const std::string METRIC_ADD_ADDRESS_BOOK_NAVIGATION = "Add.Navigation";

/// Count metric for removing contact
static const std::string METRIC_REMOVE_ADDRESS_BOOK_CONTACT = "Remove.Contact";

/// Count metric for removing navigation address
static const std::string METRIC_REMOVE_ADDRESS_BOOK_NAVIGATION = "Remove.Navigation";

/// Latency metrics to track actual upload to cloud since add event
static const std::string METRIC_TIME_TO_UPLOAD_SINCE_ADD = "Latency.Add";

/// Latency metrics to track actual remove from cloud since remove event
static const std::string METRIC_TIME_TO_UPLOAD_SINCE_REMOVE = "Latency.Remove";

/// Latency metric uploading a one batch of entries to cloud (average time)
static const std::string METRIC_TIME_TO_UPLOAD_ONE_BATCH = "Network.BatchUploadLatency";

/// Metric for Bad User Input network response
static const std::string METRIC_NETWORK_BAD_USER_INPUT = "Network.BadUserInput";

/// Metric for any Network Error
static const std::string METRIC_NETWORK_ERROR = "Network.Error";

AddressBookCloudUploader::AddressBookCloudUploader() :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), m_isShuttingDown(false), m_isAuthRefreshed(false) {
}

std::shared_ptr<AddressBookCloudUploader> AddressBookCloudUploader::create(
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    NetworkInfoObserver::NetworkStatus networkStatus,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        auto addressBookCloudUploader = std::shared_ptr<AddressBookCloudUploader>(new AddressBookCloudUploader());
        ThrowIfNot(
            addressBookCloudUploader->initialize(
                addressBookService, authDelegate, deviceInfo, networkStatus, networkObserver, alexaEndpoints),
            "initializeAddressBookCloudUploaderFailed");

        return addressBookCloudUploader;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool AddressBookCloudUploader::initialize(
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    NetworkInfoObserver::NetworkStatus networkStatus,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        m_addressBookService = addressBookService;
        m_authDelegate = authDelegate;
        m_deviceInfo = deviceInfo;
        m_networkStatus = networkStatus;
        m_networkObserver = networkObserver;

        m_addressBookCloudUploaderRESTAgent = aace::engine::addressBook::AddressBookCloudUploaderRESTAgent::create(
            authDelegate, m_deviceInfo, alexaEndpoints);
        ThrowIfNull(m_addressBookCloudUploaderRESTAgent, "createAddressBookCloudRESTAgentFailed");

        m_authDelegate->addAuthObserver(shared_from_this());
        if (m_networkObserver != nullptr) {  // This could be null when NetworkInfoProvider interface is not registered.
            m_networkObserver->addObserver(shared_from_this());
        } else {
            AACE_DEBUG(LX(TAG).m("networkObserverNotAvailable"));
        }
        m_addressBookService->addObserver(shared_from_this());

        // Infinite event loop
        m_eventThread = std::thread{&AddressBookCloudUploader::eventLoop, this};

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

void AddressBookCloudUploader::doShutdown() {
    m_isShuttingDown = true;
    m_waitStatusChange.notify_all();

    if (m_eventThread.joinable()) {
        m_eventThread.join();
    }
    m_authDelegate->removeAuthObserver(shared_from_this());
    if (m_networkObserver != nullptr) {
        m_networkObserver->removeObserver(shared_from_this());
        m_networkObserver.reset();
    }
    m_addressBookService->removeObserver(shared_from_this());
    m_authDelegate.reset();
    m_addressBookService.reset();
    m_addressBookCloudUploaderRESTAgent.reset();
}

void AddressBookCloudUploader::onAuthStateChange(
    AuthObserverInterface::State newState,
    AuthObserverInterface::Error error) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_isAuthRefreshed = (AuthObserverInterface::State::REFRESHED == newState);
    AACE_DEBUG(LX(TAG).d("m_isAuthRefreshed", m_isAuthRefreshed));
    switch (newState) {
        case AuthObserverInterface::State::UNINITIALIZED:
        case AuthObserverInterface::State::UNRECOVERABLE_ERROR:
            m_addressBookEventQ.clear();
            m_addressBookCloudUploaderRESTAgent->reset();
            break;
        case AuthObserverInterface::State::REFRESHED:
            m_waitStatusChange.notify_all();
            break;
        case AuthObserverInterface::State::EXPIRED:
        default:
            break;
    }
}

void AddressBookCloudUploader::onNetworkInfoChanged(NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_networkStatus != status) {
        m_networkStatus = status;
        AACE_DEBUG(LX(TAG, "onNetworkInfoChanged").d("m_networkStatus", m_networkStatus));
        if (status == NetworkInfoObserver::NetworkStatus::CONNECTED) {
            m_waitStatusChange.notify_all();
        }
    }
}

void AddressBookCloudUploader::onNetworkInterfaceChangeStatusChanged(
    const std::string& networkInterface,
    NetworkInterfaceChangeStatus status) {
    // No action required as we don't create and persist network connection using AVS LibCurlUtils.
}

bool AddressBookCloudUploader::addressBookAdded(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    std::string addressBookSourceId = INVALID_ADDRESS_BOOK_SOURCE_ID;
    try {
        ThrowIfNull(addressBookEntity, "invalidAddressBookEntity");
        addressBookSourceId = addressBookEntity->getSourceId();

        // For Metrics
        if (addressBookEntity->getType() == AddressBookType::CONTACT) {
            emitCounterMetrics("addressBookAdded", METRIC_ADD_ADDRESS_BOOK_CONTACT, 1);
        } else if (addressBookEntity->getType() == AddressBookType::NAVIGATION) {
            emitCounterMetrics("addressBookAdded", METRIC_ADD_ADDRESS_BOOK_NAVIGATION, 1);
        }

        std::lock_guard<std::mutex> guard(m_mutex);
        m_addressBookEventQ.emplace_back(Event::Type::ADD, addressBookEntity);

        m_waitStatusChange.notify_all();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "addressBookAdded").d("addressBookSourceId", addressBookSourceId).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploader::addressBookRemoved(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    std::string addressBookSourceId = INVALID_ADDRESS_BOOK_SOURCE_ID;
    try {
        ThrowIfNull(addressBookEntity, "invalidAddressBook");
        addressBookSourceId = addressBookEntity->getSourceId();

        // For Metrics
        if (addressBookEntity->getType() == AddressBookType::CONTACT) {
            emitCounterMetrics("addressBookRemoved", METRIC_REMOVE_ADDRESS_BOOK_CONTACT, 1);
        } else if (addressBookEntity->getType() == AddressBookType::NAVIGATION) {
            emitCounterMetrics("addressBookRemoved", METRIC_REMOVE_ADDRESS_BOOK_NAVIGATION, 1);
        }

        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_addressBookEventQ.size() > 0) {
            if (isEventEnqueuedLocked(Event::Type::ADD, addressBookEntity)) {
                AACE_INFO(LX(TAG, "removeAddressBook")
                              .m("removingCorrespondingAddEvent")
                              .d("addressBookSourceId", addressBookSourceId));
                removeMatchingAddEventFromQueueLocked(addressBookEntity);
                return true;
            }
        }

        m_addressBookEventQ.emplace_back(Event::Type::REMOVE, addressBookEntity);

        m_waitStatusChange.notify_all();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "addressBookRemoved").d("addressBookSourceId", addressBookSourceId).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploader::isEventEnqueuedLocked(
    aace::engine::addressBook::Event::Type type,
    std::shared_ptr<AddressBookEntity> addressBookEntity) {
    for (auto&& it : m_addressBookEventQ) {
        if (it.getType() == type) {
            if (it.getAddressBookEntity()->getSourceId() == addressBookEntity->getSourceId()) {
                return true;
            }
        }
    }
    return false;
}

void AddressBookCloudUploader::removeMatchingAddEventFromQueueLocked(
    std::shared_ptr<AddressBookEntity> addressBookEntity) {
    for (auto it = m_addressBookEventQ.begin(); it != m_addressBookEventQ.end(); it++) {
        if (it->getType() == Event::Type::ADD) {
            if (it->getAddressBookEntity()->getSourceId() == addressBookEntity->getSourceId()) {
                m_addressBookEventQ.erase(it);
                return;
            }
        }
    }
}

class AddressBookEntriesFactory : public aace::addressBook::AddressBook::IAddressBookEntriesFactory {
public:
    AddressBookEntriesFactory(
        std::shared_ptr<AddressBookEntity> addressBookEntity,
        std::vector<std::shared_ptr<rapidjson::Document>>& documents) :
            m_addressBookEntity(std::move(addressBookEntity)), m_documents(documents) {
    }

private:
    bool isEntryPresent(const std::string& entryId) {
        auto it = m_ids.find(entryId);
        if (it == m_ids.end()) {
            return false;
        }
        return true;
    }

    void createEntryDataField(const std::string& entryId) {
        auto it = m_ids.find(entryId);
        if (it == m_ids.end()) {
            // For "m_ids[id] = m_ids.size();" on Ubuntu platform, [] seems to increment the m_ids
            // size before assignment that causes incorrect indexes for later usage.
            auto index = m_ids.size();
            m_ids[entryId] = index;

            auto bucketIndex = m_ids[entryId] / UPLOAD_BATCH_SIZE;
            auto entriesIndex = m_ids[entryId] % UPLOAD_BATCH_SIZE;

            // Check if bucket mapping to bucketIndex is new bucket to be created.
            if (bucketIndex + 1 > (m_documents.size())) {
                auto document = std::make_shared<rapidjson::Document>();
                document->SetObject();

                m_documents.push_back(document);
                auto& allocator = document->GetAllocator();

                rapidjson::Value entries(rapidjson::kArrayType);

                document->AddMember("entries", entries, allocator);
            }

            auto& document = *m_documents[bucketIndex];
            auto& allocator = document.GetAllocator();

            auto& entries = document["entries"];

            if (entriesIndex + 1 > entries.Size()) {
                rapidjson::Value entry(rapidjson::kObjectType);

                entry.AddMember("entrySourceId", entryId, allocator);
                rapidjson::Value data(rapidjson::kObjectType);

                entry.AddMember("data", data, allocator);

                entries.PushBack(entry, allocator);
            }
        }
    }

    rapidjson::Value& getEntryDataField(const std::string& entryId) {
        auto bucketIndex = m_ids[entryId] / UPLOAD_BATCH_SIZE;
        auto entriesIndex = m_ids[entryId] % UPLOAD_BATCH_SIZE;

        auto& document = *m_documents[bucketIndex];
        auto& entries = document["entries"];

        return entries[entriesIndex]["data"];
    }

    rapidjson::Document::AllocatorType& GetAllocator(const std::string& entryId) {
        auto bucketIndex = m_ids[entryId] / UPLOAD_BATCH_SIZE;
        auto& document = *m_documents[bucketIndex];

        return document.GetAllocator();
    }

public:
    bool addName(const std::string& entryId, const std::string& name) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdInvalid");
            ThrowIf(name.size() > MAX_ALLOWED_CHARACTERS, "nameInvalid");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataField(entryId);
            auto& allocator = GetAllocator(entryId);

            rapidjson::Value nameValue(rapidjson::kObjectType);

            ThrowIf(data.HasMember("name"), "nameFound");

            if (!name.empty()) nameValue.AddMember("firstName", name, allocator);

            data.AddMember("name", nameValue, allocator);
            return true;

        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(const std::string& entryId, const std::string& firstName, const std::string& lastName) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdInvalid");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameInvalid");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameInvalid");
            ThrowIf(firstName.size() + lastName.size() > MAX_ALLOWED_CHARACTERS, "nameInvalid");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataField(entryId);
            auto& allocator = GetAllocator(entryId);

            rapidjson::Value name(rapidjson::kObjectType);

            ThrowIf(data.HasMember("name"), "nameFound");

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);

            data.AddMember("name", name, allocator);

            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(
        const std::string& entryId,
        const std::string& firstName,
        const std::string& lastName,
        const std::string& nickname) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdInvalid");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameInvalid");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameInvalid");
            ThrowIf(nickname.size() > MAX_ALLOWED_CHARACTERS, "nickNameInvalid");
            ThrowIf((firstName.size() + lastName.size() + nickname.size()) > MAX_ALLOWED_CHARACTERS, "nameInvalid");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataField(entryId);
            auto& allocator = GetAllocator(entryId);
            rapidjson::Value name(rapidjson::kObjectType);

            ThrowIf(data.HasMember("name"), "nameFound");

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);
            if (!nickname.empty()) name.AddMember("nickName", nickname, allocator);

            data.AddMember("name", name, allocator);

            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addPhone(const std::string& entryId, const std::string& label, const std::string& number) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdInvalid");
            ThrowIfNot(
                m_addressBookEntity->isAddressTypeSupported(AddressBookEntity::AddressType::PHONE),
                "addressTypeNotSupported");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataField(entryId);
            auto& allocator = GetAllocator(entryId);

            if (!data.HasMember("addresses")) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember("addresses", addresses, allocator);
            }

            auto& addresses = data["addresses"];

            ThrowIf(isMaxAllowedReached(addresses), "maxAllowedReached");

            rapidjson::Value address(rapidjson::kObjectType);
            address.AddMember("addressType", "phonenumber", allocator);

            if (!label.empty()) address.AddMember("rawType", label, allocator);
            if (!number.empty()) address.AddMember("value", number, allocator);

            addresses.PushBack(address, allocator);

            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addPhone").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addPostalAddress(
        const std::string& entryId,
        const std::string& label,
        const std::string& addressLine1,
        const std::string& addressLine2,
        const std::string& addressLine3,
        const std::string& city,
        const std::string& stateOrRegion,
        const std::string& districtOrCounty,
        const std::string& postalCode,
        const std::string& countryCode,
        float latitudeInDegrees,
        float longitudeInDegrees,
        float accuracyInMeters) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdInvalid");
            int totalCharSize = addressLine1.size() + addressLine2.size() + addressLine3.size() + city.size() +
                                stateOrRegion.size() + districtOrCounty.size() + postalCode.size() + countryCode.size();
            ThrowIf(totalCharSize > MAX_ALLOWED_CHARACTERS, "postalAddressValueSizeExceedsLimit");
            ThrowIfNot((latitudeInDegrees >= -90 && latitudeInDegrees <= 90), "latitudeInDegreesInvalid");
            ThrowIfNot((longitudeInDegrees >= -180 && longitudeInDegrees <= 180), "longitudeInDegreesInvalid");
            ThrowIf(accuracyInMeters < 0, "accuracyInMetersInvalid");

            ThrowIfNot(
                m_addressBookEntity->isAddressTypeSupported(AddressBookEntity::AddressType::POSTALADDRESS),
                "addressTypeNotSupported");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataField(entryId);
            auto& allocator = GetAllocator(entryId);

            if (!data.HasMember("addresses")) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember("addresses", addresses, allocator);
            }

            auto& addresses = data["addresses"];

            ThrowIf(isMaxAllowedReached(addresses), "maxAllowedReached");

            rapidjson::Value address(rapidjson::kObjectType);
            address.AddMember("addressType", "postaladdress", allocator);

            if (!label.empty()) address.AddMember("rawType", label, allocator);

            rapidjson::Value postalAddressValue(rapidjson::kObjectType);

            if (!addressLine1.empty()) postalAddressValue.AddMember("addressLine1", addressLine1, allocator);
            if (!addressLine2.empty()) postalAddressValue.AddMember("addressLine2", addressLine2, allocator);
            if (!addressLine3.empty()) postalAddressValue.AddMember("addressLine3", addressLine3, allocator);
            if (!city.empty()) postalAddressValue.AddMember("city", city, allocator);
            if (!stateOrRegion.empty()) postalAddressValue.AddMember("stateOrRegion", stateOrRegion, allocator);
            if (!districtOrCounty.empty())
                postalAddressValue.AddMember("districtOrCounty", districtOrCounty, allocator);
            if (!postalCode.empty()) postalAddressValue.AddMember("postalCode", postalCode, allocator);
            if (!countryCode.empty()) postalAddressValue.AddMember("countryCode", countryCode, allocator);

            rapidjson::Value coordinate(rapidjson::kObjectType);
            coordinate.AddMember("latitudeInDegrees", latitudeInDegrees, allocator);
            coordinate.AddMember("longitudeInDegrees", longitudeInDegrees, allocator);
            if (accuracyInMeters > 0) coordinate.AddMember("accuracyInMeters", accuracyInMeters, allocator);
            postalAddressValue.AddMember("coordinate", coordinate, allocator);

            address.AddMember("postalAddress", postalAddressValue, allocator);

            addresses.PushBack(address, allocator);
            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addPostalAddress").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool isMaxAllowedReached(rapidjson::Value& addresses) {
        if (addresses.IsArray() && addresses.Size() < MAX_ALLOWED_ADDRESSES_PER_ENTRY) {
            return false;
        }
        return true;
    }

private:
    std::shared_ptr<AddressBookEntity> m_addressBookEntity;
    std::vector<std::shared_ptr<rapidjson::Document>>& m_documents;
    std::unordered_map<std::string, rapidjson::SizeType> m_ids;
};

bool AddressBookCloudUploader::handleUpload(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    std::string addressBookSourceId = INVALID_ADDRESS_BOOK_SOURCE_ID;
    try {
        addressBookSourceId = addressBookEntity->getSourceId();

        std::vector<std::shared_ptr<rapidjson::Document>> documents;
        auto factory = std::make_shared<AddressBookEntriesFactory>(addressBookEntity, documents);

        AACE_INFO(LX(TAG, "handleUpload").m("GettingAddressBookEntries").d("addressBookSourceId", addressBookSourceId));

        if (!m_addressBookService->getEntries(addressBookSourceId, factory)) {
            // getEntries can return false, it probably means OEM was not successful in providing all the entries.
            // The common reason could be the address book may have become unavailable or not accessible, so do not retry.
            AACE_WARN(
                LX(TAG, "handleUpload").d("addressBookSourceId", addressBookSourceId).d("reason", "getEntriesFailed"));
            // Return true to drop this address book from retry.
            return true;
        }

        if (documents.size() <= 0) {
            // Its the empty document.
            AACE_WARN(LX(TAG, "handleUpload")
                          .d("addressBookSourceId", addressBookSourceId)
                          .d("reason", "emptyDocumentToUpload"));
            // Return true to drop this address book from retry.
            return true;
        }

        int numberOfEntries = 0;
        for (auto document : documents) {
            auto entries = document->FindMember("entries");
            if (entries != document->MemberEnd()) {
                numberOfEntries += entries->value.Size();
            }
        }

        //Preparing for the upload
        auto cloudAddressBookId = prepareForUpload(addressBookEntity);
        ThrowIf(cloudAddressBookId.empty(), "prepareUploadFailed");

        double uploadStartTimer = getCurrentTimeInMs();

        //Upload json document in a loop.
        for (auto document : documents) {
            ThrowIfNot(upload(cloudAddressBookId, document), "uploadDocumentFailed");
        }

        // It is assumed that between contacts and navigation addresses the difference is payload that should not
        // influence the latency for uploading one batch of address book entries.
        double totalDuration = getCurrentTimeInMs() - uploadStartTimer;
        double timeToUploadOneBatch = totalDuration / documents.size();
        emitTimerMetrics("handleUpload", METRIC_TIME_TO_UPLOAD_ONE_BATCH, timeToUploadOneBatch);

        AACE_INFO(LX(TAG, "handleUpload")
                      .m("SuccessfullyUploaded")
                      .d("addressBookSourceId", addressBookSourceId)
                      .d("numberOfEntries", numberOfEntries));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleUpload").d("addressBookSourceId", addressBookSourceId).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploader::handleRemove(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    std::string addressBookSourceId = INVALID_ADDRESS_BOOK_SOURCE_ID;
    try {
        addressBookSourceId = addressBookEntity->getSourceId();

        ThrowIfNot(deleteAddressBook(addressBookEntity), "addressBookDeleteFailed");

        AACE_INFO(LX(TAG, "handleRemove").m("Removed Successfully").d("addressBookSourceId", addressBookSourceId));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleRemove").d("addressBookSourceId", addressBookSourceId).d("reason", ex.what()));
        return false;
    }
}

void AddressBookCloudUploader::eventLoop() {
    AACE_INFO(LX(TAG));
    bool cleanAllAddressBookAtStart = true;  // Delete all address books in Cloud at start.
    while (!m_isShuttingDown) {
        // Clean up previous address books in cloud.
        if (cleanAllAddressBookAtStart) {
            if (cleanAllCloudAddressBooks()) {
                cleanAllAddressBookAtStart = false;
            } else {
                continue;
            }
        }

        AACE_DEBUG(LX(TAG).m("waitingForEvents"));
        auto event = popNextEventFromQ();  // blocking call.
        if (m_isShuttingDown) {
            AACE_INFO(LX(TAG).m("shutdownTriggeredExitEventLoop"));
            break;
        }

        bool result = true;
        if (Event::Type::INVALID != event.getType()) {
            if (Event::Type::ADD == event.getType()) {
                result = handleUpload(event.getAddressBookEntity());
            } else if (Event::Type::REMOVE == event.getType()) {
                result = handleRemove(event.getAddressBookEntity());
            }

            bool enqueueBackPoppedEvent = false;
            if (!result) {
                if (m_networkStatus != NetworkStatus::CONNECTED) {
                    // If network lost, add back to queue.
                    enqueueBackPoppedEvent = true;
                } else {
                    event.incrementRetryCount();
                    if (event.getRetryCount() < MAX_EVENT_RETRY) {
                        enqueueBackPoppedEvent = true;
                    } else {
                        AACE_INFO(LX(TAG, "eventLoop")
                                      .m("maxRetryReachedDropingtheEvent")
                                      .d("addressBookSourceId", event.getAddressBookEntity()->getSourceId())
                                      .d("eventType", event.getType()));
                    }
                }

                if (enqueueBackPoppedEvent) {
                    std::lock_guard<std::mutex> guard(m_mutex);
                    AACE_INFO(LX(TAG, "eventLoop")
                                  .m("addingBackForRetry")
                                  .d("addressBookSourceId", event.getAddressBookEntity()->getSourceId())
                                  .d("eventType", event.getType()));
                    if (!isEventEnqueuedLocked(Event::Type::REMOVE, event.getAddressBookEntity())) {
                        m_addressBookEventQ.emplace_front(event);
                    }
                }
            } else {
                // For Metrics
                if (Event::Type::ADD == event.getType()) {
                    emitTimerMetrics(
                        "eventLoop",
                        METRIC_TIME_TO_UPLOAD_SINCE_ADD,
                        getCurrentTimeInMs() - event.getEventCreateTime());
                } else {
                    emitTimerMetrics(
                        "eventLoop",
                        METRIC_TIME_TO_UPLOAD_SINCE_REMOVE,
                        getCurrentTimeInMs() - event.getEventCreateTime());
                }
            }
        } else {
            AACE_WARN(LX(TAG, "eventLoop").m("invalidEventFromQueue"));
        }
    }
}

const Event AddressBookCloudUploader::popNextEventFromQ() {
    std::unique_lock<std::mutex> queueLock{m_mutex};
    auto shouldNotWait = [this]() {
        return (
            m_isShuttingDown ||
            (!m_addressBookEventQ.empty() && m_networkStatus == NetworkStatus::CONNECTED && m_isAuthRefreshed));
    };

    if (!shouldNotWait()) {
        m_waitStatusChange.wait(queueLock, shouldNotWait);
    }

    if (!m_addressBookEventQ.empty()) {
        auto event = m_addressBookEventQ.front();
        m_addressBookEventQ.pop_front();

        return event;
    }

    return Event::INVALID();
}

std::string AddressBookCloudUploader::prepareForUpload(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    try {
        ThrowIfNot(m_addressBookCloudUploaderRESTAgent->isAccountProvisioned(), "accountNotProvisioned");

        // Try to delete any previous address book in cloud.
        ThrowIfNot(deleteAddressBook(addressBookEntity), "addressBookDeleteFailed");

        auto cloudAddressBookId = createAddressBook(addressBookEntity);
        ThrowIf(cloudAddressBookId.empty(), "addressBookCreateFailed");

        return cloudAddressBookId;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "prepareForUpload").d("reason", ex.what()));
        return std::string();
    }
}

bool AddressBookCloudUploader::upload(
    const std::string& cloudAddressBookId,
    std::shared_ptr<rapidjson::Document> document) {
    try {
        auto entries = document->FindMember("entries");

        AACE_DEBUG(LX(TAG, "upload").d("entries.Size()", entries->value.Size()));

        ThrowIfNot(uploadEntries(cloudAddressBookId, document), "uploadEntriesFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "upload").d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploader::uploadEntries(
    const std::string& cloudAddressBookId,
    std::shared_ptr<rapidjson::Document> document) {
    HTTPResponse httpResponse;
    auto flowState = UploadFlowState::POST;
    bool success = true;

    while (!m_isShuttingDown && flowState != UploadFlowState::FINISH) {
        auto nextFlowState = UploadFlowState::ERROR;

        AACE_DEBUG(LX(TAG, "uploadEntries").d("flowState", flowState));

        switch (flowState) {
            case UploadFlowState::POST:
                nextFlowState = handleUploadEntries(cloudAddressBookId, document, httpResponse);
                break;
            case UploadFlowState::PARSE:
                nextFlowState = handleParseHTTPResponse(httpResponse);
                break;
            case UploadFlowState::ERROR:
                nextFlowState = handleError(cloudAddressBookId);
                success = false;
                break;
            case UploadFlowState::FINISH:
                break;
        }
        flowState = nextFlowState;
    }

    return success;
}

AddressBookCloudUploader::UploadFlowState AddressBookCloudUploader::handleError(const std::string& cloudAddressBookId) {
    try {
        ThrowIfNot(
            m_addressBookCloudUploaderRESTAgent->deleteCloudAddressBook(cloudAddressBookId),
            "deleteCloudAddressBookFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleError").d("reason", ex.what()));
    }

    return UploadFlowState::FINISH;
}

AddressBookCloudUploader::UploadFlowState AddressBookCloudUploader::handleUploadEntries(
    const std::string& addressBookId,
    std::shared_ptr<rapidjson::Document> document,
    HTTPResponse& httpResponse) {
    try {
        httpResponse = m_addressBookCloudUploaderRESTAgent->uploadDocumentToCloud(document, addressBookId);

        logNetworkMetrics(httpResponse);

        switch (httpResponse.code) {
            case HTTPResponseCode::SUCCESS_OK:
                return UploadFlowState::PARSE;
            case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
            case HTTPResponseCode::SUCCESS_NO_CONTENT:
            case HTTPResponseCode::REDIRECTION_START_CODE:
            case HTTPResponseCode::REDIRECTION_END_CODE:
            case HTTPResponseCode::BAD_REQUEST:
            case HTTPResponseCode::FORBIDDEN:
            case HTTPResponseCode::SERVER_INTERNAL_ERROR:
                Throw(
                    "handleUploadEntriesFailed" +
                    m_addressBookCloudUploaderRESTAgent->getHTTPErrorString(httpResponse));
                break;
        }
        return UploadFlowState::ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleUploadEntries").d("reason", ex.what()));
        return UploadFlowState::ERROR;  //On exception return ERROR.
    }
}

AddressBookCloudUploader::UploadFlowState AddressBookCloudUploader::handleParseHTTPResponse(
    const HTTPResponse& httpResponse) {
    try {
        std::queue<std::string> failedEntries;
        ThrowIfNot(
            m_addressBookCloudUploaderRESTAgent->parseCreateAddressBookEntryResponse(httpResponse, failedEntries),
            "responseJsonParseFailed");

        // Continue to upload rest of the entries, even if there are one or more failed entries.
        if (failedEntries.size()) {
            AACE_WARN(LX(TAG, "handleParse").d("NumberOfFailedEntries", failedEntries.size()));
        }

        return UploadFlowState::FINISH;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleParse").d("reason", ex.what()));
        return UploadFlowState::ERROR;
    }
}

void AddressBookCloudUploader::logNetworkMetrics(const HTTPResponse& httpResponse) {
    switch (httpResponse.code) {
        case HTTPResponseCode::SUCCESS_OK:
            // Do Nothing
            break;
        case HTTPResponseCode::BAD_REQUEST:
            emitCounterMetrics("logNetworkMetrics", METRIC_NETWORK_BAD_USER_INPUT, 1);
            break;
        case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
        case HTTPResponseCode::SUCCESS_NO_CONTENT:
        case HTTPResponseCode::REDIRECTION_START_CODE:
        case HTTPResponseCode::REDIRECTION_END_CODE:
        case HTTPResponseCode::FORBIDDEN:
        case HTTPResponseCode::SERVER_INTERNAL_ERROR:
            emitCounterMetrics("logNetworkMetrics", METRIC_NETWORK_ERROR, 1);
    }
}

std::string AddressBookCloudUploader::createAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    try {
        auto dsn = m_deviceInfo->getDeviceSerialNumber();  // Use DSN as addressBookSourceId.

        auto cloudAddressBookId = m_addressBookCloudUploaderRESTAgent->createAndGetCloudAddressBook(
            dsn, addressBookEntity->toJSONAddressBookType());
        ThrowIf(cloudAddressBookId.empty(), "emptyCloudAddressBookId");

        return cloudAddressBookId;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createAddressBook").d("reason", ex.what()));
        return std::string();
    }
}

bool AddressBookCloudUploader::deleteAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    try {
        auto dsn = m_deviceInfo->getDeviceSerialNumber();  // Use DSN as addressBookSourceId.

        std::string cloudAddressBookId;
        ThrowIfNot(
            m_addressBookCloudUploaderRESTAgent->getCloudAddressBookId(
                dsn, addressBookEntity->toJSONAddressBookType(), cloudAddressBookId),
            "getCloudAddressBookIdFailed");

        if (!cloudAddressBookId.empty()) {
            ThrowIfNot(
                m_addressBookCloudUploaderRESTAgent->deleteCloudAddressBook(cloudAddressBookId),
                "deleteCloudAddressBookFailed");
        }

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "deleteAddressBook").d("reason", ex.what()));
        return false;
    }
}

bool AddressBookCloudUploader::cleanAllCloudAddressBooks() {
    AACE_DEBUG(LX(TAG));
    try {
        {
            std::unique_lock<std::mutex> queueLock{m_mutex};
            auto shouldNotWait = [this]() {
                return m_isShuttingDown || (m_networkStatus == NetworkStatus::CONNECTED && m_isAuthRefreshed);
            };

            if (!shouldNotWait()) {
                m_waitStatusChange.wait(queueLock, shouldNotWait);
            }
        }

        if (m_isShuttingDown) {
            AACE_INFO(LX(TAG).m("shutdownTriggered"));
            return false;
        }

        if (!m_addressBookCloudUploaderRESTAgent->isAccountProvisioned()) {
            // Account not provisioned, no further action required.
            AACE_DEBUG(LX(TAG).m("accountNotProvisioned"));
            return true;
        }

        auto dsn = m_deviceInfo->getDeviceSerialNumber();

        // Delete Contact Address book
        std::string cloudAddressBookId;
        ThrowIfNot(
            m_addressBookCloudUploaderRESTAgent->getCloudAddressBookId(dsn, "automotive", cloudAddressBookId),
            "getCloudAddressBookIdFailed");
        if (!cloudAddressBookId.empty()) {
            ThrowIfNot(
                m_addressBookCloudUploaderRESTAgent->deleteCloudAddressBook(cloudAddressBookId),
                "deleteCloudAddressBookFailed");
        }

        // Delete Navigation Address book
        cloudAddressBookId = "";
        ThrowIfNot(
            m_addressBookCloudUploaderRESTAgent->getCloudAddressBookId(
                dsn, "automotivePostalAddress", cloudAddressBookId),
            "getCloudAddressBookIdFailed");
        if (!cloudAddressBookId.empty()) {
            ThrowIfNot(
                m_addressBookCloudUploaderRESTAgent->deleteCloudAddressBook(cloudAddressBookId),
                "deleteCloudAddressBookFailed");
        }

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "cleanAllCloudAddressBooks").d("reason", ex.what()));
        return false;
    }
}

void AddressBookCloudUploader::emitCounterMetrics(
    const std::string& methodName,
    const std::string& key,
    const int value) {
    auto metricEvent = std::shared_ptr<aace::engine::metrics::MetricEvent>(
        new aace::engine::metrics::MetricEvent(METRIC_PROGRAM_NAME, methodName));
    if (metricEvent) {
        metricEvent->addCounter(key, value);
        metricEvent->record();
    }
}

void AddressBookCloudUploader::emitTimerMetrics(
    const std::string& methodName,
    const std::string& key,
    const double value) {
    auto metricEvent = std::shared_ptr<aace::engine::metrics::MetricEvent>(
        new aace::engine::metrics::MetricEvent(METRIC_PROGRAM_NAME, methodName));
    if (metricEvent) {
        metricEvent->addTimer(key, value);
        metricEvent->record();
    }
}

double AddressBookCloudUploader::getCurrentTimeInMs() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    double duration = now_ms.count();
    return duration;
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aace
