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

#ifndef AASB_DIRECTIVEDISPATCHER_H
#define AASB_DIRECTIVEDISPATCHER_H

#include <memory>
#include <string>

#include "aasb/interfaces/IAlexaCapabilityDirectiveListener.h"

namespace aasb {
namespace bridge {

/**
 * Directive Dispatcher is responsible for delivering alexa directives from all the
 * alexa handlers to the AASB client.
 */
class DirectiveDispatcher {
public:
    // Constructor
    DirectiveDispatcher() = default;

    /**
     * Register the listener which is to receive the directives from all AAC SDK platform
     * handlers.
     *
     * @param directiveListener client interested in listening to Alexa directives.
     */
    void registerCapabilityDirectiveListener(
        std::shared_ptr<aasb::bridge::IAlexaCapabilityDirectiveListener> directiveListener);

    /**
     * Exclusively used by AAC SDK platform handlers for dispatching directives to the
     * registered directive listener.
     *
     * @param topic Topic uniquely identifies the capability. Example, SpeechSynthesizer,
     *      SpeechRecognizer, PhoneCallControl etc.
     * @param action The directive to execute on device.
     * @param jsonPayload Parameters required for executing the directive on device.
     */
    void sendDirective(const std::string& topic, const std::string& action, const std::string& jsonPayload);

private:
    // Alexa directives listener.
    std::shared_ptr<aasb::bridge::IAlexaCapabilityDirectiveListener> m_AlexaCapabilityDirectiveListener;
};
}  // namespace bridge
}  // namespace aasb
#endif  // AASB_DIRECTIVEDISPATCHER_H
