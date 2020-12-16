/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_H
#define AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_H

#include <memory>
#include <mutex>

#include <AVSCommon/SDKInterfaces/Storage/MiscStorageInterface.h>

#include "AuthorizationManagerStorageInterface.h"

namespace aace {
namespace engine {
namespace alexa {

/**
 * An implementation of @c AuthorizationManagerStorageInterface that uses @c MiscStorageInterface 
 * as an underlying storage.
 */
class AuthorizationManagerStorage : public AuthorizationManagerStorageInterface {
public:
    /**
     * Factory method to create an instance of @c AuthorizationManagerStorage given the 
     * instance of @c MiscStorageInterface.
     *
     * @param storage An instance of @c MiscStorageInterface to be used as an underlying storage.
     * @return An instance of @c AuthorizationManagerStorage on success, @c nullptr otherwise.
     */
    static std::shared_ptr<AuthorizationManagerStorage> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> storage);

    /// @name AuthorizationManagerStorageInterface functions.
    /// @{
    bool saveAdapterState(AdapterState adapterState) override;
    bool loadAdapterState(AdapterState& adapterState) override;
    bool clearAdapterStateTable() override;
    /// @}

private:
    /**
     * Constructor.
     *
     * @param storage An instance of @c MiscStorageInterface to be used as an underlying storage.
     */
    AuthorizationManagerStorage(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> storage);

    /**
     * Initializes the underlying storage and prepares an instance for use.
     *
     * @return @c true if initialization succeeded, @c false otherwise.
     */
    bool initialize();

private:
    /// An instance of @c MiscStorageInterface to be used as an underlying storage.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> m_miscStorage;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_H
