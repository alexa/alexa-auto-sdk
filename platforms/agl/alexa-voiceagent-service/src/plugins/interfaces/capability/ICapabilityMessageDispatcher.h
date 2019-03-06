/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AGL_CAPABILITY_MESSAGE_DISPATCHER_H_
#define AGL_CAPABILITY_MESSAGE_DISPATCHER_H_

#include <string>

namespace agl {
namespace capability {
namespace interfaces {

/**
 * Interface to represent directive/message processor for one capability.
 */
class ICapabilityMessageDispatcher {
public:
    /**
     * Notifies that a new directive has arrived from alexa services.
     *
     * @param action The directive to execute.
     * @param jsonPayload Parameters required for executing the directive.
     */
    virtual void onReceivedDirective(const std::string& action, const std::string& jsonPayload) = 0;
};

}  // namespace interfaces
}  // namespace common
}  // namespace agl

#endif  // AGL_CAPABILITY_MESSAGE_DISPATCHER_H_
