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

#ifndef AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_SERVICE_INTERFACE_H
#define AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_SERVICE_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace carControl {

/**
 * Interface for the car control engine, with responsibilities as follows:
 *  @li Provides access to invoke the capability controllers of the @c CarControl platform interface
 *  @li Register and query whether a @c CarControlLocalService is available, which disables some functionaility of
 *  the online-only car control engine for delegation to the local service
 */
class CarControlServiceInterface {
public:
    /**
     * Power on the controller identified by @c endpointId.
     *
     * @param [in] endpointId The unique identifier of the endpoint
     * @return @c true if successful.
     */
    virtual bool turnPowerControllerOn(const std::string& endpointId) = 0;
    /**
     * Power off the controller identified by @c endpointId.
     *
     * @param [in] endpointId The unique identifier of the endpoint
     * @return @c true if successful.
     */
    virtual bool turnPowerControllerOff(const std::string& endpointId) = 0;
    /**
     * Retrieve the power state of the controller identified by @c endpointId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [out] isOn Set to true by the implementation if the control is powered on.
     * @return @c true if successful.
     */
    virtual bool isPowerControllerOn(const std::string& endpointId, bool& isOn) = 0;

    /**
     * Turn on the controller identified by @c endpointId and  @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @return @c true if successful.
     */
    virtual bool turnToggleControllerOn(const std::string& endpointId, const std::string& instance) = 0;
    /**
     * Turn off the controller identified by @c endpointId and  @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @return @c true if successful.
     */
    virtual bool turnToggleControllerOff(const std::string& endpointId, const std::string& instance) = 0;
    /**
     * Retrieve the power state of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] isOn To be set to @c true by the implementation if the control is turned on.
     * @return @c true if successful.
     */
    virtual bool isToggleControllerOn(const std::string& endpointId, const std::string& instance, bool& isOn) = 0;

    /**
     * Set the range value of the controller identified by @c endpointId and  @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] value The new range value to set.
     * @return @c true if successful.
     */
    virtual bool setRangeControllerValue(const std::string& endpointId, const std::string& instance, double value) = 0;
    /**
     * Adjust the range value of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] delta The delta by which to adjust the range value.
     * @return @c true if successful.
     */
    virtual bool adjustRangeControllerValue(
        const std::string& endpointId,
        const std::string& instance,
        double delta) = 0;
    /**
     * Retrieve the range value of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] value To be set to the current range value by the implementation.
     * @return @c true if successful.
     */
    virtual bool getRangeControllerValue(const std::string& endpointId, const std::string& instance, double& value) = 0;

    /**
     * Set the mode of the control identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] value The new mode to set.
     * @return @c true if successful.
     */
    virtual bool setModeControllerValue(
        const std::string& endpointId,
        const std::string& instance,
        const std::string& value) = 0;
    /**
     * Adjust the mode of the control identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [in] delta The delta by which to adjust the mode.
     * @return @c true if successful.
     */
    virtual bool adjustModeControllerValue(const std::string& endpointId, const std::string& instance, int delta) = 0;
    /**
     * Retrieve the range value of the controller identified by @c endpointId and @c controllerId.
     *
     * @param [in] endpointId The unique identifier of the endpoint.
     * @param [in] controllerId The unique identifier of the controller.
     * @param [out] value To be set to the current mode by the implementation.
     * @return @c true if successful.
     */
    virtual bool getModeControllerValue(
        const std::string& endpointId,
        const std::string& instance,
        std::string& value) = 0;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_SERVICE_INTERFACE_H
