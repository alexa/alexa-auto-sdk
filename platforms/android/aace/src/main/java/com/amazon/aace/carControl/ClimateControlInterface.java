/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import com.amazon.aace.carControl.ApplianceInterface;

/**
 * ClimateControlInterface should be extended to interface the elements that can be controlled in the vehicle.
 * 
 * ClimateControlInterface provides interfaces for:
 * @li Climate control switch
 * @li Air conditioning switch and mode
 * @li Fan swicth and speed
 * @li Temperature in different zones
 */
public class ClimateControlInterface extends ApplianceInterface
{
    public ClimateControlInterface() {
        super();
    }

    /**
     * Notifies the Engine to add a switch to turn climate control on or off. 
     *
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addClimateControlSwitch() {
        return addClimateControlSwitch( getNativeObject() );
    }

    /**
     * Notifies the platform implementation to turn climate control on. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnClimateControlOn() {
        return false;
    }

    /**
     * Notifies the platform implementation to turn climate control off. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnClimateControlOff() {
        return false;
    }

    /**
     * Returns true if climate control is turned on
     *
     * @return The current state of the climate control platform implementation.
     */
    public boolean isClimateControlOn() {
        return false;
    }

    /**
     * Notifies the Engine to add a switch to turn air conditioning on or off. 
     *
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addAirConditioningSwitch() {
        return addAirConditioningSwitch( getNativeObject() );
    }
    
    /**
     * Notifies the platform implementation to turn air conditioning on. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnAirConditioningOn() {
        return false;
    }

    /**
     * Notifies the platform implementation to turn air conditioning off. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnAirConditioningOff() {
        return false;
    }

    /**
     * Returns true if air conditioning is turned on
     *
     * @return The current state of the climate control platform implementation.
     */
    public boolean isAirConditioningOn() {
        return false;
    }

    /**
     * Air conditioning modes
     */
    public enum AirConditioningMode {
        /**
         * Manual control
         */
        MANUAL("MANUAL"),
        /**
         * Automatic control
         */
        AUTO("AUTO");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AirConditioningMode( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    };

    /**
     * Notifies the Engine to add a selector to choose the air conditioning mode. 
     *
     * @param [in] modes Vector of supported @c AirConditioningMode
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addAirConditioningModeSelector( AirConditioningMode[] modes ) {
        return addAirConditioningModeSelector( getNativeObject(), modes );
    }

    /**
     * Notifies the platform implementation to select a air conditioning mode. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @param [in] mode The @c AirConditioningMode to select
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean setAirConditioningMode( ClimateControlInterface.AirConditioningMode mode ) {
        return false;
    }

    /**
     * Returns the current air conditioning mode.
     *
     * @return The current air conditioning mode of the climate control platform implementation. 
     */
    public ClimateControlInterface.AirConditioningMode getAirConditioningMode() {
        return ClimateControlInterface.AirConditioningMode.MANUAL;
    }

    /**
     * Fan zones
     */
    public enum FanZone {
        /**
         * All zones or the entire vehicle
         */
        ALL("ALL");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private FanZone( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    };

    /**
     * Notifies the Engine to add a switch to turn fan on or off for a specific @c zone. 
     *
     * @param [in] zone Identifier of the zone
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addFanSwitch( FanZone zone ) {
        return addFanSwitch( getNativeObject(), zone );
    }

    /**
     * Notifies the platform implementation to turn the fan on for a specific @c zone. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnFanOn( FanZone zone ) {
        return false;
    }

    /**
     * Notifies the platform implementation to turn the fan off for a specific @c zone. The platform implementation should change the state in the 
     * vehicle accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean turnFanOff( FanZone zone ) {
        return false;
    }

    /**
     * Returns true if the fan is turned on in a specific @c zone.
     *
     * @param [in] zone Identifier of the zone
     * @return The current state of the fan platform implementation.
     */
    public boolean isFanOn( FanZone zone ) {
        return false;
    }

