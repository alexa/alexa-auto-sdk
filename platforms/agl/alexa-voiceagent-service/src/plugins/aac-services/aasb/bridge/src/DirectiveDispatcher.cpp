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

#include "DirectiveDispatcher.h"
#include "PlatformSpecificLoggingMacros.h"
namespace aasb {
namespace bridge {

// ToDo synchronization required for below functions

// Send Directive to Application
void DirectiveDispatcher::sendDirective(
    const std::string& topic,
    const std::string& action,
    const std::string& payload) {
    if (m_AlexaCapabilityDirectiveListener != nullptr)
        m_AlexaCapabilityDirectiveListener->onReceivedDirective(topic, action, payload);
    else
        AASB_ERROR("AlexaCapabilityDirectiveListener doesn't exist");
}

void DirectiveDispatcher::registerCapabilityDirectiveListener(
    std::shared_ptr<aasb::bridge::IAlexaCapabilityDirectiveListener> directiveListener) {
    m_AlexaCapabilityDirectiveListener = directiveListener;
}

}  // namespace bridge
}  // namespace aasb