/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_NETWORK_INTERFACE_NETWORK_OBSERVABLE_INTERFACE_H
#define AACE_ENGINE_NETWORK_INTERFACE_NETWORK_OBSERVABLE_INTERFACE_H

#include <memory>
#include "AACE/Engine/Network/NetworkInfoObserver.h"

namespace aace {
namespace engine {
namespace network {

class NetworkObservableInterface {
public:
    virtual ~NetworkObservableInterface();
    virtual void addObserver(std::shared_ptr<NetworkInfoObserver> observer) = 0;
    virtual void removeObserver(std::shared_ptr<NetworkInfoObserver> observer) = 0;
};

}  // namespace network
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NETWORK_INTERFACE_NETWORK_OBSERVABLE_INTERFACE_H
