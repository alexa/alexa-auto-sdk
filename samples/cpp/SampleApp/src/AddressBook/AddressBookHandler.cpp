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

#include "SampleApp/AddressBook/AddressBookHandler.h"
#include "SampleApp/ApplicationContext.h"

#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

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

AddressBookHandler::AddressBookHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AddressBookHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AddressBookHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::location::LocationProvider interface

// private

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
        return addAddressBook(CONTACTS_ID, "PhoneBook", aace::addressBook::AddressBook::AddressBookType::CONTACT);
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
        return addAddressBook(
            NAVIGATION_FAVORITES_ID, "AutoBook", aace::addressBook::AddressBook::AddressBookType::NAVIGATION);
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

bool AddressBookHandler::getEntries(
    const std::string& addressBookId,
    std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> contactFactory) {
    if (auto factory = contactFactory.lock()) {
        if (addressBookId == CONTACTS_ID) {
            for (auto&& contact : m_contacts) {
                json payload;
                payload["entryId"] = contact.id;
                payload["name"] = json::object();
                auto& nameNode = payload["name"];

                if (!contact.name.firstName.empty()) nameNode["firstName"] = contact.name.firstName;
                if (!contact.name.lastName.empty()) nameNode["lastName"] = contact.name.lastName;
                if (!contact.name.nickName.empty()) nameNode["nickName"] = contact.name.nickName;
                if (!contact.name.phoneticFirstName.empty())
                    nameNode["phoneticFirstName"] = contact.name.phoneticFirstName;
                if (!contact.name.phoneticLastName.empty())
                    nameNode["phoneticLastName"] = contact.name.phoneticLastName;

                if (contact.phoneNumbers.size() > 0) {
                    payload["phoneNumbers"] = json::array();
                }

                for (auto&& phone : contact.phoneNumbers) {
                    // clang-format off
                payload["phoneNumbers"].push_back({
                    {"label", phone.label},
                    {"number", phone.number}
                });
                    // clang-format on
                }
                factory->addEntry(payload.dump());
            }
            return true;
        } else if (addressBookId == NAVIGATION_FAVORITES_ID) {
            for (auto&& navigationFavorite : m_navigationFavorites) {
                json payload;
                payload["entryId"] = navigationFavorite.id;
                payload["name"] = json::object();
                auto& nameNode = payload["name"];

                if (!navigationFavorite.name.firstName.empty())
                    nameNode["firstName"] = navigationFavorite.name.firstName;
                if (!navigationFavorite.name.phoneticFirstName.empty())
                    nameNode["phoneticFirstName"] = navigationFavorite.name.phoneticFirstName;

                payload["postalAddresses"] = json::array();
                json postalAddress = {{"label", navigationFavorite.postalAddress.label},
                                      {"addressLine1", navigationFavorite.postalAddress.addressLine1},
                                      {"addressLine2", navigationFavorite.postalAddress.addressLine2},
                                      {"addressLine3", navigationFavorite.postalAddress.addressLine3},
                                      {"city", navigationFavorite.postalAddress.city},
                                      {"stateOrRegion", navigationFavorite.postalAddress.stateOrRegion},
                                      {"districtOrCounty", navigationFavorite.postalAddress.districtOrCounty},
                                      {"postalCode", navigationFavorite.postalAddress.postalCode},
                                      {"country", navigationFavorite.postalAddress.country},
                                      {"latitudeInDegrees", navigationFavorite.postalAddress.latitudeInDegrees},
                                      {"longitudeInDegrees", navigationFavorite.postalAddress.longitudeInDegrees},
                                      {"accuracyInMeters", navigationFavorite.postalAddress.accuracyInMeters}};

                payload["postalAddresses"].push_back(postalAddress);
                factory->addEntry(payload.dump());
            }
            return true;
        } else {
            log(logger::LoggerHandler::Level::ERROR, "Unknown addressBookId");
        }
    }
    return false;
}

}  // namespace addressBook
}  // namespace sampleApp
