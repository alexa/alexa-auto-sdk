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

#ifndef AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_INTERFACE_H
#define AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace alexa {

/**
 * Interface to persist the adapter state of @c AuthorizationManager.
 */
class AuthorizationManagerStorageInterface {
public:
    /// Describes the adapter's name and its state.
    using AdapterState = std::pair<std::string, std::string>;

    /**
     *  Virtual Destructor.
     */
    virtual ~AuthorizationManagerStorageInterface() = default;

    /**
     * Saves the adapter state to the storage
     *
     * @param service The name of the adapter.
     * @param state The state of the adapter. 
     * @return Returns @c true on successful storage, otherwise @c false.
     */
    virtual bool saveAdapterState(AdapterState adapterState) = 0;

    /**
     * Loads a stored adapter state from the storage.
     *
     * @return Returns @c true on successfully retrieving the state from storage, otherwise @c false.
     */
    virtual bool loadAdapterState(AdapterState& adapterState) = 0;

    /**
     * Clears the adapter state from the storage.
     * 
     * @return Returns @c true if adapter state is successfully cleared, otherwise @c false.
     */
    virtual bool clearAdapterStateTable() = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_STORAGE_INTERFACE_H
