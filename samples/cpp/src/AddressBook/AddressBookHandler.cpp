/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Core/EngineMacros.h>
#include "SampleApp/AddressBook/AddressBookHandler.h"
#include "SampleApp/ApplicationContext.h"

#include <AASB/Message/AddressBook/AddressBook/ContactName.h>
#include <AASB/Message/AddressBook/AddressBook/NavigationName.h>
#include <AASB/Message/AddressBook/AddressBook/PhoneData.h>
#include <AASB/Message/AddressBook/AddressBook/PostalAddress.h>

#include <AASB/Message/AddressBook/AddressBook/AddAddressBookMessage.h>
#include <AASB/Message/AddressBook/AddressBook/RemoveAddressBookMessage.h>

#include <fstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace {
constexpr char CONTACTS_ID[] = "0001";
constexpr char NAVIGATION_FAVORITES_ID[] = "0002";
};  // namespace

namespace sampleApp {
namespace addressBook {

void from_json(const json& j, Phone& p) {
    j.at("label").get_to(p.label);
    j.at("number").get_to(p.number);
}

void from_json(const json& j, Name& p) {
    j.at("firstName").get_to(p.firstName);
    j.at("lastName").get_to(p.lastName);
    j.at("nickName").get_to(p.nickName);
    if (j.contains("phoneticFirstName")) j.at("phoneticFirstName").get_to(p.phoneticFirstName);
    if (j.contains("phoneticLastName")) j.at("phoneticLastName").get_to(p.phoneticLastName);
}

void from_json(const json& j, PostalAddress& p) {
    j.at("label").get_to(p.label);
    j.at("addressLine1").get_to(p.addressLine1);
    j.at("addressLine2").get_to(p.addressLine2);
    j.at("addressLine3").get_to(p.addressLine3);
    j.at("city").get_to(p.city);
    j.at("stateOrRegion").get_to(p.stateOrRegion);
    j.at("districtOrCounty").get_to(p.districtOrCounty);
    j.at("postalCode").get_to(p.postalCode);
    j.at("country").get_to(p.country);
    j.at("latitudeInDegrees").get_to(p.latitudeInDegrees);
    j.at("latitudeInDegrees").get_to(p.latitudeInDegrees);
    j.at("longitudeInDegrees").get_to(p.longitudeInDegrees);
    j.at("accuracyInMeters").get_to(p.accuracyInMeters);
}

void from_json(const json& j, Contact& p) {
    j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
    if (j.count("phoneNumbers") > 0) {
        j.at("phoneNumbers").get_to(p.phoneNumbers);
    }
}

void from_json(const json& j, NavigationFavorite& p) {
    j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
    j.at("postalAddress").get_to(p.postalAddress);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddressBookHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;

using MessageBroker = aace::core::MessageBroker;

AddressBookHandler::AddressBookHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AddressBookHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AddressBookHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AddressBookHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to AddAddressBookReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAddAddressBookReplyMessage(message); },
        AddAddressBookMessage::topic(),
        AddAddressBookMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to RemoveAddressBookReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleRemoveAddressBookReplyMessage(message); },
        RemoveAddressBookMessage::topic(),
        RemoveAddressBookMessage::action());
}

void AddressBookHandler::handleAddAddressBookReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AddAddressBookReplyMessage");
    AddAddressBookMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    if (promise) {
        promise->set_value(msg.payload.success);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "promise is null");
    }
}

void AddressBookHandler::handleRemoveAddressBookReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received RemoveAddressBookReplyMessage");
    RemoveAddressBookMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    if (promise) {
        promise->set_value(msg.payload.success);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "promise is null");
    }
}

bool AddressBookHandler::addAddressBook(
    const std::string& addressBookSourceId,
    const std::string& name,
    AddressBookType type) {
    // Publish the "AddAddressBook" message
    AddAddressBookMessage msg;
    msg.payload.addressBookSourceId = addressBookSourceId;
    msg.payload.name = name;
    msg.payload.type = type;
    msg.payload.addressBookData = populateAddressBook(type);
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

bool AddressBookHandler::removeAddressBook(const std::string& addressBookSourceId) {
    // Publish the "RemoveAddressBook" message
    RemoveAddressBookMessage msg;
    msg.payload.addressBookSourceId = addressBookSourceId;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

void AddressBookHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AddressBookHandler", message);
}

void AddressBookHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    activity->registerObserver(Event::onAddAddressBookPhone, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onAddAddressBookPhone");
        if (!LoadContactData(value)) {
            return false;
        }
        return addAddressBook(CONTACTS_ID, "PhoneBook", AddressBookType::CONTACT);
    });

    activity->registerObserver(Event::onRemoveAddressBookPhone, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onRemoveAddressBookPhone");
        return removeAddressBook(CONTACTS_ID);
    });

    activity->registerObserver(Event::onAddAddressBookAuto, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onAddAddressBookAuto");
        if (!LoadNavigationFavoritesData(value)) {
            return false;
        }
        return addAddressBook(NAVIGATION_FAVORITES_ID, "AutoBook", AddressBookType::NAVIGATION);
    });

    activity->registerObserver(Event::onRemoveAddressBookAuto, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onRemoveAddressBookAuto");
        return removeAddressBook(NAVIGATION_FAVORITES_ID);
    });

    activity->registerObserver(Event::onRemoveAllAddressBooks, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onRemoveAllAddressBooks");
        return removeAddressBook("");
    });
}

