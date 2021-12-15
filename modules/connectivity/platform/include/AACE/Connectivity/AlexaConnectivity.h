/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_H
#define AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_H

#include <memory>
#include <string>

#include <AACE/Core/PlatformInterface.h>

#include "AACE/Connectivity/AlexaConnectivityEngineInterface.h"

namespace aace {
namespace connectivity {

/**
 * AlexaConnectivity should be extended to send network connectivity status and data plan status to Alexa.
 * The information sent determines what Alexa features are available to the device.
 *
 * @deprecated This platform interface is deprecated. 
 *             Use the Alexa Auto Services Bridge (AASB) message broker 
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
class AlexaConnectivity : public aace::core::PlatformInterface {
protected:
    AlexaConnectivity() = default;

public:
    virtual ~AlexaConnectivity();

    /// Represents the delivery status of @c sendConnectivityEvent.
    using StatusCode = aace::connectivity::AlexaConnectivityEngineInterface::StatusCode;

    /**
     * Retrieve the connectivity state from the platform implementation.
     *
     * **ConnectivityState**
     *
     * Name                 | Description                                                       | Type      | Status
     * -------------------- | ----------------------------------------------------------------- | --------- | ---------
     * `managedProvider`    | Provides information about connectivity of the device.            | `Object`  | Required
     * `dataPlan`           | Provides the active data plan type and end date.                  | `Object`  | Optional
     * `dataPlansAvailable` | Indicates the data plans that can be activated on a device.       | `Array`   | Optional
     * `termsStatus`        | Indicates whether the customer has accepted the terms and conditions of the OEM and network provider. | `String`  | Optional
     * `termsVersion`       | Indicates the version of the terms and conditions put forward to the user. Max 250 characters | `String`  | Optional
     *
     * **managedProvider**
     *
     * Name                 | Description                                                       | Type      | Status
     * -------------------- | ----------------------------------------------------------------- | --------- | ---------
     * `type`               | Type of the managed provider, MANAGED or NOT_MANAGED (see below). | `String`  | Required
     * `id`                 | Specifies the name of the provider that manages connectivity. The only accepted value is AMAZON. | `String`  | Required*
     *
     * **managedProvider.type**
     *
     * Set to MANAGED if the device's internet connectivity is managed by a provider. The only possible provider
     * that manages connectivity is Amazon. When managedProvider.type is MANAGED, the Alexa experience is affected
     * by the attribute values of the InternetDataPlan capability. Set to NOT_MANAGED if the device's internet
     * connectivity is not managed by a provider (for example, if the customer accesses the internet via a WiFi
     * network or mobile hotspot). In this case, the customer can access all Alexa features, regardless of the
     * attribute values of the InternetDataPlan capability.
     *
     * **managedProvider.id**
     *
     * \c * Required only when managedProvider.type is MANAGED.
     *
     * **dataPlan**
     *
     * Name                 | Description                                                       | Type      | Status
     * -------------------- | ----------------------------------------------------------------- | --------- | ---------
     * `type`               | Type of the data plan (see below).                                | `String`  | Required
     * `endDate`            | RFC 3339 format date on which the current data plan ends.         | `String`  | Optional
     *
     * **dataPlan.type**
     *
     * Possible values are PAID, TRIAL, and AMAZON_SPONSORED. A customer with either of these data plan has
     * unrestricted access to all Alexa features. PAID indicates that the device has an active data plan paid for
     * by the customer.TRIAL indicates that the device has an active data plan which has been provided to the
     * customer as a promotional event. AMAZON_SPONSORED means that the customer has not paid for a data plan or
     * signed up for a free trial. The customer can connect to the internet via a plan sponsored by Amazon and can
     * access a limited number of Alexa features.
     *
     * For example:
     * @code{.json}
     * {
     *   "managedProvider": {
     *     "type": "MANAGED",
     *     "id": "AMAZON"
     *   },
     *   "dataPlan": {
     *     "type": "AMAZON_SPONSORED",
     *     "endDate": "2021-12-31T23:59:59.999Z"
     *   },
     *   "dataPlansAvailable": [
     *     "TRIAL", "PAID", "AMAZON_SPONSORED"
     *   ],
     *   "termsStatus": "ACCEPTED",
     *   "termsVersion": "1"
     * }
     * @endcode
     *
     * @note AlexaConnectivity platform interface registration requires a valid connectivity state.
     * @note You may return an empty string to indicate that connectivity state is not available.
     *
     * @return A @c std::string representing the connectivity state in structured JSON format.
     */
    virtual std::string getConnectivityState() = 0;

