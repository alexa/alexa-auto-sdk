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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_IMPL_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_IMPL_H

#include <unordered_map>
#include <unordered_set>

#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AACE/AddressBook/AddressBook.h>
#include <AACE/AddressBook/AddressBookEngineInterface.h>

#include "AddressBookServiceInterface.h"
#include "AddressBookObserver.h"

namespace aace {
namespace engine {
namespace addressBook {

class AddressBookEntity;

using AddressBookType = aace::addressBook::AddressBook::AddressBookType;

class AddressBookEngineImpl
        : public AddressBookServiceInterface
        , public aace::addressBook::AddressBookEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AddressBookEngineImpl> {
private:
    AddressBookEngineImpl(std::shared_ptr<aace::addressBook::AddressBook> platfromInterface);

public:
    static std::shared_ptr<AddressBookEngineImpl> create(
        std::shared_ptr<aace::addressBook::AddressBook> platfromInterface);

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::addressBook::AddressBook> m_platformInterface;

public:
    //AddressBookServiceInterface
    void addObserver(std::shared_ptr<AddressBookObserver> observer) override;
    void removeObserver(std::shared_ptr<AddressBookObserver> observer) override;
    bool getEntries(
        const std::string& addressBookSourceId,
        std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory) override;

    // AddressBookEngineInterface
    bool onAddAddressBook(const std::string& addressBookSourceId, const std::string& name, const AddressBookType type)
        override;
    bool onRemoveAddressBook(const std::string& addressBookSourceId) override;

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<AddressBookEntity>> m_addressBookEntities;
    std::unordered_set<std::shared_ptr<AddressBookObserver>> m_observers;
};

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_IMPL_H
