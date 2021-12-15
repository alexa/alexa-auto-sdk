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

#ifndef SAMPLEAPP_ADDRESS_BOOK_HANDLER_H
#define SAMPLEAPP_ADDRESS_BOOK_HANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AASB/Message/AddressBook/AddressBook/AddressBook.h>
#include <AASB/Message/AddressBook/AddressBook/AddressBookType.h>

#include <AACE/Core/MessageBroker.h>

#include <unordered_map>
#include <future>
#include <utility>

namespace sampleApp {
namespace addressBook {

using namespace aasb::message::addressBook::addressBook;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddressBookHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AddressBookHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AddressBookHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AddressBookHandler> {
        return std::shared_ptr<AddressBookHandler>(new AddressBookHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the AddAddressBookReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAddAddressBookReplyMessage(const std::string& message);

    /**
     * Handles the RemoveAddressBookReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleRemoveAddressBookReplyMessage(const std::string& message);

    /**
     * Publishes the AddAddressBook message to notify the engine on an availability of an address book.
     *
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book.
     * @param [in] name Friendly name of the address book, or an empty string if not available
     * @param [in] type Type of the address book @c AddressBookType
     * @return true after message is published to the Engine
     */
    bool addAddressBook(const std::string& addressBookSourceId, const std::string& name, AddressBookType type);

    /**
     * Publishes the RemoveAddressBook message to notify the engine on a non-availability of an already
     * available address book.
     *
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book. Set this to empty string for
     * engine to remove all uploaded address books
     * @return true after message is published to the Engine
     */
    bool removeAddressBook(const std::string& addressBookSourceId);

private:
    using AddressBookPromise = std::promise<bool>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<AddressBookPromise>> m_promiseMap;

    bool waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<AddressBookPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<AddressBookPromise> getReplyMessagePromise(const std::string& messageId);

private:
    std::weak_ptr<View> m_console{};

    std::vector<struct Contact> m_contacts;
    std::vector<struct NavigationFavorite> m_navigationFavorites;

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto LoadContactData(const std::string& filepath) -> bool;
    auto LoadNavigationFavoritesData(const std::string& filepath) -> bool;
    auto populateAddressBook(AddressBookType type) -> AddressBook;
};

struct Phone {
    std::string label;
    std::string number;
};

struct Name {
    std::string firstName;
    std::string lastName;
    std::string nickName;
    std::string phoneticLastName;
    std::string phoneticFirstName;
};

struct PostalAddress {
    std::string label;
    std::string addressLine1;
    std::string addressLine2;
    std::string addressLine3;
    std::string city;
    std::string stateOrRegion;
    std::string districtOrCounty;
    std::string postalCode;
    std::string country;
    float latitudeInDegrees;
    float longitudeInDegrees;
    float accuracyInMeters;
};

struct Contact {
    std::string id;
    Name name;
    std::vector<Phone> phoneNumbers;
};

struct NavigationFavorite {
    std::string id;
    Name name;
    PostalAddress postalAddress;
};

}  // namespace addressBook
}  // namespace sampleApp

#endif  // SAMPLEAPP_ADDRESS_BOOK_HANDLER_H