    /**
     * Retrieve the network identifier from the platform implementation.
     *
     * The network identifier is agnostic of the data plan and is assigned when initially integrated
     * into the vehicle. It links the device with the network provider and enables the network provider
     * to identify and provide device connectivity. The network identifier is optional, return an
     * empty string to automatically use VEHICLE_IDENTIFIER from Engine configuration instead.
     *
     * @return A @c std::string representing the network identifier, or empty for default behavior.
     *
     * @sa VehicleConfiguration
     */
    virtual std::string getIdentifier();

    /**
     * Notifies the Engine of a change in the connectivity state. The Engine calls @c getConnectivityState
     * to retrieve the the connectivity state and communicate any changes to Alexa.
     *
     * @return @c true if connectivity state was processed successfully, @c false otherwise.
     */
    bool connectivityStateChange();

    /**
     * Notifies the Engine of an event in connectivity.
     * 
     * @param [in] event The stringified JSON containing the event.
     * @code{.json}
     * {
     *   "type": "{{STRING}}"
     * }
     * @endcode
     * @li type (required) : Indicates the connectivity event type.
     * 
     * **Activating Trial**
     * 
     * Set event @c type to `"ACTIVATE_TRIAL"` for Alexa to begin the trial data plan activation (if available) via voice conversation with the
     * customer. Alexa, upon receiving this event, may perform some validations and eligibility checks before starting the voice conversation.
     * 
     * @note Alexa requires the customer to have accepted the OEM and network provider's terms and conditions before starting the
     * voice conversation for activating the trial data plan. 
     * 
     * @note If the platform implementation cannot determine the data plan type, then send this event. Alexa would first check the 
     * trial eligibility. If customer is not eligible, Alexa begins the paid plan voice conversation.
     * 
     * Example:
     * @code{.json}
     * {
     *   "type": "ACTIVATE_TRIAL"
     * }
     * @endcode
     * 
     * **Starting Paid Plan**
     * 
     * Set event @c type to `"ACTIVATE_PAID_PLAN"` for Alexa to begin the paid data plan activation via voice conversation with the customer.
     * Alexa, upon receiving this event, may perform some validations and eligibility checks before starting the voice conversation.
     * 
     * @note Alexa requires the customer to have accepted the OEM and network provider's terms and conditions before starting the 
     * voice conversation for activating the paid data plan.
     * 
     * Example:
     * @code{.json}
     * {
     *   "type": "ACTIVATE_PAID_PLAN"
     * }
     * @endcode
     * @param token An identifier to correlate @c connectivityEventResponse to @c sendConnectivityEvent. Default value is empty, but override this
     * to a unique non-empty value for engine to call @c connectivityEventResponse. If it is empty, engine will not call @c connectivityEventResponse.
     */
    void sendConnectivityEvent(const std::string& event, const std::string& token = "");

    /**
     * Notifies the platform implementation of the delivery status of @c sendConnectivityEvent. This is only
     * called if a non-empty token was provided in the @c sendConnectivityEvent.
     * 
     * @param [in] token The identifier that was provided in @c sendConnectivityEvent call.
     * @param [in] statusCode Represents the delivery status of event sent using @c sendConnectivityEvent.
     */
    virtual void connectivityEventResponse(const std::string& token, StatusCode statusCode){};

    /**
     * @internal
     * Sets the Engine interface delegate.
     * 
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<AlexaConnectivityEngineInterface> alexaConnectivityEngineInterface);

private:
    std::weak_ptr<AlexaConnectivityEngineInterface> m_alexaConnectivityEngineInterface;
};

}  // namespace connectivity
}  // namespace aace

#endif  // AACE_CONNECTIVITY_ALEXA_CONNECTIVITY_H
