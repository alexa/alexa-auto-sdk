/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

 import com.amazon.aace.core.config.EngineConfiguration;
 import com.amazon.aace.core.NativeRef;
 import com.amazon.aace.carControl.CarControl;

 /**
 * The @c CarControlConfiguration class is a factory interface for creating Car Control configuration objects.
 */
 public class CarControlConfiguration extends EngineConfiguration {

    private CarControlConfiguration() {
    }

    public class Zone {
        public static final String ALL                  = "zone.all";
        public static final String DRIVER               = "zone.driver";
        public static final String DRIVER_ROW           = "zone.driver.row";
        public static final String FIRST_ROW            = "zone.first.row";
        public static final String FOURTH_ROW           = "zone.fourth.row";
        public static final String FRONT                = "zone.front";
        public static final String PASSENGER            = "zone.passenger";
        public static final String PASSENGER_ROW        = "zone.passenger.row";
        public static final String REAR                 = "zone.rear";
        public static final String REAR_DRIVER          = "zone.rear.driver";
        public static final String REAR_PASSENGER       = "zone.rear.passenger";
        public static final String SECOND_ROW           = "zone.second.row";
        public static final String THIRD_ROW            = "zone.third.row";
    }

    public class Control {
        public static final String AIR_CONDITIONER    = "air.conditioner";
        public static final String AMBIENT_LIGHT      = "ambient.light";
        public static final String CLIMATE_CONTROL    = "climate";
        public static final String DOME_LIGHT         = "dome.light";
        public static final String FAN                = "fan";
        public static final String HEATER             = "heater";
        public static final String LIGHT              = "light";
        public static final String READING_LIGHT      = "reading.light";
        public static final String TRUNK_LIGHT        = "trunk.light";
        public static final String VENT               = "vent";
        public static final String WINDOW             = "window";
    }
    
    public class Instance {
        public static final String AIR_CONDITIONER_MODE     = "air.conditioner.mode";
        public static final String AIR_RECIRCULATION        = "air.recirculation";
        public static final String CLIMATE_SYNC             = "climate.sync";
        public static final String VENT_POSITION            = "vent.position";
        public static final String FAN_SPEED                = "fan.speed";
        public static final String DEFROST                  = "defrost";
        public static final String LIGHT_COLOR              = "light.color";
        public static final String TEMPERATURE              = "temperature";
    }

    public class AC {
        public static final String AUTO               = "air.conditioner.auto";
        public static final String ECONOMY            = "air.conditioner.economy";
        public static final String MANUAL             = "air.conditioner.manual";
        public static final String MAXIMUM            = "air.conditioner.maximum";
    }
    
    public class Vent {
        public static final String BODY           = "body.vents";
        public static final String MIX            = "mix.vents";
        public static final String FLOOR          = "floor.vents";
        public static final String WINDSHIELD     = "windshield.vents";
    }
    
    public class Color {
        public static final String WHITE          = "white";
        public static final String RED            = "red";
        public static final String ORANGE         = "orange";
        public static final String YELLOW         = "yellow";
        public static final String GREEN          = "green";
        public static final String BLUE           = "blue";
        public static final String INDIGO         = "indigo";
        public static final String VIOLET         = "violet";
    }

    public static CarControlConfiguration create() {
        return new CarControlConfiguration();
    }

    /**
     * Create a control endpoint.
     *
     * @param endpointId Unique identifier of the control.
     * @param zoneId Unique identifier of the zone the control is located in. Default is @c zone::ALL.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration createControl( String controlId, String zoneId ) {
        createControl( getNativeRef(), controlId, zoneId );
        return this;
    }

    /**
     * Add an @c assetId to the enclosing element. Allowed for control, toggle controller, range controller,
     * mode controller, preset and mode.
     *
     * @param assetId Identifier of the asset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addAssetId( String assetId ) {
        addAssetId( getNativeRef(), assetId );
        return this;
    }

    /**
     * Add a PowerController to the enclosing control
     *
     * @param retrievable @true if the value is evented (not use currently).
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addPowerController ( boolean retrievable ) {
        addPowerController( getNativeRef(), retrievable );
        return this;
    }

    /**
     * Add a ToggleController to the enclosing control
     *
     * @param controllerId Identifier of the controller (unique for the control).
     * @param retrievable @true if the value is evented (not use currently).
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addToggleController( String instanceId, boolean retrievable ) {
        addToggleController( getNativeRef(), instanceId, retrievable );
        return this;
    }

    /**
     * Add a RangeController to the enclosing control
     *
     * @param controllerId Identifier of the controller (unique for the control).
     * @param retrievable @true if the value is evented (not use currently).
     * @param minimum Minimal value of the controlled range.
     * @param minimum Maximal value of the controlled range.
     * @param precision Allowed precision of the value.
     * @param unit Unit of the range.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addRangeController( String instanceId, boolean retrievable, double minimum, double maximum, double precision, String unit ) {
        addRangeController( getNativeRef(), instanceId, retrievable, minimum, maximum, precision, unit );
        return this;
    }

    /**
     * Add a Preset to the enclosing RangeController (Only allowed for RangeController).
     *
     * @param value Value of the preset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addPreset( double value ) {
        addPreset( getNativeRef(), value );
        return this;
    }

    /**
     * Add a ModeController to the enclosing control
     *
     * @param controllerId Identifier of the controller (unique for the control).
     * @param retrievable @true if the value is evented (not use currently).
     * @param ordered @true if the modes are ordered.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addModeController( String instanceId, boolean retrievable, boolean ordered ) {
        addModeController( getNativeRef(), instanceId, retrievable, ordered );
        return this;
    }

    /**
     * Add a @c value to the enclosing ModeController (Only allowed for ModeController).
     *
     * @param value Value of the mode.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addValue( String value ) {
        addValue( getNativeRef(), value );
        return this;
    }

    /**
     * Create a zone.
     *
     * @param zoneId Unique identifier of the zone.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration createZone( String zoneId ) {
        createZone( getNativeRef(), zoneId );
        return this;
    }

    /**
     * Specify the path to a file that contains the predefined assets.
     *
     * @param path The path to the file
     */
    final public CarControlConfiguration addDefaultAssetsPath( String path) {
        addDefaultAssetsPath( getNativeRef(), path );
        return this;
    }

    /**
     * Specify the path to a file that defines additional assets.
     *
     * @param path The path to the file
     */
    final public CarControlConfiguration addCustomAssetsPath( String path) {
        addCustomAssetsPath( getNativeRef(), path );
        return this;
    }

    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef( long nativeRef ) {
        disposeBinder( nativeRef );
    }

    // Binder
    private native long createBinder();
    private native void disposeBinder( long nativeRef );

    // Native Platform Interfaces
    private native void createControl( long nativeRef, String controlId, String zoneId );
    private native void addAssetId ( long nativeRef, String assetId );
    private native void addPowerController( long nativeRef, boolean retrievable );
    private native void addToggleController( long nativeRef, String instanceId, boolean retrievable );
    private native void addRangeController( long nativeRef, String instanceId, boolean retrievable, double minimum, double maximum, double precision, String unit );
    private native void addPreset( long nativeRef, double value );
    private native void addModeController( long nativeRef, String instanceId, boolean retrievable, boolean ordered );
    private native void addValue( long nativeRef, String value );
    private native void createZone( long nativeRef, String zoneId );
    private native void addDefaultAssetsPath( long nativeRef, String path );
    private native void addCustomAssetsPath( long nativeRef, String path );

 };