bool AddressBookHandler::LoadContactData(const std::string& filepath) {
    m_contacts.clear();

    std::ifstream i(filepath);

    if (!i) {
        log(logger::LoggerHandler::Level::ERROR, "Cannot find file when loading contact data: " + filepath);
        return false;
    }

    try {
        json j;
        i >> j;

        if (j.is_object()) {
            auto contacts = j.find("contacts");
            if (contacts != j.end() && contacts->is_array()) {
                m_contacts = contacts->get<std::vector<Contact>>();
                return true;
            } else {
                log(logger::LoggerHandler::Level::ERROR, "Cannot find json array with key of 'contacts' ");
            }
        }
    } catch (const std::exception& e) {
        log(logger::LoggerHandler::Level::ERROR, e.what());
    }

    return false;
}

bool AddressBookHandler::LoadNavigationFavoritesData(const std::string& filepath) {
    m_navigationFavorites.clear();

    std::ifstream i(filepath);

    if (!i) {
        log(logger::LoggerHandler::Level::ERROR, "Cannot find file when loading navigation favorite data: " + filepath);
        return false;
    }

    try {
        json j;
        i >> j;

        if (j.is_object()) {
            auto navigationFavorites = j.find("navigationFavorites");
            if (navigationFavorites != j.end() && navigationFavorites->is_array()) {
                m_navigationFavorites = navigationFavorites->get<std::vector<NavigationFavorite>>();
                return true;
            } else {
                log(logger::LoggerHandler::Level::ERROR, "Cannot find json array with key of 'navigationFavorites' ");
            }
        }
    } catch (const std::exception& e) {
        log(logger::LoggerHandler::Level::ERROR, e.what());
    }

    return false;
}

AddressBook AddressBookHandler::populateAddressBook(AddressBookType type) {
    AddressBook addressBook;
    if (type == AddressBookType::CONTACT) {
        for (Contact contact : m_contacts) {
            ContactName contactName;
            contactName.entryId = contact.id;
            contactName.firstName = contact.name.firstName;
            contactName.lastName = contact.name.lastName;
            contactName.nickname = contact.name.nickName;
            contactName.phoneticFirstName = contact.name.phoneticFirstName;
            contactName.phoneticFirstName = contact.name.phoneticFirstName;
            addressBook.contactNames.push_back(contactName);

            PhoneData phoneData;
            for (Phone phone : contact.phoneNumbers) {
                phoneData.entryId = contact.id;
                phoneData.label = phone.label;
                phoneData.number = phone.number;
            }
            addressBook.phoneData.push_back(phoneData);
        }
    } else if (type == AddressBookType::NAVIGATION) {
        for (NavigationFavorite navigationFavorite : m_navigationFavorites) {
            NavigationName navigationName;
            navigationName.entryId = navigationFavorite.id;
            navigationName.name = navigationFavorite.name.firstName;
            navigationName.phoneticName = navigationFavorite.name.phoneticFirstName;
            addressBook.navigationNames.push_back(navigationName);

            aasb::message::addressBook::addressBook::PostalAddress postalAddress;
            postalAddress.entryId = navigationFavorite.id;
            postalAddress.label = navigationFavorite.postalAddress.label;
            postalAddress.addressLine1 = navigationFavorite.postalAddress.addressLine1;
            postalAddress.addressLine2 = navigationFavorite.postalAddress.addressLine2;
            postalAddress.addressLine3 = navigationFavorite.postalAddress.addressLine3;
            postalAddress.city = navigationFavorite.postalAddress.city;
            postalAddress.stateOrRegion = navigationFavorite.postalAddress.stateOrRegion;
            postalAddress.districtOrCounty = navigationFavorite.postalAddress.districtOrCounty;
            postalAddress.postalCode = navigationFavorite.postalAddress.postalCode;
            postalAddress.country = navigationFavorite.postalAddress.country;
            postalAddress.latitudeInDegrees = navigationFavorite.postalAddress.latitudeInDegrees;
            postalAddress.longitudeInDegrees = navigationFavorite.postalAddress.longitudeInDegrees;
            postalAddress.accuracyInMeters = navigationFavorite.postalAddress.accuracyInMeters;
            addressBook.postalAddresses.push_back(postalAddress);
        }
    }
    return addressBook;
}

bool AddressBookHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the address book reply message to fulfill
    std::shared_ptr<AddressBookPromise> promise = std::make_shared<AddressBookPromise>();

    // create a future to receive the promised address book reply message when it is received
    std::shared_future<bool> future(promise->get_future());

    bool success = false;
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void AddressBookHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<AddressBookPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void AddressBookHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<AddressBookHandler::AddressBookPromise> AddressBookHandler::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

}  // namespace addressBook
}  // namespace sampleApp
