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

#ifndef AASB_ALEXA_CAPABILITY_DIRECTIVE_LISTENER_H
#define AASB_ALEXA_CAPABILITY_DIRECTIVE_LISTENER_H

#include <string>

namespace aasb {
namespace bridge {

/**
 * Listener interface which is implemented by AASB clients to receive various callbacks
 * such as directives coming from Alexa, and for streaming media data to aasb client.
 */
class IAlexaCapabilityDirectiveListener {
public:
    /**
     * Notifies that a new directive has arrived from alexa services which ought to be
     * executed by the aasb client service on device.
     *
     * @param topic Topic uniquely identifies the capability. Example, SpeechSynthesizer,
     *      SpeechRecognizer, PhoneCallControl etc.
     * @param action The directive to execute on device.
     * @param jsonPayload Parameters required for executing the directive on device.
     */
    virtual void onReceivedDirective(
        const std::string& topic,
        const std::string& action,
        const std::string& jsonPayload) = 0;
};
}  // namespace bridge
}  // namespace aasb

#endif  // AASB_LISTENER_H
