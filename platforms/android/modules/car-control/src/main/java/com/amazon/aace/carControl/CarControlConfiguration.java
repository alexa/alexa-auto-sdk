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

import com.amazon.aace.carControl.CarControl;
import com.amazon.aace.core.NativeRef;
import com.amazon.aace.core.config.EngineConfiguration;

/**
 * The @c CarControlConfiguration class programmatically generates the "aace.carControl"
 * @c com.amazon.aace.core.config.EngineConfiguration. @b "aace.carControl" configuration specifies additional endpoints
 * that are connected and controllable through the root endpoint that maintains the connection to the Alexa service.
 * Each connected endpoint represents a controllable component of the vehicle and should be configured with any of the
 * supported capabilities (i.e. @b PowerController and primitives @b ModeController, @b ToggleController,
 * @b RangeController ) to model how the endpoint can be controlled with voice.
 *
 * Using the methods on an instance of this class produces configuration data in the format of the following example:
 *
 * @code{.json}
 * {
 * "aace.carControl": {
 *     "endpoints": [
 *         {
 *             "endpointId": "default.ac",
 *             "endpointResources": {
 *                 "friendlyNames": [
 *                     {
 *                         "@type": "asset",
 *                         "value": {
 *                             "assetId": "Alexa.Automotive.DeviceName.AirConditioner"
 *                         }
 *                     }
 *                 ]
 *             },
 *             "capabilities": [
 *                 {
 *                     "type": "AlexaInterface",
 *                     "interface": "Alexa.PowerController",
 *                     "version": "3",
 *                     "properties": {
 *                         "supported": [
 *                             {
 *                                 "name": "powerState"
 *                             }
 *                         ],
 *                         "proactivelyReported": false,
 *                         "retrievable": false
 *                     }
 *                 },
 *                 {
 *                     "type": "AlexaInterface",
 *                     "interface": "Alexa.ModeController",
 *                     "version": "3",
 *                     "instance": "intensity",
 *                     "capabilityResources": {
 *                         "friendlyNames": [
 *                             {
 *                                 "@type": "asset",
 *                                 "value": {
 *                                     "assetId": "Alexa.Automotive.Setting.Intensity"
 *                                 }
 *                             }
 *                         ]
 *                     },
 *                     "properties": {
 *                         "supported": [
 *                             {
 *                                 "name": "mode"
 *                             }
 *                         ],
 *                         "proactivelyReported": false,
 *                         "retrievable": false
 *                     },
 *                     "configuration": {
 *                         "ordered": true,
 *                         "supportedModes": [
 *                             {
 *                                 "value": "LOW",
 *                                 "modeResources": {
 *                                     "friendlyNames": [
 *                                         {
 *                                             "@type": "asset",
 *                                             "value": {
 *                                                 "assetId": "Alexa.Automotive.Value.Low"
 *                                             }
 *                                         },
 *                                         {
 *                                             "@type": "asset",
 *                                             "value": {
 *                                                 "assetId": "Alexa.Automotive.Value.Minimum"
 *                                             }
 *                                         }
 *                                     ]
 *                                 }
 *                             },
 *                             {
 *                                 "value": "MEDIUM",
 *                                 "modeResources": {
 *                                     "friendlyNames": [
 *                                         {
 *                                             "@type": "asset",
 *                                             "value": {
 *                                                 "assetId": "Alexa.Automotive.Value.Medium"
 *                                             }
 *                                         }
 *                                     ]
 *                                 }
 *                             },
 *                             {
 *                                 "value": "HIGH",
 *                                 "modeResources": {
 *                                     "friendlyNames": [
 *                                         {
 *                                             "@type": "asset",
 *                                             "value": {
 *                                                 "assetId": "Alexa.Automotive.Value.High"
 *                                             }
 *                                         },
 *                                         {
 *                                             "@type": "asset",
 *                                             "value": {
 *                                                 "assetId": "Alexa.Automotive.Value.Maximum"
 *                                             }
 *                                         }
 *                                     ]
 *                                 }
 *                             }
 *                         ]
 *                     },
 *                     "semantics" : {
 *                          "actionMappings": [
 *                               {
 *                                  "@type": "ActionsToDirective",
 *                                  "actions": ["Alexa.Actions.Raise"],
 *                                   "directive": {
 *                                      "name": "AdjustMode",
 *                                      "payload": {"modeDelta": 1}
 *                                   }
 *                               },
 *                               {
 *                                  "@type": "ActionsToDirective",
 *                                  "actions": ["Alexa.Actions.Lower"],
 *                                   "directive": {
 *                                      "name": "AdjustMode",
 *                                      "payload": {"modeDelta": -1}
 *                                   }
 *                               }
 *                          ]
 *                     }
 *                 }
 *             ]
 *         }
 *     ],
 *     "zones": [
 *         {
 *             "zoneId": "zone.default",
 *             "zoneResources": {
 *                 "friendlyNames": [
 *                     {
 *                         "@type": "asset",
 *                         "value": {
 *                             "assetId": "Alexa.Automotive.Location.All"
 *                         }
 *                     }
 *                 ]
 *             },
 *             "members": [
 *                 {
 *                     "endpointId": "default.ac"
 *                 }
 *             ]
 *         }
 *     ],
 *     "defaultZoneId" : "zone.default",
 *     "assets" : {
 *          "customAssetsPath" : "/opt/AAC/assets-3P.json"
 *      }
 * }
 * }
 * @endcode
 *
 * @c note This class acts as a configuration builder, so the method calls must be ordered to produce the intended
 * configuration.
 */
