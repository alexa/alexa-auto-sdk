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

#ifndef AACE_ADDRESS_BOOK_ADDRESS_BOOK_H
#define AACE_ADDRESS_BOOK_ADDRESS_BOOK_H

/** @file */

#include <string>
#include <iostream>

#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace addressBook {

class AddressBookEngineInterface;

class AddressBook : public aace::core::PlatformInterface {
public:
    virtual ~AddressBook();

    enum class AddressBookType {
        // Contacts
        CONTACT,

        // Navigation Address
        NAVIGATION
    };

    /**
     * Factory class for ingesting the AddressBook Entries.
     * 
     */
    class IAddressBookEntriesFactory {
    public:
        virtual ~IAddressBookEntriesFactory() = default;
        /**
         * Add name.
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] name Name of the entry, or an empty string if not available.
         * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
         */
        virtual bool addName(const std::string& entryId, const std::string& name) = 0;

        /**
         * Add first and last name.
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] firstName First name of the entry, or an empty string if not available.
         * @param [in] lastName Last name of the entry, or an empty string if not available.
         * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
         */
        virtual bool addName(const std::string& entryId, const std::string& firstName, const std::string& lastName) = 0;

        /**
         * Add first, last and nick name.
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] firstName First name of the entry, or an empty string if not available.
         * @param [in] lastName Last name of the entry, or an empty string if not available.
         * @param [in] nickName Nick name of the entry, or an empty string if not available.
         * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
         */
        virtual bool addName(
            const std::string& entryId,
            const std::string& firstName,
            const std::string& lastName,
            const std::string& nickname) = 0;

        /**
         * Add phone number(s).
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] label Alphanumeric phone label (Example: HOME, MOBILE), or an empty string if not available.
         * @param [in] number Numeric phone number, or an empty string if not available.
         * @return @c true on successful or @c false when reached the max allowed per entryId or if entryId is empty.
         */
        virtual bool addPhone(const std::string& entryId, const std::string& label, const std::string& number) = 0;

        /**
         * Add postal address(es).
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] label Alphanumeric postal address label (Example: HOME, WORK), or an empty string if not available.
         * @param [in] addressLine1 First line of the postal address, or an empty string if not available.
         * @param [in] addressLine2 Second line of the postal address, or an empty string if not available.
         * @param [in] addressLine3 Third line of the postal address, or an empty string if not available.
         * @param [in] city City name, or an empty string if not available.
         * @param [in] stateOrRegion State or Region name, or an empty string if not available.
         * @param [in] districtOrCounty District or County name, or an empty string if not available.
         * @param [in] postalCode Postal code or Zip code, or an empty string if not available.
         * @param [in] country Country name, or an empty string if not available.
         * @param [in] latitudeInDegrees Geo latitude in degrees.
         * @param [in] longitudeInDegrees Geo longitute in degrees.
         * @param [in] accuracyInMeters Accuracy in meters, or zero if not available.
         * @return @c true on successful or @c false when reached the max allowed per id or if entryId is empty.
         */
        virtual bool addPostalAddress(
            const std::string& entryId,
            const std::string& label,
            const std::string& addressLine1,
            const std::string& addressLine2,
            const std::string& addressLine3,
            const std::string& city,
            const std::string& stateOrRegion,
            const std::string& districtOrCounty,
            const std::string& postalCode,
            const std::string& country,
            float latitudeInDegrees,
            float longitudeInDegrees,
            float accuracyInMeters) = 0;
    };

    /**
     * Notifies the engine on an availability of an address book
     *
     * @param [in] addressBookSourceId A unique identifier for an address book.
     * @param [in] name Friendly name of the address book, or an empty string if not available.
     * @param [in] type Type of the address book @c AddressBookType.
     * @return @c false if address book was already added or some internal error otherwise @c true on successful.
     */
    bool addAddressBook(const std::string& addressBookSourceId, const std::string& name, AddressBookType type);

    /**
     * Notifies the engine on a non-availability of an already available address book.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book. 
     * @return @c false if address book does not already added or some internal error otherwise @c true on successful.
     */
    bool removeAddressBook(const std::string& addressBookSourceId);

    /**
     * Notifies the platform implementation to ingest address book entries using factory methods. Please
     * note that engine may call this function multiple times to get the entries of the same address book.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book added using @c addAddressBook
     * @param [in] factory Factory class for ingesting address book entries.
     * @return Return @c true if platform implementation has successfully ingested all the entries of the address book
     * otherwise @c false.
     */
    virtual bool getEntries(
        const std::string& addressBookSourceId,
        std::weak_ptr<IAddressBookEntriesFactory> factory) = 0;

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::addressBook::AddressBookEngineInterface> engineInterface);

private:
    std::shared_ptr<aace::addressBook::AddressBookEngineInterface> m_engineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const AddressBook::AddressBookType& type) {
    switch (type) {
        case AddressBook::AddressBookType::CONTACT:
            stream << "CONTACT";
            break;
        case AddressBook::AddressBookType::NAVIGATION:
            stream << "NAVIGATION";
            break;
    }
    return stream;
}

}  // namespace addressBook
}  // namespace aace

#endif  // AACE_ADDRESS_BOOK_ADDRESS_BOOK_H
