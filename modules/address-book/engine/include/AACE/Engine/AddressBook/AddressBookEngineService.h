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

#ifndef AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_SERVICE_H
#define AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_SERVICE_H

#include <AACE/Engine/AddressBook/AddressBookCloudUploader.h>
#include <AACE/Engine/AddressBook/AddressBookEngineImpl.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/Core/EngineService.h>

namespace aace {
namespace engine {
namespace addressBook {

class AddressBookEngineService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.addressBook", VERSION("1.0"), DEPENDS(aace::engine::alexa::AlexaEngineService))

protected:
    // EngineService
    bool shutdown() override;

    AddressBookEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~AddressBookEngineService();

    // EngineService
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::addressBook::AddressBook> platformInterface);

    std::shared_ptr<AddressBookEngineImpl> m_addressBookEngineImpl;
    std::shared_ptr<AddressBookCloudUploader> m_addressBookCloudUploader;
};

}  // namespace addressBook
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ADDRESS_BOOK_ADDRESS_BOOK_ENGINE_SERVICE_H
