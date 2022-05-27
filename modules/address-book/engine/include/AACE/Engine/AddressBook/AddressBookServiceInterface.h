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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_SERVICE_INTERFACE_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_SERVICE_INTERFACE_H

#include <AACE/AddressBook/AddressBook.h>
#include "AddressBookDelegateInterface.h"
#include "AddressBookObserver.h"

namespace aace {
namespace engine {
namespace addressBook {

class AddressBookServiceInterface {
public:
    virtual ~AddressBookServiceInterface() = default;

    // Subscribe to AddressBook Engine
    virtual void addObserver(std::shared_ptr<AddressBookObserver> observer, const std::string& serviceType) = 0;

    // Remove observer to AddressBook Engine
    virtual void removeObserver(std::shared_ptr<AddressBookObserver> observer) = 0;

    // Engine service to get entries of an address book from AddressBook Engine Service
    virtual bool getEntries(
        const std::string& addressBookSourceId,
        std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory) = 0;

    // Set delegate to validate service usage for an address book type.
    // If no delegate is set, all services can be used (default)
    virtual void setDelegate(std::shared_ptr<AddressBookDelegateInterface> delegate) = 0;

    // Service usage changed
    virtual void servicesEnablementChanged() = 0;
};

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_SERVICE_INTERFACE_H
