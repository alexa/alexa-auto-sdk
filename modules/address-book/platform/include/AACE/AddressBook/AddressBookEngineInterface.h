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

#ifndef AACE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_INTERFACE_H
#define AACE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_INTERFACE_H

#include "AddressBook.h"

namespace aace {
namespace addressBook {

class AddressBookEngineInterface {
public:
    using AddressBookType = aace::addressBook::AddressBook::AddressBookType;

    virtual bool onAddAddressBook(
        const std::string& addressBookSourceId,
        const std::string& name,
        const AddressBookType type) = 0;
    virtual bool onRemoveAddressBook(const std::string& addressBookSourceId) = 0;
};

}  // namespace addressBook
}  // namespace aace

#endif  // AACE_ADDRESS_BOOK_ADDRESS_BOOK_INTERFACE_H
