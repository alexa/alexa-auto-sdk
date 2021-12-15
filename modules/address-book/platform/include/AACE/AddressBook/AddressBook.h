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

#ifndef AACE_ADDRESS_BOOK_ADDRESS_BOOK_H
#define AACE_ADDRESS_BOOK_ADDRESS_BOOK_H

/** @file */

#include <string>
#include <iostream>

#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace addressBook {

class AddressBookEngineInterface;

/**
 * @deprecated This platform interface is deprecated. 
 *             Use the Alexa Auto Services Bridge (AASB) message broker 
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
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
         * @deprecated This function is deprecated. Use @c addEntry() instead.
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] name Name of the entry, or an empty string if not available.
         * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
         */
        virtual bool addName(const std::string& entryId, const std::string& name) = 0;

        /**
         * Add first and last name.
         * 
         * @deprecated This function is deprecated. Use @c addEntry() instead.
         *
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] firstName First name of the entry, or an empty string if not available.
         * @param [in] lastName Last name of the entry, or an empty string if not available.
         * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
         */
        virtual bool addName(const std::string& entryId, const std::string& firstName, const std::string& lastName) = 0;

        /**
         * Add first name, last name, nickname, and phonetics.
         * 
         * @deprecated This function is deprecated. Use @c addEntry() instead.
         
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] firstName First name of the entry, or an empty string if not available.
         * @param [in] lastName Last name of the entry, or an empty string if not available.
         * @param [in] nickname Nickname of the entry, or an empty string if not available.
         * @param [in] phoneticFirstName The phonetic first name of the entry, or an empty string if not available.
         * @param [in] phoneticLastName The phonetic last name of the entry, or an empty string if not available.
         * @return @c true if successful or @c false when the entryId is empty or already used.
         *
         * @note If a name field contains Japanese Kanji characters, you are required to provide the corresponding phonetic field as well. 
         * Alexa uses the phonetic values for entity resolution and TTS when the device locale setting is "ja-JP".
         */
        virtual bool addName(
            const std::string& entryId,
            const std::string& firstName,
            const std::string& lastName,
            const std::string& nickname,
            const std::string& phoneticFirstName = "",
            const std::string& phoneticLastName = "") = 0;

        /**
         * Add phone number(s).
         * 
         * @deprecated This function is deprecated. Use @c addEntry() instead.
         * 
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] label Alphanumeric phone label (e.g., Home, Mobile, Work), or an empty string if not available.
         * @param [in] number Numeric phone number, or an empty string if not available.
         * @return @c true on successful or @c false when reached the max allowed per entryId or if entryId is empty.
         * 
         * @note For the phone labels recognized by Alexa and for information about disambiguating phone numbers 
         * when multiple labels are associated with a contact, see [Phone number type disambiguation](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/communication.html#phone-number-type-disambiguation).
         */
        virtual bool addPhone(const std::string& entryId, const std::string& label, const std::string& number) = 0;

        /**
         * Add postal address(es).
         *
         * @deprecated This function is deprecated. Use @c addEntry() instead.
         * 
         * @param [in] entryId A unique identifier of entry in an address book.
         * @param [in] label Alphanumeric postal address label (e.g., Home, Work), or an empty string if not available.
         * @param [in] addressLine1 First line of the postal address, or an empty string if not available.
         * @param [in] addressLine2 Second line of the postal address, or an empty string if not available.
         * @param [in] addressLine3 Third line of the postal address, or an empty string if not available.
         * @param [in] city City name, or an empty string if not available.
         * @param [in] stateOrRegion State or Region name, or an empty string if not available.
         * @param [in] districtOrCounty District or County name, or an empty string if not available.
         * @param [in] postalCode Postal code or Zip code, or an empty string if not available.
         * @param [in] country Country name, or an empty string if not available.
         * @param [in] latitudeInDegrees Geo latitude in degrees.
         * @param [in] longitudeInDegrees Geo longitude in degrees.
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

        /**
         * Add name, phone number and postal address of an entry in the address book
         *
         * @code{.json}
         * {
         *      "entryId": "{{STRING}}",
         *      "name": {
         *          "firstName": "{{STRING}}",
         *          "lastName": "{{STRING}}",
         *          "nickName": "{{STRING}}",
         *          "phoneticFirstName": "{{STRING}}",
         *          "phoneticLastName": "{{STRING}}"
         *      },
         *      "phoneNumbers": [
         *          {
         *              "label": "{{STRING}}",
         *              "number": "{{STRING}}"
         *          }
         *      ],
         *      "postalAddresses": [
         *          {
         *              "label": "{{STRING}}",
         *              "addressLine1": "{{STRING}}",
         *              "addressLine2": "{{STRING}}",
         *              "addressLine3": "{{STRING}}",
         *              "city": "{{STRING}}",
         *              "stateOrRegion": "{{STRING}}",
         *              "districtOrCounty": "{{STRING}}",
         *              "postalCode": "{{STRING}}",
         *              "country": "{{STRING}}",
         *              "latitudeInDegrees": {{FLOAT}},
         *              "longitudeInDegrees": {{FLOAT}},
         *              "accuracyInMeters": {{FLOAT}}
         *          }
         *      ]
         * }
         * @endcode
         * @li entryId A unique identifier for the address book entry. This field is required and must not be an empty string. Max size is 200 characters.
         * @li name.firstName The first name of the entry.
         * @li name.lastName The last name of the entry.
         * @li name.nickName The nick name of the entry.
         * @li name.phoneticFirstName The phonetic first name of the entry.
         * @li name.phoneticLastName The phonetic last name of the entry.
         * @li phoneNumbers.label Alphanumeric phone label (e.g., Home, Mobile, Work). Max size is 100 characters.
         * @li phoneNumbers.number Numeric phone number.
         * @li postalAddresses.label Alphanumeric postal address label (e.g., Home, Work). Max size is 100 characters.
         * @li postalAddresses.addressLine1 First line of the postal address. Max size 60.
         * @li postalAddresses.addressLine2 Second line of the postal address. Max size 60.
         * @li postalAddresses.addressLine3 Third line of the postal address. Max size 60.
         * @li postalAddresses.city City name of the postal address.
         * @li postalAddresses.stateOrRegion State or region name of the postal address.
         * @li postalAddresses.districtOrCounty District or county name of the postal address.
         * @li postalAddresses.postalCode Postal code or zip code of the postal address.
         * @li postalAddresses.country Country name of the postal address.
         * @li postalAddresses.latitudeInDegrees The latitude component of GPS geo coordinate in degrees (-90 to 90). This field is required.
         * @li postalAddresses.longitudeInDegrees The longitude component of GPS geo coordinate in degrees (-180 to 180). This field is required.
         * @li postalAddresses.accuracyInMeters GPS provided accuracy value for the geo coordinates.
         *
         * @param payload The address book entry data in JSON format.
         * @return @c true if successful or @c false when the input validation fails.
         *
         * @note For each field not marked required, exclude the field if the value is not available.
         *
         * @note If any required field is absent, the Engine returns @c false from @c addEntry() and discards the entire entry.
         *
         * @note The total length of all fields in the "name" element cannot exceed 1000 characters. If the element exceeds 
         * the limit, the Engine returns @c false from @c addEntry() and discards the entry.
         *
         * @note For each element in the “postalAddresses” or “phoneNumbers” arrays, the total number of characters used in a single element cannot 
         * exceed 1000. If an element exceeds the limit, the Engine returns @c false from @c addEntry() as a warning, uses the rest of the entry, and 
         * discards the element exceeding the limit.
         *
         * @note The maximum number of elements allowed in the “postalAddresses” or “phoneNumbers” array is 30. If the total number of elements exceeds the
         * limit, the Engine returns @c false from @c addEntry() as a warning and uses the first 30 elements.
         *
         * @note The Engine ignores the "postalAddresses" node while ingesting entries for the address book type @c AddressBookType::CONTACT. Similarly, the 
         * Engine ignores the "phoneNumbers" node while ingesting entries for the address book type @c AddressBookType::NAVIGATION.
         *
         * @note For the phone labels recognized by Alexa and for information about disambiguating phone numbers 
         * when multiple labels are associated with a contact, see [Phone number type disambiguation](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/communication.html#phone-number-type-disambiguation).
         *
         * @note If a name field contains Japanese Kanji characters, you are required to provide the corresponding phonetic field as well. 
         * Alexa uses the phonetic values for entity resolution and TTS when the device locale setting is "ja-JP".
         */
        virtual bool addEntry(const std::string& payload) = 0;
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
     * @param [in] addressBookSourceId A unique identifier for an address book. Set this to empty string for engine to remove all uploaded address books.
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
