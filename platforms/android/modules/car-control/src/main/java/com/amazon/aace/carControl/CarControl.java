/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.carControl;

import com.amazon.aace.core.PlatformInterface;

/**
 * CarControl should be extended to interface the elements that can be controlled in the vehicle. Each controllable
 * element is an 'endpoint' with a unique @c endpointId. @c CarControl provides interfaces for four types of
 * controllers. A custom combination of these controllers describes controls of the entire vehicle:
 *  @li Power Controller: Controls the power state of an endpoint. A maximum of one Power Controller is allowed per
 *      endpoint, identified by @c endpointId.
 *  @li Toggle Controller: Controls the power state of a particular setting of an endpoint. Multiple Toggle Controllers
 *      for unique settings are allowed per endpoint, each identified by the combination of @c endpointId and
 *      @c controllerId.
 *  @li Range Controller: Controls the settings of an endpoint that can be accessed with a range of values. Multiple
 *      Range Controllers for unique settings are allowed per endpoint, each identified by the combination of
 *      @c endpointId and @c controllerId.
 *  @li Mode Controller: Controls the settings of an endpoint that can be accessed with a set of modes. Multiple
 *      Mode Controllers for unique settings are allowed per endpoint, each identified by the combination of
 *      @c endpointId and @c controllerId.
 */
abstract public class CarControl extends PlatformInterface {
    /**
     * CarControl constructor.
     */
    public CarControl() {}

    /**
     * Notifies the platform implementation to power on the controller identified by @c endpointId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @exception Exception The controller cannot be powered on.
     */
    public void turnPowerControllerOn(String endpointId) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Notifies the platform implementation to power off the controller identified by @c endpointId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @exception Exception The controller cannot be powered off.
     */
    public void turnPowerControllerOff(String endpointId) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Retrieves the power state of the controller identified by @c endpointId from the platform implementation.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @exception Exception The power state of the controller identified by @c endpointId cannot be retrieved.
     * @return @c true if the power controller is on.
     */
    public boolean isPowerControllerOn(String endpointId) throws Exception {
        throw new Exception("Invalid");
    }

    /**
     * Notifies the platform implementation to turn on the controller identified by @c endpointId and @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @exception Exception The controller cannot be powered on.
     */
    public void turnToggleControllerOn(String endpointId, String controllerId) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Notifies the platform implementation to turn off the controller identified by @c endpointId and @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @exception Exception The controller cannot be powered off.
     */
    public void turnToggleControllerOff(String endpointId, String controllerId) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Retrieves the power state of the controller identified by @c endpointId and @c controllerId from the platform
     * implementation.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @exception Exception The power state of the controller identified by @c endpointId and @c controllerId cannot be
     * retrieved.
     * @return @c true if the controller is on.
     */
    public boolean isToggleControllerOn(String endpointId, String controllerId) throws Exception {
        throw new Exception("Invalid");
    }

    /**
     * Notifies the platform implementation to set the range setting of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @param value The new range setting.
     * @exception Exception The range setting of the controller cannot be set.
     */
    public void setRangeControllerValue(String endpointId, String controllerId, double value) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Notifies the platform implementation to adjust the range setting of the controller identified by @c endpointId
     * and
     * @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @param delta The delta by which to adjust the range setting.
     * @exception Exception The range setting of the controller cannot be adjusted.
     */
    public void adjustRangeControllerValue(String endpointId, String controllerId, double delta) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Retrieves the range value of the controller identified by @c endpointId and @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @exception Exception The range setting of the controller identified by @c endpointId and @c controllerId cannot
     * be retrieved.
     * @return The range setting of the controller.
     */
    public double getRangeControllerValue(String endpointId, String controllerId) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Notifies the platform implementation to set the mode of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @param value The new mode to set.
     * @exception Exception The mode of the controller cannot be set.
     */
    public void setModeControllerValue(String endpointId, String controllerId, String value) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Notifies the platform implementation to adjust the mode of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @param delta The delta by which to adjust the mode.
     * @exception Exception The mode of the controller cannot be adjusted.
     */
    public void adjustModeControllerValue(String endpointId, String controllerId, int delta) throws Exception {
        throw new Exception("Invalid");
    }
    /**
     * Retrieves the mode of the controller identified by @c endpointId and @c controllerId.
     *
     * @param endpointId The unique identifier of the endpoint.
     * @param controllerId The unique identifier of the controller.
     * @exception Exception The mode of the controller identified by @c endpointId and @c controllerId cannot be
     * retrieved.
     * @return The mode of the controller.
     */
    public String getModeControllerValue(String endpointId, String controllerId) throws Exception {
        throw new Exception("Invalid");
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
}