public class CarControlConfiguration extends EngineConfiguration {
    private CarControlConfiguration() {}

    /**
     * Supported action IDs used to create action mapping 'semantic annotations' for capability instances on endpoints.
     * Action mapping semantic annotations enable mapping specific additional utterances to the directives of individual
     * capability instances.
     *
     * @note These are the only supported action IDs.
     * @sa
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/capability-primitives.html#semantic-annotation
     * for additional reference, but note that state mappings are not supported.
     */
    public class Action {
        /// "Close" action.
        public static final String CLOSE = "Alexa.Actions.Close";
        /// "Lower" action.
        public static final String LOWER = "Alexa.Actions.Lower";
        /// "Open" action.
        public static final String OPEN = "Alexa.Actions.Open";
        /// "Raise" action.
        public static final String RAISE = "Alexa.Actions.Raise";
    }

    /**
     * Create a @c CarControlConfiguration instance.
     */
    public static CarControlConfiguration create() {
        return new CarControlConfiguration();
    }

    /**
     * Begin an endpoint definition using the specified endpoint ID. This creates a single entry in the "endpoints"
     * array of 'aace.carControl'.
     *
     * @note Do not use the following format for the @a endpointId:
     * <clientId>::<productId>::<serialNumber>::[-<extEndpoint>].
     * The Engine internally prepends the 3-part device prefix to your specified @a endpointId before sending the
     * configuration to the cloud in an @b AddOrUpdateReport event. Configuring the full ID directly results in
     * duplication and excess characters.
     *
     * @param endpointId The unique identifier for the endpoint.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration createEndpoint(String endpointId) {
        createEndpoint(getNativeRef(), endpointId);
        return this;
    }

    /**
     * Add an asset ID to the enclosing element. This can be called to add an asset ID to the friendly names list for
     * the following elements:
     *  @li The 'endpointResources' of an endpoint definition
     *  @li The 'capabilityResources' of a ToggleController, RangeController, or ModeController definition
     *  @li The 'presetResources' of a RangeController preset definition
     *  @li The 'modeResources' of a ModeController mode definition
     *
     * @note Specify only valid asset IDs. See @c CarControlAssets.java for a full list of the asset IDs supported by
     * default.
     *
     * @param assetId The identifier of the asset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addAssetId(String assetId) {
        addAssetId(getNativeRef(), assetId);
        return this;
    }

    /**
     * Add a @b PowerController capability to the enclosing endpoint. Only one instance is allowed per endpoint.
     *
     * @sa https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-powercontroller.html.
     *
     * @param retrievable Whether the state of this instance may be retrieved by the Alexa service. Only @c false
     *        is supported.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addPowerController(boolean retrievable) {
        addPowerController(getNativeRef(), retrievable);
        return this;
    }

    /**
     * Add a @b ToggleController capability to the enclosing endpoint. Multiple instances are allowed per endpoint.
     * Call @c addAsset() to add friendly names to the 'capabilityResources' of this instance.
     *
     * @sa https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-togglecontroller.html.
     *
     * @param instanceId The identifier of this @b ToggleController instance. Must be unique with respect to the
     *        enclosing endpoint.
     * @param retrievable Whether the state of this instance may be retrieved by the Alexa service. Only @c false
     *        is supported.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addToggleController(String instanceId, boolean retrievable) {
        addToggleController(getNativeRef(), instanceId, retrievable);
        return this;
    }

    /**
     * Maps the specified action IDs to the @b TurnOn directive for the enclosing @b ToggleController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl.turnToggleControllerOn() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b TurnOn directive.
     */
    final public CarControlConfiguration addActionTurnOn(String[] actions) {
        addActionTurnOn(getNativeRef(), actions);
        return this;
    }

