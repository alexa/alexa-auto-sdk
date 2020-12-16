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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_SERVICE_INTERFACE_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_SERVICE_INTERFACE_H

#include <string>
#include <memory>
#include <mutex>

#include "AuthorizationProvider.h"

namespace aace {
namespace engine {
namespace authorization {

class AuthorizationServiceInterface {
public:
    /**
     * To register the authorization provider
     * 
     * @note If the service name provided already exists, then it will be over written with the
     * latest instance of @c AuthorizationProvider. 
     * 
     * @param provider The reference to @c AuthorizationProvider implementing the authorization.
     * @param service This represents the name of the authorization service. This needs to be unique.
     */
    virtual bool registerProvider(std::shared_ptr<AuthorizationProvider> provider, const std::string& service) = 0;

    /**
     * Gets the instance of @c AuthorizationProvider
     * 
     * @param service represents the authorization service for which instance is requested.
     * @return Returns the reference to the  @c AuthorizationProvider if the service is found, otherwise
     * @c nullptr.
     */
    virtual std::shared_ptr<AuthorizationProvider> getProvider(const std::string& service) = 0;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_IMPL_H