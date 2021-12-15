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

#ifndef AACE_JNI_ADDRESSBOOK_ADDRESSBOOK_BINDER_H
#define AACE_JNI_ADDRESSBOOK_ADDRESSBOOK_BINDER_H

#include <AACE/AddressBook/AddressBook.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace addressbook {

//
// AddressBookHandler
//

class AddressBookHandler : public aace::addressBook::AddressBook {
public:
    AddressBookHandler(jobject obj);

    // aace::addressBook::AddressBook
    bool getEntries(const std::string& addressBookSourceId, std::weak_ptr<IAddressBookEntriesFactory> factory) override;

private:
    JObject m_obj;
};

//
// AddressBookBinder
//

class AddressBookBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AddressBookBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_addressBookHandler;
    }

    std::shared_ptr<AddressBookHandler> getAddressBook() {
        return m_addressBookHandler;
    }

private:
    std::shared_ptr<AddressBookHandler> m_addressBookHandler;
};

//
// JAddressBookType
//

class JAddressBookTypeConfig : public EnumConfiguration<AddressBookHandler::AddressBookType> {
public:
    using T = AddressBookHandler::AddressBookType;

    const char* getClassName() override {
        return "com/amazon/aace/addressbook/AddressBook$AddressBookType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::CONTACT, "CONTACT"}, {T::NAVIGATION, "NAVIGATION"}};
    }
};

using JAddressBookType = JEnum<AddressBookHandler::AddressBookType, JAddressBookTypeConfig>;

}  // namespace addressbook
}  // namespace jni
}  // namespace aace
#endif  //AACE_JNI_ADDRESSBOOK_ADDRESSBOOK_BINDER_H
