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

#ifndef AACE_ENGINE_ALEXA_AUTHORIZATION_ADAPTER_INTERFACE_H
#define AACE_ENGINE_ALEXA_AUTHORIZATION_ADAPTER_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace alexa {

class AuthorizationAdapterInterface {
public:
    /// Virtual destructor
    virtual ~AuthorizationAdapterInterface() = default;

public:
    /**
     * Starts the deregister process.
     * @note This is a blocking call.
     */
    virtual void deregister() = 0;

    /**
     * Get the current authorization token.
     * 
     * @return The current authorization token. The returned value will be empty if an authorization token
     * has yet to be acquired or if the most recently acquired token has expired.
     */
    virtual std::string getAuthToken() = 0;

    /**
     * Notifies that an operation using the specified auth token experienced an authorization failure.
     *
     * @param token The token used to authorize the forbidden operation.
     */

    virtual void onAuthFailure(const std::string& token) = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace
#endif  // AACE_ENGINE_ALEXA_AUTHORIZATION_ADAPTER_INTERFACE_H