    /**
     * Notifies the Engine to add a fan speed control for a specific @c zone. 
     *
     * @param [in] zone Identifier of the zone
     * @param [in] minimum Minimum value of the fan speed
     * @param [in] maximum Maximum value of the fan speed
     * @param [in] precision Precision of the controlled value
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addFanSpeedControl( FanZone zone, double minimum, double maximum, double precision ) {
        return addFanSpeedControl( getNativeObject(), zone, minimum, maximum, precision );
    }

    /**
     * Notifies the platform implementation that the value of the fan speed for a specific @c zone
     * should be set to @c value. The platform implementation should change the value accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @param [in] value The adjustment to apply to the fan speed. @c value will be
     * within the specified range.
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean setFanSpeed( FanZone zone, double value ) {
        return false;
    }

    /**
     * Notifies the platform implementation that the value of the fan speed for a specific @c zone
     * should be adjusted by @c delta. The platform implementation should change the value accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @param [in] delta The adjustment to apply to the fan speed. @c delta will leave the value
     * within the specified range.
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean adjustFanSpeed( FanZone zone, double delta ) {
        return false;
    }

    /**
     * Returns the platform value of the fan speed for a specific @c zone.
     *
     * @param [in] zone Identifier of the zone
     * @return The current value of the fan speed for zone @c zone provided by the platform implementation.
     */
    public double getFanSpeed( FanZone zone ) {
        return 0;
    }

    /**
     * Temperature zones
     */
    public enum TemperatureZone {
        /**
         * All zones or the entire vehicle
         */
        ALL("ALL");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private TemperatureZone( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    };

    /**
     * Temperature units
     */
    public enum TemperatureUnit {
        /**
         * Temperature in Celsius
         */
        CELSIUS("CELSIUS"),
        /**
         * Temperature in Fahrenheit
         */
        FAHRENHEIT("FAHRENHEIT");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private TemperatureUnit( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    };

    /**
     * Notifies the Engine to add a temperature controller for a specific @c zone. 
     *
     * @param [in] zone Identifier of the zone
     * @param [in] minimum Minimum value of the temperature
     * @param [in] maximum Maximum value of the temperature
     * @param [in] precision Precision of the controlled value
     * @param [in] unit Unit of the temperature value
     * @return @c true if the Engine successfully added the controller, else @c false
     */
    public boolean addTemperatureControl( TemperatureZone zone, double minimum, double maximum, double precision, TemperatureUnit unit ) {
        return addTemperatureControl( getNativeObject(), zone, minimum, maximum, precision, unit );
    }

    /**
     * Notifies the platform implementation that the value of the temperature for the specified zone @c zone
     * should be set to @c value. The platform implementation should change the value accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @param [in] value The adjustment to apply to the temperature. @c value will be
     * within the specified range.
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean setTemperature( TemperatureZone zone, double value ) {
        return false;
    }

    /**
     * Notifies the platform implementation that the value of the temperature for the specified zone @c zone
     * should be adjusted by @c delta. The platform implementation should change the value accordingly.
     *
     * @param [in] zone Identifier of the zone
     * @param [in] delta The adjustment to apply to the temperature. @c delta will leave the value
     * within the specified range.
     * @return @c true if the platform implementation successfully handled the call, else @c false
     */
    public boolean adjustTemperature( TemperatureZone zone, double delta ) {
        return false;
    }

    /**
     * Returns the platform value of the temperature for the specified zone @c zone
     *
     * @param [in] zone Identifier of the zone
     * @return The current value of the temperature for zone @c zone provided by the platform implementation.
     */
    public double getTemperature( TemperatureZone zone ) {
        return 0;
    }
    
    private native boolean addClimateControlSwitch( long nativeObject );
    private native boolean addAirConditioningSwitch( long nativeObject );
    private native boolean addAirConditioningModeSelector( long nativeObject, AirConditioningMode[] modes );
    private native boolean addFanSwitch( long nativeObject, FanZone zone );
    private native boolean addFanSpeedControl( long nativeObject, FanZone zone, double minimum, double maximum, double precision );
    private native boolean addTemperatureControl( long nativeObject, TemperatureZone zone, double minimum, double maximum, double precision, TemperatureUnit unit );

}

// END OF FILE
