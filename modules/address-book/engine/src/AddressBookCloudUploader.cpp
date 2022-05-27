/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// JSON for Modern C++
#include <nlohmann/json.hpp>

#include <typeinfo>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <AACE/Engine/Network/NetworkEngineService.h>
#include <AACE/Engine/AddressBook/AddressBookCloudUploader.h>

namespace aace {
namespace engine {
namespace addressBook {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBook.addressBookCloudUploader");

/// Upload entries batch size
static const int UPLOAD_BATCH_SIZE = 100;

/// Max allowed phone numbers per entry
static const int MAX_ALLOWED_ADDRESSES_PER_ENTRY = 30;

/// Max allowed characters
static const int MAX_ALLOWED_CHARACTERS = 1000;

/// Max allowed EntryId size
static const int MAX_ALLOWED_ENTRY_ID_SIZE = 200;

/// Max allowed EntryId size
static const int MAX_ALLOWED_ADDRESS_LINE_SIZE = 60;

/// Max event retry
static const int MAX_EVENT_RETRY = 3;

/// Invalid Address Id
static const std::string INVALID_ADDRESS_BOOK_SOURCE_ID = "INVALID";

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AddressBookCloudUploader";

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

/// Address book AVS service type
static const std::string ADDRESS_BOOK_SERVICE_TYPE_AVS = "AVS";

using json = nlohmann::json;

AddressBookCloudUploader::AddressBookCloudUploader() :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), m_isShuttingDown(false), m_isAuthRefreshed(false) {
}

std::shared_ptr<AddressBookCloudUploader> AddressBookCloudUploader::create(
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    NetworkInfoObserver::NetworkStatus networkStatus,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
    bool cleanAllAddressBooksAtStart) {
    try {
        auto addressBookCloudUploader = std::shared_ptr<AddressBookCloudUploader>(new AddressBookCloudUploader());
        ThrowIfNot(
            addressBookCloudUploader->initialize(
                addressBookService,
                authDelegate,
                deviceInfo,
                networkStatus,
                networkObserver,
                alexaEndpoints,
                cleanAllAddressBooksAtStart),
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
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
    bool cleanAllAddressBooksAtStart) {
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
        m_addressBookService->addObserver(shared_from_this(), ADDRESS_BOOK_SERVICE_TYPE_AVS);

        // Infinite event loop
        m_eventThread = std::thread{&AddressBookCloudUploader::eventLoop, this, cleanAllAddressBooksAtStart};

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
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "addressBookAdded", METRIC_ADD_ADDRESS_BOOK_CONTACT, 1);
        } else if (addressBookEntity->getType() == AddressBookType::NAVIGATION) {
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "addressBookAdded", METRIC_ADD_ADDRESS_BOOK_NAVIGATION, 1);
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
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "addressBookRemoved", METRIC_REMOVE_ADDRESS_BOOK_CONTACT, 1);
        } else if (addressBookEntity->getType() == AddressBookType::NAVIGATION) {
            emitCounterMetrics(
                METRIC_PROGRAM_NAME_SUFFIX, "addressBookRemoved", METRIC_REMOVE_ADDRESS_BOOK_NAVIGATION, 1);
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

    rapidjson::Value& getEntryDataNode(const std::string& entryId) {
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
    bool addEntry(const std::string& payload) {
        bool success = true;
        try {
            ThrowIf(payload.empty(), "payloadEmpty");
            auto entryPayload = json::parse(payload);

            ThrowIfNot(
                entryPayload.contains("entryId") && entryPayload["entryId"].is_string(), "entryIdMissingOrNotString");
            ThrowIf(entryPayload["entryId"].empty(), "entryIdEmpty");

            std::string entryId = entryPayload["entryId"];
            AACE_DEBUG(LX(TAG).d("entryId", entryId));

            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdSizeExceedsMaxSize");
            ThrowIfNot(entryPayload.contains("name") && entryPayload["name"].is_object(), "nameMissingOrInvalid");

            auto& nameNode = entryPayload["name"];

            // clang-format off
            // Sanitize name  field types
            ThrowIfNot((nameNode.contains("firstName") ? nameNode["firstName"].is_string() : true), "firstNameInvalid");
            ThrowIfNot((nameNode.contains("lastName") ? nameNode["lastName"].is_string() : true), "lastNameInvalid");
            ThrowIfNot((nameNode.contains("nickName") ? nameNode["nickName"].is_string() : true), "nickNameInvalid");
            ThrowIfNot((nameNode.contains("phoneticFirstName") ? nameNode["phoneticFirstName"].is_string() : true), "phoneticFirstNameInvalid");
            ThrowIfNot((nameNode.contains("phoneticLastName") ? nameNode["phoneticLastName"].is_string() : true), "phoneticLastNameInvalid");
            // clang-format on

            std::string firstName = nameNode.value("firstName", "");
            std::string lastName = nameNode.value("lastName", "");
            std::string nickName = nameNode.value("nickName", "");
            std::string phoneticFirstName = nameNode.value("phoneticFirstName", "");
            std::string phoneticLastName = nameNode.value("phoneticLastName", "");

            // Sanitize field size
            auto totalSize = firstName.size() + lastName.size() + nickName.size() + phoneticFirstName.size() +
                             phoneticLastName.size();
            if (totalSize > MAX_ALLOWED_CHARACTERS) {
                AACE_ERROR(LX(TAG)
                               .m("nameTotalLengthExceedsMaxSize")
                               .d("entryId", entryId)
                               .d("size", totalSize)
                               .d("maxSize", MAX_ALLOWED_CHARACTERS));
                return false;
            }

            ThrowIf(isEntryPresent(entryId), "entryAlreadyExists");

            createEntryDataField(entryId);

            auto& data = getEntryDataNode(entryId);
            auto& allocator = GetAllocator(entryId);
            rapidjson::Value name(rapidjson::kObjectType);

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);
            if (!nickName.empty()) name.AddMember("nickName", nickName, allocator);
            if (!phoneticFirstName.empty()) name.AddMember("phoneticFirstName", phoneticFirstName, allocator);
            if (!phoneticLastName.empty()) name.AddMember("phoneticLastName", phoneticLastName, allocator);

            data.AddMember("name", name, allocator);

            if (entryPayload.contains("phoneNumbers") && !entryPayload["phoneNumbers"].empty()) {
                // Consider phone numbers only when the address book type is CONTACT
                if (m_addressBookEntity->getType() == AddressBookType::CONTACT) {
                    if (!entryPayload["phoneNumbers"].is_array()) {
                        Throw("phoneNumbersFieldIsNotAnArray");
                    }

                    if (!data.HasMember("addresses")) {
                        auto addresses(rapidjson::kArrayType);
                        data.AddMember("addresses", addresses, allocator);
                    }

                    auto& addresses = data["addresses"];

                    int counter = 0;
                    for (auto& phoneNumber : entryPayload["phoneNumbers"]) {
                        if (++counter > MAX_ALLOWED_ADDRESSES_PER_ENTRY) {
                            AACE_WARN(LX(TAG).m("maxAllowedPhoneNumberEntriesReached"));
                            success = false;
                            break;  // bail out
                        }

                        // clang-format off
                        // Sanitize phone number field types
                        ThrowIfNot((phoneNumber.contains("label") ? phoneNumber["label"].is_string() : true), "phoneNumberLabelInvalid");
                        ThrowIfNot((phoneNumber.contains("number") ? phoneNumber["number"].is_string() : true), "phoneNumberNumberInvalid");
                        // clang-format on

                        // Sanitize phone number field sizes
                        std::string label = phoneNumber.value("label", "");
                        std::string number = phoneNumber.value("number", "");

                        totalSize = label.size() + number.size();
                        if (totalSize > MAX_ALLOWED_CHARACTERS) {
                            AACE_WARN(LX(TAG)
                                          .m("phoneNumberFieldExceedsMaxSize")
                                          .d("label", label)
                                          .d("number", number)
                                          .d("size", totalSize)
                                          .d("maxSize", MAX_ALLOWED_CHARACTERS));
                            success = false;
                            continue;
                        }

                        rapidjson::Value address(rapidjson::kObjectType);
                        address.AddMember("addressType", "phonenumber", allocator);
                        if (!label.empty()) address.AddMember("rawType", label, allocator);
                        if (!number.empty()) address.AddMember("value", number, allocator);

                        addresses.PushBack(address, allocator);
                    }
                } else {
                    AACE_WARN(LX(TAG).m("phoneNumbersNotSupportedInNavigationType"));
                    success = false;
                }
            }

            if (entryPayload.contains("postalAddresses") && !entryPayload["postalAddresses"].empty()) {
                // Consider postal addresses  only when the address book type is NAVIGATION
                if (m_addressBookEntity->getType() == AddressBookType::NAVIGATION) {
                    if (!entryPayload["postalAddresses"].is_array()) {
                        Throw("postalAddressesFieldIsNotAnArray");
                    }

                    if (!data.HasMember("addresses")) {
                        auto addresses(rapidjson::kArrayType);
                        data.AddMember("addresses", addresses, allocator);
                    }

                    auto& addresses = data["addresses"];

                    int counter = 0;
                    for (auto& postalAddress : entryPayload["postalAddresses"]) {
                        if (++counter > MAX_ALLOWED_ADDRESSES_PER_ENTRY) {
                            AACE_WARN(LX(TAG).m("maxAllowedPostalAddressEntriesReached"));
                            success = false;
                            break;  // bail out
                        }

                        // clang-format off
                        // Sanitize postal address fields types
                        ThrowIfNot((postalAddress.contains("label") ? postalAddress["label"].is_string() : true), "postalAddressLabelInvalid");
                        ThrowIfNot((postalAddress.contains("addressLine1") ? postalAddress["addressLine1"].is_string() : true), "postalAddressAddressLine1Invalid");
                        ThrowIfNot((postalAddress.contains("addressLine2") ? postalAddress["addressLine2"].is_string() : true), "postalAddressAddressLine2Invalid");
                        ThrowIfNot((postalAddress.contains("city") ? postalAddress["city"].is_string() : true), "postalAddressCityInvalid");
                        ThrowIfNot((postalAddress.contains("stateOrRegion") ? postalAddress["stateOrRegion"].is_string() : true), "postalAddressStateOrRegionInvalid");
                        ThrowIfNot((postalAddress.contains("districtOrCounty") ? postalAddress["districtOrCounty"].is_string() : true), "postalAddressDistrictOrCountyInvalid");
                        ThrowIfNot((postalAddress.contains("postalCode") ? postalAddress["postalCode"].is_string() : true), "postalAddressPostalCodeInvalid");
                        ThrowIfNot((postalAddress.contains("countryCode") ? postalAddress["countryCode"].is_string() : true), "postalAddressCountryCodeInvalid");
                        ThrowIfNot(postalAddress.contains("latitudeInDegrees") && postalAddress["latitudeInDegrees"].is_number(), "postalAddressLatitudeInDegreesNotPresetOrInvalid");
                        ThrowIfNot(postalAddress.contains("longitudeInDegrees") && postalAddress["longitudeInDegrees"].is_number(), "postalAddressLongitudeInDegreesNotPresetOrInvalid");
                        ThrowIfNot((postalAddress.contains("accuracyInMeters") ? postalAddress["accuracyInMeters"].is_number() : true), "postalAddressAccuracyInMetersInvalid");
                        // clang-format on

                        std::string label = postalAddress.value("label", "");
                        std::string addressLine1 = postalAddress.value("addressLine1", "");
                        std::string addressLine2 = postalAddress.value("addressLine2", "");
                        std::string addressLine3 = postalAddress.value("addressLine3", "");
                        std::string city = postalAddress.value("city", "");
                        std::string stateOrRegion = postalAddress.value("stateOrRegion", "");
                        std::string districtOrCounty = postalAddress.value("districtOrCounty", "");
                        std::string postalCode = postalAddress.value("postalCode", "");
                        std::string countryCode = postalAddress.value("countryCode", "");
                        float latitudeInDegrees = postalAddress.value("latitudeInDegrees", 0.0f);
                        float longitudeInDegrees = postalAddress.value("longitudeInDegrees", 0.0f);
                        float accuracyInMeters = postalAddress.value("accuracyInMeters", 0.0f);

                        // Sanitize postal address fields sizes
                        if (addressLine1.size() > MAX_ALLOWED_ADDRESS_LINE_SIZE) {
                            AACE_WARN(LX(TAG)
                                          .m("addressLine1ExceedsMaxCharacterSize")
                                          .d("entryId", entryId)
                                          .d("size", addressLine1.size())
                                          .d("maxSize", MAX_ALLOWED_ADDRESS_LINE_SIZE));
                            success = false;
                            continue;
                        }
                        if (addressLine2.size() > MAX_ALLOWED_ADDRESS_LINE_SIZE) {
                            AACE_WARN(LX(TAG)
                                          .m("addressLine2ExceedsMaxCharacterSize")
                                          .d("entryId", entryId)
                                          .d("size", addressLine2.size())
                                          .d("maxSize", MAX_ALLOWED_ADDRESS_LINE_SIZE));
                            success = false;
                            continue;
                        }
                        if (addressLine3.size() > MAX_ALLOWED_ADDRESS_LINE_SIZE) {
                            AACE_WARN(LX(TAG)
                                          .m("addressLine3ExceedsMaxCharacterSize")
                                          .d("entryId", entryId)
                                          .d("size", addressLine3.size())
                                          .d("maxSize", MAX_ALLOWED_ADDRESS_LINE_SIZE));
                            success = false;
                            continue;
                        }

                        int totalSize = label.size() + addressLine1.size() + addressLine2.size() + addressLine3.size() +
                                        city.size() + stateOrRegion.size() + districtOrCounty.size() +
                                        postalCode.size() + countryCode.size();

                        if (totalSize > MAX_ALLOWED_CHARACTERS) {
                            AACE_WARN(LX(TAG)
                                          .m("postalAddressExceedsMaxCharacterSize")
                                          .d("entryId", entryId)
                                          .d("size", totalSize)
                                          .d("maxSize", MAX_ALLOWED_CHARACTERS));
                            success = false;
                            continue;
                        }

                        if (!(latitudeInDegrees >= -90 && latitudeInDegrees <= 90)) {
                            AACE_WARN(LX(TAG).m("latitudeInDegreesInvalid").d("latitudeInDegrees", latitudeInDegrees));
                            success = false;
                            continue;
                        }

                        if (!(longitudeInDegrees >= -180 && longitudeInDegrees <= 180)) {
                            AACE_WARN(
                                LX(TAG).m("longitudeInDegreesInvalid").d("longitudeInDegrees", longitudeInDegrees));
                            success = false;
                            continue;
                        }

                        if (accuracyInMeters < 0) {
                            AACE_WARN(LX(TAG).m("accuracyInMetersInvalid").d("accuracyInMeters", accuracyInMeters));
                            success = false;
                            continue;
                        }

                        rapidjson::Value address(rapidjson::kObjectType);
                        address.AddMember("addressType", "postaladdress", allocator);
                        if (!label.empty()) address.AddMember("rawType", label, allocator);

                        rapidjson::Value postalAddressValue(rapidjson::kObjectType);
                        if (!addressLine1.empty())
                            postalAddressValue.AddMember("addressLine1", addressLine1, allocator);
                        if (!addressLine2.empty())
                            postalAddressValue.AddMember("addressLine2", addressLine2, allocator);
                        if (!addressLine3.empty())
                            postalAddressValue.AddMember("addressLine3", addressLine3, allocator);
                        if (!city.empty()) postalAddressValue.AddMember("city", city, allocator);
                        if (!stateOrRegion.empty())
                            postalAddressValue.AddMember("stateOrRegion", stateOrRegion, allocator);
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
                    }
                } else {
                    AACE_WARN(LX(TAG).m("postalAddressesNotSupportedInContactType"));
                    success = false;
                }
            }
            return success;
        } catch (json::parse_error& ex) {
            AACE_ERROR(LX(TAG).m("payLoadParseError").d("exception", ex.what()));
            return false;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(const std::string& entryId, const std::string& name) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(name.size() > MAX_ALLOWED_CHARACTERS, "nameExceedsMaxSize");

            ThrowIfNot(addName(entryId, name, "", ""), "addNameFailed");
            return true;

        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(const std::string& entryId, const std::string& firstName, const std::string& lastName) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameExceedsMaxSize");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameExceedsMaxSize");
            ThrowIf(firstName.size() + lastName.size() > MAX_ALLOWED_CHARACTERS, "nameExceedsMaxSize");

            ThrowIfNot(addName(entryId, firstName, lastName, ""), "addNameFailed");
            return true;

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
        const std::string& nickname,
        const std::string& phoneticFirstName = "",
        const std::string& phoneticLastName = "") {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameExceedsMaxSize");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameExceedsMaxSize");
            ThrowIf(nickname.size() > MAX_ALLOWED_CHARACTERS, "nickNameExceedsMaxSize");
            ThrowIf(phoneticFirstName.size() > MAX_ALLOWED_CHARACTERS, "phoneticFirstNameExceedsMaxSize");
            ThrowIf(phoneticLastName.size() > MAX_ALLOWED_CHARACTERS, "phoneticLastNameExceedsMaxSize");

            auto totalSize = firstName.size() + lastName.size() + nickname.size() + phoneticFirstName.size() +
                             phoneticLastName.size();
            if (totalSize > MAX_ALLOWED_CHARACTERS) {
                AACE_ERROR(LX(TAG)
                               .m("nameTotalLengthExceedsMaxSize")
                               .d("entryId", entryId)
                               .d("size", totalSize)
                               .d("maxSize", MAX_ALLOWED_CHARACTERS));
                return false;
            }

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataNode(entryId);
            auto& allocator = GetAllocator(entryId);
            rapidjson::Value name(rapidjson::kObjectType);

            ThrowIf(data.HasMember("name"), "nameFound");

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);
            if (!nickname.empty()) name.AddMember("nickName", nickname, allocator);
            if (!phoneticFirstName.empty()) name.AddMember("phoneticFirstName", phoneticFirstName, allocator);
            if (!phoneticLastName.empty()) name.AddMember("phoneticLastName", phoneticLastName, allocator);

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
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIfNot(
                m_addressBookEntity->isAddressTypeSupported(AddressBookEntity::AddressType::PHONE),
                "addressTypeNotSupported");

