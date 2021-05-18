/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CBL_CBL_SERVICE_INTERFACE_H
#define AACE_ENGINE_CBL_CBL_SERVICE_INTERFACE_H

#include <memory>

#include "CBLEventListenerInterface.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLServiceInterface {
public:
    virtual ~CBLServiceInterface() = default;

    /**
     * Starts CBL authorization.
     */
    virtual void startCBL() = 0;

    /**
     * Cancels on-going CBL authorization.
     */
    virtual void cancelCBL() = 0;

    /**
     * Adds an CBL event listener.
     *
     * @param eventListener listener to add
     */
    virtual void addEventListener(std::shared_ptr<CBLEventListenerInterface> eventListener) = 0;

    /**
     * Removes an CBL event listener.
     * @param eventListener listener to remove
     */
    virtual void removeEventListener(std::shared_ptr<CBLEventListenerInterface> eventListener) = 0;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CBL_CBL_SERVICE_INTERFACE_H