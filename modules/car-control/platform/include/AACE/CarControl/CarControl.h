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

#ifndef AACE_CAR_CONTROL_CAR_CONTROL_H
#define AACE_CAR_CONTROL_CAR_CONTROL_H

#include <iostream>

#include "AACE/Core/PlatformInterface.h"

/** @file */

namespace aace {
namespace carControl {

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
class CarControl : public aace::core::PlatformInterface {
public:
    /**
     * CarControl constructor.
     */
    CarControl() = default;

    /**
     * CarControl destructor.
     */
    virtual ~CarControl();

    /**
     * Notifies the platform implementation to power on the controller identified by @c endpointId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @return @c true if successful.
     */
    virtual bool turnPowerControllerOn(const std::string& endpointId);
    /**
     * Notifies the platform implementation to power off the controller identified by @c endpointId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @return @c true if successful.
     */
    virtual bool turnPowerControllerOff(const std::string& endpointId);
    /**
     * Retrieves the power state of the controller identified by @c endpointId from the platform implementation.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [out] isOn To be set to @c true by the implementation if the controller is powered on.
     * @return @c true if @c isOn was successfully updated, @c false if the power state of the controller cannot 
     * be retrieved.
     */
    virtual bool isPowerControllerOn(const std::string& endpointId, bool& isOn);

    /**
     * Notifies the platform implementation to turn on the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @return @c true if successful.
     */
    virtual bool turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId);
    /**
     * Notifies the platform implementation to turn off the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @return @c true if successful.
     */
    virtual bool turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId);
    /**
     * Retrieves the power state of the controller identified by @c endpointId and @c controllerId from the platform
     * implementation.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] isOn To be set to @c true by the implementation if the controller is turned on.
     * @return @c true if @c isOn was successfully updated, @c false if the power state of the controller cannot 
     * be retrieved.
     */
    virtual bool isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn);

    /**
     * Notifies the platform implementation to set the range setting of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] value The new range setting.
     * @return @c true if successful.
     */
    virtual bool setRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double value);
    /**
     * Notifies the platform implementation to adjust the range setting of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] delta The delta by which to adjust the range setting.
     * @return @c true if successful.
     */
    virtual bool adjustRangeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        double delta);
    /**
     * Retrieves the range setting of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] value To be set to the current range setting by the implementation.
     * @return @c true if @c value was successfully updated, @c false if the range setting of the controller cannot 
     * be retrieved.
     */
    virtual bool getRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double& value);

    /**
     * Notifies the platform implementation to set the mode of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] value The new mode to set.
     * @return @c true if successful.
     */
    virtual bool setModeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        const std::string& value);
    /**
     * Notifies the platform implementation to adjust the mode of the controller identified by @c endpointId and
     * @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] delta The delta by which to adjust the mode.
     * @return @c true if successful.
     */
    virtual bool adjustModeControllerValue(const std::string& endpointId, const std::string& controllerId, int delta);
    /**
     * Retrieves the mode of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] value To be set to the current mode by the implementation.
     * @return @c true if @c value was successfully updated, @c false if the mode of the controller cannot 
     * be retrieved.
     */
    virtual bool getModeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        std::string& value);
};

}  // namespace carControl
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_H