            if (!isEntryPresent(entryId)) {
                createEntryDataField(entryId);
            }
            auto& data = getEntryDataNode(entryId);
            auto& allocator = GetAllocator(entryId);

            if (!data.HasMember("addresses")) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember("addresses", addresses, allocator);
            }

            auto& addresses = data["addresses"];

            ThrowIf(hasMaxAllowedReached(addresses), "maxAllowedReached");

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
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
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
            auto& data = getEntryDataNode(entryId);
            auto& allocator = GetAllocator(entryId);

            if (!data.HasMember("addresses")) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember("addresses", addresses, allocator);
            }

            auto& addresses = data["addresses"];

            ThrowIf(hasMaxAllowedReached(addresses), "maxAllowedReached");

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

    bool hasMaxAllowedReached(rapidjson::Value& addresses) {
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
        emitTimerMetrics(
            METRIC_PROGRAM_NAME_SUFFIX, "handleUpload", METRIC_TIME_TO_UPLOAD_ONE_BATCH, timeToUploadOneBatch);

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
        ThrowIfNot(m_addressBookCloudUploaderRESTAgent->isAccountProvisioned(), "accountNotProvisioned");

        addressBookSourceId = addressBookEntity->getSourceId();

        ThrowIfNot(deleteAddressBook(addressBookEntity), "addressBookDeleteFailed");

        AACE_INFO(LX(TAG, "handleRemove")
                      .m("Removed Successfully")
                      .d("addressBookType", addressBookEntity->getType())
                      .d("addressBookSourceId", addressBookSourceId));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleRemove")
                       .d("addressBookType", addressBookEntity->getType())
                       .d("addressBookSourceId", addressBookSourceId)
                       .d("reason", ex.what()));
        return false;
    }
}

