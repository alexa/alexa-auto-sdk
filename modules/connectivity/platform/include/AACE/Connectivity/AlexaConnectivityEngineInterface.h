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

#ifndef AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_ENGINE_INTERFACE_H
#define AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_ENGINE_INTERFACE_H

#include <string>

namespace aace {
namespace connectivity {

class AlexaConnectivityEngineInterface {
public:
    virtual ~AlexaConnectivityEngineInterface() = default;

    /**
     * Indicates the delivery status of @c sendConnectivityEvent.
     */
    enum class StatusCode {
        /**
         * The @c sendConnectivityEvent event was sent to AVS successfully.
         */
        SUCCESS,

        /**
         * The @c sendConnectivityEvent event was not sent to AVS successfully.
         */
        FAIL
    };

    virtual bool onConnectivityStateChange() = 0;
    virtual void onSendConnectivityEvent(const std::string& event, const std::string& token) = 0;
};

}  // namespace connectivity
}  // namespace aace

#endif  // AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_ENGINE_INTERFACE_H
