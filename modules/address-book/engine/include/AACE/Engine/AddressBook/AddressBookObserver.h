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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_OBSERVER_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_OBSERVER_H

#include <AACE/AddressBook/AddressBook.h>

namespace aace {
namespace engine {
namespace addressBook {

class AddressBookEntity;

class AddressBookObserver {
public:
    // Notifies the observer when an address book is added by platform
    virtual bool addressBookAdded(std::shared_ptr<AddressBookEntity> addressBookEntity) = 0;

    // Notifies the observer when an address book is removed by platform
    virtual bool addressBookRemoved(std::shared_ptr<AddressBookEntity> addressBookEntity) = 0;
};

using AddressBookType = aace::addressBook::AddressBook::AddressBookType;

class AddressBookEntity {
public:
    AddressBookEntity(const std::string& addressBookSourceId, const std::string& name, AddressBookType type) :
            m_addressBookSourceId(addressBookSourceId), m_name(name), m_type(type) {
    }
    enum class AddressType {
        /// Phone Numbers
        PHONE,

        /// Postal Address
        POSTALADDRESS
    };

    std::string getSourceId() {
        return m_addressBookSourceId;
    }

    std::string getName() {
        return m_name;
    }

    std::string toJSONAddressBookType() {
        switch (m_type) {
            case AddressBookType::CONTACT:
                return "automotive";
            case AddressBookType::NAVIGATION:
                return "automotivePostalAddress";
        }
        return "error";
    }

    AddressBookType getType() {
        return m_type;
    }

    bool isAddressTypeSupported(AddressType type) {
        if (AddressType::PHONE == type) {
            return AddressBookType::CONTACT == m_type;
        } else if (AddressType::POSTALADDRESS == type) {
            return AddressBookType::NAVIGATION == m_type;
        } else {
            return false;
        }
    }

private:
    std::string m_addressBookSourceId;
    std::string m_name;
    AddressBookType m_type;
};

inline std::ostream& operator<<(std::ostream& stream, const AddressBookEntity::AddressType& type) {
    switch (type) {
        case AddressBookEntity::AddressType::PHONE:
            stream << "PHONE";
            break;
        case AddressBookEntity::AddressType::POSTALADDRESS:
            stream << "POSTALADDRESS";
            break;
    }
    return stream;
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_IMPL_H
