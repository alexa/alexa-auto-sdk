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

package com.amazon.aace.deviceUsage;

import com.amazon.aace.core.PlatformInterface;

/**
 * Device Usage should be extended by the platform implementation to provide the
 * Alexa app usage on the device to Auto SDK. The interface currently supports
 * providing the network data consumption only.
 */
abstract public class DeviceUsage extends PlatformInterface {
    public DeviceUsage() {}

    /**
     * Reports the network usage data consumed by the Alexa application
     * between the @c startTimeStamp and @c endTimeStamp
     *
     * @param usage The network usage data.
     * @code{.json}
     * {
     *   "startTimeStamp" : {{LONG}},
     *   "endTimeStamp" : {{LONG}},
     *   "networkInterfaceType": "{{STRING}}",
     *   "dataPlanType" : "{{STRING}}",
     *    "bytesUsage" :{
     *        "rxBytes" : {{LONG}},
     *        "txBytes" : {{LONG}}
     *    }
     * }
     * @endcode
     * @li startTimeStamp The starting timestamp in milliseconds from when the network usage data is captured
     * @li endTimeStamp The ending timestamp in milliseconds till when the network usage data is captured
     * @li networkInterfaceType The network interface name over which the data is recorded. For e.g WIFI, MOBILE
     * @li dataPlanType The type of current data plan the device is subscribed to. This is an optional field and
     * should be provided only when the @c aace::connectivity::AlexaConnectivity platform interface is registered
     * by the platform implementation. Refer to @c aace::connectivity::AlexaConnectivity for more details about
     * dataPlanType
     * @li bytesUsage.rxBytes The received bytes over the @c networkInterfaceType
     * @li bytesUsage.txBytes The transmitted bytes over the @c networkInterfaceType
     */
    final public void reportNetworkDataUsage(String usage) {
        reportNetworkDataUsage(getNativeRef(), usage);
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
    private native void reportNetworkDataUsage(long nativeRef, String usage);
}
