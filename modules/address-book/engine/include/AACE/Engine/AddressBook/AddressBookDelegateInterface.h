/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_DELEGATE_INTERFACE_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_DELEGATE_INTERFACE_H

#include <AACE/AddressBook/AddressBook.h>
#include "AddressBookObserver.h"

namespace aace {
namespace engine {
namespace addressBook {

class AddressBookDelegateInterface {
public:
    virtual ~AddressBookDelegateInterface() = default;

    // Allow the implementation to specify whether an address book is enabled for
    // the given service. If not implemented, the default is that is is enabled
    virtual bool isAddressBookServiceEnabled(
        const std::string& serviceType,
        aace::addressBook::AddressBook::AddressBookType addressBookType) = 0;
};

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_DELEGATE_INTERFACE_H