    /**
     * Maps the specified action IDs to the @b TurnOff directive for the enclosing @b ToggleController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl.turnToggleControllerOn() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b TurnOff directive.
     */
    final public CarControlConfiguration addActionTurnOff(String[] actions) {
        addActionTurnOff(getNativeRef(), actions);
        return this;
    }

    /**
     * Add a @b RangeController capability to the enclosing endpoint. Multiple instances are allowed per endpoint.
     * Call @c addAsset() to add friendly names to the 'capabilityResources' of this instance.
     *
     * @sa https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-rangecontroller.html
     *
     * @param instanceId The identifier of this @b RangeController instance. Must be unique with respect to the
     *        enclosing endpoint.
     * @param retrievable Whether the state of this instance may be retrieved by the Alexa service. Only @c false
     *        is supported.
     * @param minimum The minimum value of the range supported by this instance.
     * @param minimum The maximum value of the range supported by this instance.
     * @param precision The amount by which the set value changes when iterating through the range.
     * @param unit The unit of the measure.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addRangeController(
            String instanceId, boolean retrievable, double minimum, double maximum, double precision, String unit) {
        addRangeController(getNativeRef(), instanceId, retrievable, minimum, maximum, precision, unit);
        return this;
    }

    /**
     * Add a 'preset' to the enclosing @b RangeController instance. A preset describes a value that can be invoked by
     * name. Call @c addAsset() to add friendly names to 'presetResources' of this preset.
     *
     * @param value The value within the range that has an associated named preset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addPreset(double value) {
        addPreset(getNativeRef(), value);
        return this;
    }

    /**
     * Maps the specified action IDs to the @b SetRangeValue directive for the enclosing @b RangeController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl.setRangeControllerValue() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b SetRangeValue directive.
     * @param value The @a value argument of @c setRangeControllerValue(). The value must be within the configured
     *        range of this @b RangeController instance.
     */
    final public CarControlConfiguration addActionSetRange(String[] actions, double value) {
        addActionSetRange(getNativeRef(), actions, value);
        return this;
    }

    /**
     * Maps the specified action IDs to @b AdjustRangeValue directive for the enclosing @b RangeController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl/adjustRangeControllerValue() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b AdjustRangeValue directive.
     * @param delta The @a delta argument of @c adjustRangeControllerValue(). The absolute value must be <= (max - min)
     *        configured for this @b RangeController instance.
     */
    final public CarControlConfiguration addActionAdjustRange(String[] actions, double delta) {
        addActionAdjustRange(getNativeRef(), actions, delta);
        return this;
    }

    /**
     * Add a @b ModeController capability to the enclosing endpoint. Multiple instances are allowed per endpoint.
     * Call @c addAsset() to add friendly names to the 'capabilityResources' of this instance.
     *
     * @sa https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-modecontroller.html
     *
     * @param instanceId The identifier of this @b ModeController instance. Must be unique with respect to the
     *        enclosing endpoint.
     * @param retrievable Whether the state of this instance may be retrieved by the Alexa service. Only @c false
     *        is supported.
     * @param ordered Whether the modes are ordered, enabling iteration through them using the @b AdjustMode directive.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addModeController(String instanceId, boolean retrievable, boolean ordered) {
        addModeController(getNativeRef(), instanceId, retrievable, ordered);
        return this;
    }

    /**
     * Add a supported mode 'value' to the enclosing @b ModeController instance. If @a ordered is true, the order in
     * which modes are created with calls to this method determines the ordering for iteration by voice. Call
     * @c addAsset() to add friendly names to 'modeResources' of this mode.
     *
     * @param value The identifier of the mode.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addValue(String value) {
        addValue(getNativeRef(), value);
        return this;
    }

    /**
     * Maps the specified action IDs to the @b SetMode directive for the enclosing @b ModeController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl.setModeControllerValue() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b SetMode directive.
     * @param value The @a value argument of @c setModeControllerValue() specifying the mode to set. The value must be
     *        one configured for this @b ModeController instance.
     */
    final public CarControlConfiguration addActionSetMode(String[] actions, String value) {
        addActionSetMode(getNativeRef(), actions, value);
        return this;
    }

