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

#ifndef AASB_ENGINE_ADDRESS_BOOK_AASB_ADDRESS_BOOK_H
#define AASB_ENGINE_ADDRESS_BOOK_AASB_ADDRESS_BOOK_H

#include <AACE/AddressBook/AddressBook.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AASB/Message/AddressBook/AddressBook/AddressBook.h>

#include <string>
#include <memory>
#include <unordered_map>

namespace aasb {
namespace engine {
namespace addressBook {

/**
 * Provides a AASB Address Book handler.
 */
class AASBAddressBook
        : public aace::addressBook::AddressBook
        , public std::enable_shared_from_this<AASBAddressBook> {
private:
    AASBAddressBook() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBAddressBook> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::addressBook::AddressBook
    bool getEntries(const std::string& addressBookSourceId, std::weak_ptr<IAddressBookEntriesFactory> factory) override;

private:
    std::unordered_map<std::string, aasb::message::addressBook::addressBook::AddressBook> m_addressBookCache;
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace addressBook
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_ADDRESS_BOOK_AASB_ADDRESS_BOOK_H
