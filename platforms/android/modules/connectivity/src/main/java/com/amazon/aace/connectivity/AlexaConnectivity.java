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

package com.amazon.aace.connectivity;

import com.amazon.aace.core.PlatformInterface;

/**
 * AlexaConnectivity should be extended to send network connectivity status and data plan status to Alexa.
 * The information sent determines what Alexa features are available to the device.
 */
abstract public class AlexaConnectivity extends PlatformInterface {
    public AlexaConnectivity() {}

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
     * `termsStatus`        | Indicates whether the customer has accepted the terms and conditions of the OEM and
     * network provider. | `String`  | Optional
     *
     * **managedProvider**
     *
     * Name                 | Description                                                       | Type      | Status
     * -------------------- | ----------------------------------------------------------------- | --------- | ---------
     * `type`               | Type of the managed provider, MANAGED or NOT_MANAGED (see below). | `String`  | Required
     * `id`                 | Specifies the name of the provider that manages connectivity. The only accepted value is
     * AMAZON. | `String`  | Required*
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
     *   "termsStatus": "ACCEPTED"
     * }
     * @endcode
     *
     * Note: You may return an empty string to indicate that connectivity state is not available.
     *
     * @return A @c String representing the connectivity state in structured JSON format.
     */
    public String getConnectivityState() {
        return "";
    }

    /**
     * Notifies the Engine of a change in the connectivity state. The Engine calls @c getConnectivityState
     * to retrieve the the connectivity state and communicate any changes to Alexa.
     *
     * @return @c true if connectivity state was processed successfully, @c false otherwise.
     */
    final public boolean connectivityStateChange() {
        return connectivityStateChange(getNativeRef());
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean connectivityStateChange(long nativeObject);
}
