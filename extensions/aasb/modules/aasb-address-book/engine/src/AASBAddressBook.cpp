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

#include <AASB/Engine/AddressBook/AASBAddressBook.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/AddressBook/AddressBook/AddAddressBookMessage.h>
#include <AASB/Message/AddressBook/AddressBook/AddAddressBookMessageReply.h>
#include <AASB/Message/AddressBook/AddressBook/AddressBook.h>
#include <AASB/Message/AddressBook/AddressBook/AddressBookType.h>
#include <AASB/Message/AddressBook/AddressBook/ContactName.h>
#include <AASB/Message/AddressBook/AddressBook/NavigationName.h>
#include <AASB/Message/AddressBook/AddressBook/PhoneData.h>
#include <AASB/Message/AddressBook/AddressBook/PostalAddress.h>
#include <AASB/Message/AddressBook/AddressBook/RemoveAddressBookMessage.h>
#include <AASB/Message/AddressBook/AddressBook/RemoveAddressBookMessageReply.h>

namespace aasb {
namespace engine {
namespace addressBook {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.addressbook.AASBAddressBook");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAddressBook> AASBAddressBook::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBAddressBook>(new AASBAddressBook());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBAddressBookFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAddressBook::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;
        std::weak_ptr<AASBAddressBook> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::addressBook::addressBook::AddAddressBookMessage::topic(),
            aasb::message::addressBook::addressBook::AddAddressBookMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::addressBook::addressBook::AddAddressBookMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->m_addressBookCache[payload.addressBookSourceId] = payload.addressBookData;
                    bool success = sp->addAddressBook(
                        payload.addressBookSourceId, payload.name, static_cast<AddressBookType>(payload.type));

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::addressBook::addressBook::AddAddressBookMessageReply addAddressBookMessageReply;
                    addAddressBookMessageReply.header.messageDescription.replyToId = message.messageId();
                    addAddressBookMessageReply.payload.success = success;
                    m_messageBroker_lock->publish(addAddressBookMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "AddAddressBookMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::addressBook::addressBook::RemoveAddressBookMessage::topic(),
            aasb::message::addressBook::addressBook::RemoveAddressBookMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::addressBook::addressBook::RemoveAddressBookMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->m_addressBookCache.erase(payload.addressBookSourceId);
                    bool success = sp->removeAddressBook(payload.addressBookSourceId);

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::addressBook::addressBook::RemoveAddressBookMessageReply
                        removeAddressBookMessageReply;
                    removeAddressBookMessageReply.header.messageDescription.replyToId = message.messageId();
                    removeAddressBookMessageReply.payload.success = success;
                    m_messageBroker_lock->publish(removeAddressBookMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "RemoveAddressBookMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::addressBook::AddressBook
//

bool AASBAddressBook::getEntries(
    const std::string& addressBookSourceId,
    std::weak_ptr<IAddressBookEntriesFactory> factory) {
    try {
        AACE_VERBOSE(LX(TAG));

        //We send the entries through the cached address book.  We do not do this over AASB messages as the address has already been sent at the start.

        auto sp = factory.lock();
        ThrowIfNull(sp, "invalidWeakPtrReference");

        AACE_VERBOSE(LX(TAG).d("addressBookSourceId", addressBookSourceId));

        auto addressBookIter = m_addressBookCache.find(addressBookSourceId);
        if (addressBookIter != m_addressBookCache.end()) {
            const auto& addressBook = addressBookIter->second;

            for (const auto& navName : addressBook.navigationNames) {
                AACE_VERBOSE(LX(TAG).d("adding navName", navName.name));
                sp->addName(navName.entryId, navName.name);
            }

            for (const auto& contactName : addressBook.contactNames) {
                AACE_VERBOSE(LX(TAG).d("adding contactName", contactName.firstName));
                sp->addName(contactName.entryId, contactName.firstName, contactName.lastName, contactName.nickname);
            }

            for (const auto& phone : addressBook.phoneData) {
                AACE_VERBOSE(LX(TAG).d("adding phone", phone.label));
                sp->addPhone(phone.entryId, phone.label, phone.number);
            }

            for (const auto& postalAddress : addressBook.postalAddresses) {
                AACE_VERBOSE(LX(TAG).d("adding postalAddress", postalAddress.label));
                sp->addPostalAddress(
                    postalAddress.entryId,
                    postalAddress.label,
                    postalAddress.addressLine1,
                    postalAddress.addressLine2,
                    postalAddress.addressLine3,
                    postalAddress.city,
                    postalAddress.stateOrRegion,
                    postalAddress.districtOrCounty,
                    postalAddress.postalCode,
                    postalAddress.country,
                    postalAddress.latitudeInDegrees,
                    postalAddress.longitudeInDegrees,
                    postalAddress.accuracyInMeters);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aasb
