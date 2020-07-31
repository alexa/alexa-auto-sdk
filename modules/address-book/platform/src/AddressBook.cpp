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

#include <AACE/AddressBook/AddressBook.h>
#include <AACE/AddressBook/AddressBookEngineInterface.h>

namespace aace {
namespace addressBook {

AddressBook::~AddressBook() = default;

// Engine Interface
void AddressBook::setEngineInterface(std::shared_ptr<aace::addressBook::AddressBookEngineInterface> engineInterface) {
    m_engineInterface = engineInterface;
}

bool AddressBook::addAddressBook(
    const std::string& addressBookSourceId,
    const std::string& name,
    AddressBookType type) {
    if (m_engineInterface != nullptr) {
        return m_engineInterface->onAddAddressBook(addressBookSourceId, name, type);
    }
    return false;
}

bool AddressBook::removeAddressBook(const std::string& addressBookSourceId) {
    if (m_engineInterface != nullptr) {
        return m_engineInterface->onRemoveAddressBook(addressBookSourceId);
    }
    return false;
}

}  // namespace addressBook
}  // namespace aace
