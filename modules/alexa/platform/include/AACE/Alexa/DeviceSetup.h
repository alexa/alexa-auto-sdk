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

#ifndef AACE_ALEXA_DEVICESETUP_H_
#define AACE_ALEXA_DEVICESETUP_H_

#include <AACE/Core/PlatformInterface.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * DeviceSetup platform interface should be extended and registered into the Alexa Engine.
 * This class is used to send an event to AVS when device setup is completed.
 * Alexa can use this event to trigger on-boarding experiences like a short first time conversation on supported devices.

https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/devicesetup.html 
 * setupCompleted event is triggered.
 *
 * @deprecated This platform interface is deprecated.
 *             Use the Alexa Auto Services Bridge (AASB) message broker
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
class DeviceSetup : public aace::core::PlatformInterface {
protected:
    DeviceSetup() = default;

public:
    virtual ~DeviceSetup();

    /**
     * Indicates the delivery status of @c setupCompleted.
     */
    enum class StatusCode {

        /**
         * The @c setupCompleted event was sent to AVS successfully.
         */
        SUCCESS,

        /**
         * The @c setupCompleted event was not sent to AVS successfully.
         */
        FAIL
    };

    /**
     * Notifies the platform implementation of the delivery status of @c setupCompleted. 
     */
    virtual void setupCompletedResponse(StatusCode statusCode) = 0;

    /**
     * Notifies the Engine to send an event to inform Alexa when a product has completed setup after an out-of-box experience (OOBE). The response status of the sending the event will be reported through @c setupCompletedResponse.
    * @note This method should be called only once after completion of OOBE. Subsequent calls may be ignored by the cloud.
    * @note Do not call this method if user is in Connectivity Mode, Preview Mode, or if Alexa wake word is disabled. Calling the API in such conditions results in undesired user experience.
    */
    void setupCompleted();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<DeviceSetupEngineInterface> deviceSetupEngineInterface);

private:
    std::weak_ptr<DeviceSetupEngineInterface> m_deviceSetupEngineInterface;
};

/**
 * This function returns the std::string equivalent of the input enum value
 */
inline std::string deviceSetupStatusCodeToString(const DeviceSetup::StatusCode& statusCode) {
    switch (statusCode) {
        case DeviceSetup::StatusCode::SUCCESS:
            return "SUCCESS";
        case DeviceSetup::StatusCode::FAIL:
            return "FAIL";
    }
    return "UNKNOWN";
}

inline std::ostream& operator<<(std::ostream& stream, const DeviceSetup::StatusCode& statusCode) {
    stream << deviceSetupStatusCodeToString(statusCode);
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_DEVICESETUP_H_
