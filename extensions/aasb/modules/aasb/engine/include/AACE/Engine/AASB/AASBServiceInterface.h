/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_AASB_AASB_SERVICE_INTERFACE_H
#define AACE_ENGINE_AASB_AASB_SERVICE_INTERFACE_H

#include <memory>
#include <AACE/Engine/Core/ServiceDescription.h>

#include "MessageBrokerInterface.h"
#include "StreamManagerInterface.h"

namespace aace {
namespace engine {
namespace aasb {

class AASBServiceInterface {
public:
    virtual ~AASBServiceInterface();

public:
    virtual std::shared_ptr<MessageBrokerInterface> getMessageBroker() = 0;
    virtual std::shared_ptr<StreamManagerInterface> getStreamManager() = 0;
    virtual aace::engine::core::Version getConfiguredVersion() = 0;
    virtual aace::engine::core::Version getCurrentVersion() = 0;
    virtual bool getAutoEnableInterfaces() = 0;
    virtual uint16_t getDefaultMessageTimeout() = 0;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AASB_AASB_SERVICE_INTERFACE_H