    /**
     * Maps the specified action IDs to the @b AdjustMode directive for the enclosing @b ModeController. This adds a
     * 'semantics.actionMappings' entry to its definition.
     * @c com.amazon.aace.carControl.CarControl.adjustModeControllerValue() is invoked when the user says the
     * additional utterances enabled by the specified action ID(s).
     *
     * @note This may only be used if @a ordered is @c true for this @b ModeController instance.
     *
     * @note Use a specific action ID only once per endpoint.
     *
     * @note Only the action IDs defined in @c com.amazon.aace.carControl.CarControlConfiguration.Action are supported.
     *
     * @param actions The action IDs to map to the @b AdjustMode directive.
     * @param delta The @a delta argument of @c adjustModeControllerValue() specifying the number of modes to advance
     *        from the current mode setting.
     */
    final public CarControlConfiguration addActionAdjustMode(String[] actions, int delta) {
        addActionAdjustMode(getNativeRef(), actions, delta);
        return this;
    }

    /**
     * Begin a zone definition using the specified zone ID. This creates a single entry in the "zones" array of
     * 'aace.carControl'. Call @c addMembers() to add endpoint IDs as members of this zone.
     *
     * @param zoneId The unique identifier for the zone.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration createZone(String zoneId) {
        createZone(getNativeRef(), zoneId);
        return this;
    }

    /**
     * Add the specified endpoint IDs to the definition of the enclosing zone.
     *
     * @note An endpoint ID added to this zone instance may belong to other zones as well.
     *
     * @param endpointIds The IDs of the endpoints belonging to this zone. Endpoint IDs used must correspond to
     *        endpoints created with @c createEndpoint().
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration addMembers(String[] endpointIds) {
        addMembers(getNativeRef(), endpointIds);
        return this;
    }

    /**
     * Set the default zone to the specified zone ID. This allows the endpoints in the specified default zone to take
     * precedence over endpoints not in the default zone when the user does not specify any zone in the utterance.
     *
     * @note There can only be one default zone. Multiple calls to this method will override the previously set default.
     *
     * @param zoneId The ID of the default zone.
     * @return @c CarControlConfiguration to allow chaining.
     */
    final public CarControlConfiguration setDefaultZone(String zoneId) {
        setDefaultZone(getNativeRef(), zoneId);
        return this;
    }

    /**
     * Specify the path to a file that defines the default assets. This creates an "assets.defaultAssetsPath" entry in
     * 'aace.carControl'.
     *
     * @note Using this method is not necessary or recommended. The default assets are already defined internally.
     * including all supported locales and use the asset IDs present in @c CarControlAssets.h. This method can override
     * the internal asset definitions in online-only use cases (without the LVC extension).
     *
     * @param path The path to the file defining the default assets.
     */
    final public CarControlConfiguration addDefaultAssetsPath(String path) {
        addDefaultAssetsPath(getNativeRef(), path);
        return this;
    }

    /**
     * Specify the path to a file that defines additional assets. This creates an "assets.customAssetsPath" entry in
     * 'aace.carControl'. Specify a path to additional assets if you have endpoints that cannot be modeled using the
     * default asset IDs present in @c CarControlAssets.java.
     *
     * @param path The path to the file defining additional assets.
     */
    final public CarControlConfiguration addCustomAssetsPath(String path) {
        addCustomAssetsPath(getNativeRef(), path);
        return this;
    }

    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Binder
    private native long createBinder();
    private native void disposeBinder(long nativeRef);

    // Native Platform Interfaces
    private native void createEndpoint(long nativeRef, String endpointId);
    private native void addAssetId(long nativeRef, String assetId);
    private native void addPowerController(long nativeRef, boolean retrievable);
    private native void addToggleController(long nativeRef, String instanceId, boolean retrievable);
    private native void addActionTurnOn(long nativeRef, String[] actions);
    private native void addActionTurnOff(long nativeRef, String[] actions);
    private native void addRangeController(long nativeRef, String instanceId, boolean retrievable, double minimum,
            double maximum, double precision, String unit);
    private native void addPreset(long nativeRef, double value);
    private native void addActionSetRange(long nativeRef, String[] actions, double value);
    private native void addActionAdjustRange(long nativeRef, String[] actions, double delta);
    private native void addModeController(long nativeRef, String instanceId, boolean retrievable, boolean ordered);
    private native void addValue(long nativeRef, String value);
    private native void addActionSetMode(long nativeRef, String[] actions, String value);
    private native void addActionAdjustMode(long nativeRef, String[] actions, int delta);
    private native void createZone(long nativeRef, String zoneId);
    private native void addMembers(long nativeRef, String[] endpointIds);
    private native void setDefaultZone(long nativeRef, String zoneId);
    private native void addDefaultAssetsPath(long nativeRef, String path);
    private native void addCustomAssetsPath(long nativeRef, String path);
};