void AddressBookCloudUploader::eventLoop(bool cleanAllAddressBooksAtStart) {
    AACE_INFO(LX(TAG));
    bool cleanAllAddressBooks = cleanAllAddressBooksAtStart;
    while (!m_isShuttingDown) {
        // Clean up previous address books in cloud.
        if (cleanAllAddressBooks) {
            if (cleanAllCloudAddressBooks()) {
                cleanAllAddressBooks = false;
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
                                      .m("maxRetryReachedDroppingtheEvent")
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
                        METRIC_PROGRAM_NAME_SUFFIX,
                        "eventLoop",
                        METRIC_TIME_TO_UPLOAD_SINCE_ADD,
                        getCurrentTimeInMs() - event.getEventCreateTime());
                } else {
                    emitTimerMetrics(
                        METRIC_PROGRAM_NAME_SUFFIX,
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
    AACE_DEBUG(LX(TAG));
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
        AACE_DEBUG(LX(TAG, "upload").d("entries.Size()", document->FindMember("entries")->value.Size()));

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
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "logNetworkMetrics", METRIC_NETWORK_BAD_USER_INPUT, 1);
            break;
        case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
        case HTTPResponseCode::SUCCESS_NO_CONTENT:
        case HTTPResponseCode::REDIRECTION_START_CODE:
        case HTTPResponseCode::REDIRECTION_END_CODE:
        case HTTPResponseCode::FORBIDDEN:
        case HTTPResponseCode::SERVER_INTERNAL_ERROR:
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "logNetworkMetrics", METRIC_NETWORK_ERROR, 1);
    }
}

std::string AddressBookCloudUploader::createAddressBook(std::shared_ptr<AddressBookEntity> addressBookEntity) {
    try {
        // At the time of ER, AVS selects the address book using the DSN instead of the provided addressBookSourceId.
        // This is because at the time of utterance, Alexa does not know about the addressBookSourceId.
        auto dsn = m_deviceInfo->getDeviceSerialNumber();

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
        // DSN is used to get the cloud address book Id. See above comment on
        // why DSN is used instead of addressBookSourceId
        auto dsn = m_deviceInfo->getDeviceSerialNumber();

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

}  // namespace addressBook
}  // namespace engine
}  // namespace aace
