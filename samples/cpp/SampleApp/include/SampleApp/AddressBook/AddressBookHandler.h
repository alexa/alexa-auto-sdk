/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/AddressBook/AddressBook.h>

namespace sampleApp {
namespace addressBook {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddressBookHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AddressBookHandler : public aace::addressBook::AddressBook /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AddressBookHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AddressBookHandler> {
        return std::shared_ptr<AddressBookHandler>(new AddressBookHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // AddressBook
    bool getEntries(
        const std::string& id,
        std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory) override;

private:
    std::weak_ptr<View> m_console{};

    std::vector<struct Contact> m_contacts;
    std::vector<struct NavigationFavorite> m_navigationFavorites;

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto LoadContactData(const std::string& filepath) -> bool;
    auto LoadNavigationFavoritesData(const std::string& filepath) -> bool;
};

struct Phone {
    std::string label;
    std::string number;
};

struct Name {
    std::string firstName;
    std::string lastName;
    std::string nickName;
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
