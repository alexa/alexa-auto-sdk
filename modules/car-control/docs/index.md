# Car Control Module

## Overview

The Alexa Auto SDK `Car Control` module enables you to build a custom experience that allows users to use Alexa to voice-control vehicle features. The following concepts comprise the `Car Control` module APIs:

### Endpoints

The head unit device acting as an Alexa Auto SDK client is an "endpoint" that connects to the Alexa service. Other Auto SDK modules, such as `Alexa`, configure capabilities on this "root" or "default" endpoint because the capabilities pertain to the head unit itself. The `Car Control` module enables the default Auto SDK client endpoint to act as a proxy to receive events and directives on behalf of connected endpoints. You can configure a separate endpoint for every vehicle component that the head unit can control through device-level connections. This enables the user to target individual vehicle components directly with utterances like "Alexa, turn on the AC" or "Alexa, set the temperature to 65."

### Capabilities

In the utterance "Alexa, turn on the AC", "turn on" corresponds to a specific capability configured for the "AC" endpoint. Defining an endpoint declares a vehicle feature to be controllable, and defining capabilities on the endpoint declares *how* the endpoint can be controlled. 

Car Control supports four capability interfaces that can be declared alone or in combination for a particular endpoint to model its individual control experience:

* **Power Controller** controls the overall power state of an endpoint. For example, configuring an "AC" endpoint with a Power Controller capability enables utterances such as "Alexa, turn on the AC" and "Alexa, power off the AC".
  
* **Toggle Controller** controls a particular named property of an endpoint that can be turned on and off. For example, configuring a "windshield" endpoint with a "defroster" Toggle Controller capability instance enables utterances such as "Alexa, turn on the windshield defroster." 
  
* **Mode Controller** controls a particular named property of an endpoint that can be set to a discrete value from a defined set of values. For example, if an ambient light endpoint has red and green color settings, configuring an "ambient light" endpoint with a "color" Mode Controller capability instance enables utterances such as "Alexa, set the ambient light color to red" and "Alexa, change the ambient light to green."
  
* **Range Controller** controls a particular named property of an endpoint that can be set to a numeric value within a range. For example, if a fan endpoint has a speed property with settings 1 through 3, configuring a "fan" endpoint with a "speed" Range Controller capability instance enables utterances such as "Alexa, set the fan speed to 2." You can configure names, such as "medium", for a range value to enable additional utterances such as "Alexa, set the fan to medium" to set the fan speed setting to 2. 

#### Capability Primitives and Semantic Annotations

Toggle Controller, Mode Controller, and Range Controller are known as "capability primitives." You can use multiple instances of the same capability primitive interface on an endpoint under different instance names. For example, a heater endpoint might have intensity and position properties that are both best modeled as modes. You can declare an "intensity" Mode Controller instance and a "position" Mode Controller instance on the same "heater" endpoint so the user can target each property separately. 

To provide intuitive experiences for users, capability primitives offer "semantic annotations" for the devices to map specific utterances to the behaviors of capability instances. For example, if the vehicle uses a Range Controller to control a window, a user would prefer to say "Alexa, open the window" over the default utterances of the Range Controller such as "Alexa, set the window height to 0". For any endpoint to which the "open", "close", "raise", or "lower" concepts apply, you can configure the capability primitive instances of the endpoint with a "semantics" object that maps user utterances for these actions to the appropriate capability directives. Each action (e.g., "open") is allowed only once per endpoint since the action expresses intent to control the endpoint as a whole.

The actions specified in configuration are action IDs rather than literal strings, which ensures Alexa recognizes all synonyms and translations for the action in the user utterance. The supported actions are "Alexa.Actions.Open", "Alexa.Actions.Close", "Alexa.Actions.Raise", and "Alexa.Actions.Lower."
    
### Zones

Each endpoint can belong to zero, one, or many "zones." Zones, configured with member endpoints, define named regions of the vehicle and allow users to target endpoints by location. Zones are essential for unambiguous targeting of endpoints that have friendly names that overlap with other endpoints. For example, defining "driver" and "passenger" zones and assigning distinct "seat" endpoints to each allows proper control of the "driver seat" and the "passenger seat" independently. 

Assigning one zone in particular as the "default" enables endpoints in this zone to take precedence over endpoints sharing the same friendly name but not in the default zone when the user does not specify a zone in the utterance. This is useful for distinguishing "zoneless" endpoints from "zoned" endpoints with the same name when it is most likely that the user intends to target the "zoneless" one. For example, consider a vehicle with zone IDs "zone.all", "zone.rear", and "zone.left" with a distinct fan endpoint in each zone. If the user says "Alexa, turn on the fan", it is most likely that he wants to turn on the fan that refers to the vehicle as a whole because there is no natural way to specify its location. You can ensure that Alexa will resolve this utterance to the fan in the "all" zone by assigning "zone.all" as the default zone.

Additionally, the default zone is useful for cases in which you have zoned endpoints with overlapping names, but one of the endpoints is a clear "default" to the user. For example, consider a vehicle with zones "zone.all" (assigned as default), "zone.driver", and "zone.passenger". The vehicle has a "driver window" in "zone.driver" and a "passenger window" in "zone.passenger", but Alexa cannot resolve which endpoint is the intended target of the user utterance "Alexa, open the window." However, the user probably means "Alexa, open the *driver* window". You can ensure that Alexa considers the "driver window" as the "default" window by assigning it to "zone.all" as well.

### Assets

The definitions of endpoints, capabilities, and zones include "assets." Assets, identified by unique IDs, group a voice-accessible friendly name like "air conditioner" into a named group of synonyms and translations for all supported languages. For example, using the asset with ID "Alexa.Automotive.DeviceName.AirConditioner" in your car control module configuration for an AC endpoint not only enables the user to target the air conditioner with the default phrase "air conditioner", but also with phrases like "air con" and "AC" in English as well as synonyms in other supported locales.

Using assets allows decoupling the many ways of identifying components from the core configuration of the components and enables de-duplication across different components that have overlapping ways to be identified.

The Alexa Auto SDK provides a list of IDs for the "default assets," which form an automotive-specific catalog. The catalog contains asset definitions for supported car control features, including endpoint names, zone names, and capability settings. Each default asset ID is prefixed with "Alexa.Automotive." You can use these asset IDs in your `Car Control` module configuration without the corresponding definitions of friendly names, synonyms, and translations, because the definitions are specified in the Alexa cloud.

## Configuring the Car Control Module

`Car Control` module configuration is vehicle-specific and tells the Auto SDK Engine which vehicle features to advertise to Alexa for control by the user. You must configure the Auto SDK Engine with an `EngineConfiguration` object that describes the vehicle. Like all Auto SDK Engine configuration, you can either define the JSON in a file and construct an `EngineConfiguration` from that file, or you can use the provided `CarControlConfiguration` class to programmatically construct the `EngineConfiguration` in the proper format. The following subsections describe the JSON schema. See the [CarControlConfiguration](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1car_control_1_1config_1_1_car_control_configuration.html) class for details on how to build configuration programmatically.

### Configuration Format

The Engine configuration for the `Car Control` module includes definitions of endpoints with their capabilities, zones with their member endpoints, and an optional path to a JSON file defining additional assets.

**Sample JSON Object**

```
{
    "aace.carControl": {
        "endpoints": [
            {
                "endpointId": "{{STRING}}",
                "endpointResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "{{STRING}}"
                            }
                        },
                        ...
                    ]
                },
                "capabilities": [
                    // list of capability definitions for this endpoint
                ]
            }
        ],
        "zones": [
            {
                "zoneID": "{{STRING}}",
                "zoneResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "{{STRING}}"
                            }
                        }
                    ]
                },
                "members": [
                    {
                        "endpointId": "{{STRING}}"
                    }
                ]
            }
        ],
        "defaultZoneID": "{{STRING}}",
        "assets": {
            "customAssetsPath": "{{STRING}}"
        },
    }
}
```

**Object Parameters**

| Property | Type | Required | Description |
|-|-|-|-|
| aace.carControl.<br>endpoints | list | Yes | The list of connected endpoints for which the device implements capabilities. Each endpoint describes one controllable vehicle component. |
| aace.carControl.<br>endpoints[i].<br>endpointId | string | Yes | The identifier for the endpoint, unique amongst all endpoints in the vehicle. The same `endpointId` is used to identify the endpoint targeted in an AASB message sent by the Engine.<br><br>**Note:** Do not use this format for the `endpointId`: <br>\<clientId\>::\<productId\>::\<serialNumber\>::[-\<extEndpoint\>]<br><br>The Engine internally prepends the 3-part device prefix to your specified `endpointId` before sending the configuration to Alexa. Configuring the full ID directly results in duplication and excess characters. |
| aace.carControl.<br>endpoints[i].<br>endpointResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this endpoint.<br><br>**Note:** Only `“asset”` type labels are supported. |
| aace.carControl.<br>endpoints[i].<br>endpointResources.<br>friendlyNames[j].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to the endpoint. <br>The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-about-assets) for more details.|
| aace.carControl.<br>endpoints[i].<br>capabilities | list | Yes | A list of capability definitions, representing capabilities implemented by the device on behalf of the endpoint, that define how the endpoint can be controlled by the user. <br><br>Each object in this list must be a valid definition for one of the capabilities supported by the `Car Control` module:<br>[Alexa.PowerController](#power-controller-capability-configuration), [Alexa.ToggleController](#toggle-controller-capability-configuration), [Alexa.ModeController](#mode-controller-capability-configuration), and [Alexa.RangeController](#range-controller-capability-configuration) |
| aace.carControl.<br>zones | list | No, but recommended | A list of zone definitions for the named regions in the vehicle. |
| aace.carControl.<br>zones[i].<br>zoneId | string | Yes | The identifier for the zone, unique amongst all zones in the vehicle. |
| aace.carControl.<br>zones[i].<br>zoneResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible ways to refer to this zone.<br><br>**Note:** Only `“asset”` type labels are supported. |
| aace.carControl.<br>zones[i].<br>zoneResources.<br>friendlyNames[j].<br>assetId | string | Yes | The ID of an asset definition that includes the list of strings used to refer to the zone in all supported locales.<br>The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-about-assets) for more details.  |
| aace.carControl.<br>zones[i].<br>members | list | Yes | A list of endpoints that belong to this zone. |
| aace.carControl.<br>zones[i].<br>members[j].<br>endpointId | string | Yes | The `endpointId` for an endpoint that belongs to this zone. |
| aace.carControl.<br>defaultZoneId | string | No, but recommended | The `zoneId` of the default zone. Endpoints in this zone take precedence when a user utterance does not specify a zone. <br> It is recommended to use a zone that describes the whole vehicle as the default rather than a zone describing a specific region. |
| aace.carControl.<br>assets.customAssetsPath | string<br>(file path) | No | Specifies the path to a JSON file defining additional assets. |


### Power Controller Capability Configuration

<details markdown="1"><summary>Click to expand or collapse description</summary>

See ["Alexa.PowerController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-powercontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```
{
    "type": "AlexaInterface",
    "interface": "Alexa.PowerController",
    "version": "3",
    "properties": {
        "supported": [
            {
                "name": "powerState"
            }
        ],
        "proactivelyReported": false,
        "retrievable": false
    }
}
```

**Object Parameters**

| Property | Type | Required | Description |
|-|-|-|-|
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "powerState") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "powerState") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |

</details>


### Toggle Controller Capability Configuration

<details markdown="1"><summary>Click to expand or collapse description</summary>

See ["Alexa.ToggleController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-togglecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```
{
    "type": "AlexaInterface",
    "interface": "Alexa.ToggleController",
    "version": "3",
    "instance": "{{STRING}}",
    "capabilityResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "{{STRING}}"
                }
            },
            ...
        ]
    },
    "properties": {
        "proactivelyReported": false,
        "retrievable": false,
        "supported": [
            {
                "name": "toggleState"
            }
        ]
    },
    "semantics": {
        "actionMappings": [
            {
                "@type": "ActionsToDirective",
                "actions": ["{{STRING}}", ...],
                "directive": {
                    "name": "{{STRING}}",
                    "payload": {}
                }
            },
            ...
        ]
    }
}
```

**Object Parameters**

| Property | Type | Required | Description |
|-|-|-|-|
| instance | string | Yes | The identifier of this instance of Alexa.ToggleController on this endpoint. |
| capabilityResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this instance of this capability on this endpoint. <br><br>**Note:** Only `“asset”` type labels are supported. |
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-about-assets) for more details.  |
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "toggleState") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "toggleState") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |
| semantics | object | No | Semantic annotations that enable mapping user utterances with directives targeting this capability instance.<br><br>**Note:** `semantics.stateMappings` is not supported. |
| semantics.<br>actionMappings[i].<br>actions[j] | string | Yes, if `semantics` is present | The identifiers of the utterances that should trigger the specified directive.<br><br>**Accepted values:** <ul> <li>`"Alexa.Actions.Open"`: "open {endpoint}"</li> <li>`"Alexa.Actions.Close"`: "close {endpoint}"</li> <li>`"Alexa.Actions.Raise"`: "raise {endpoint}"</li> <li>`"Alexa.Actions.Lower"`: "lower {endpoint}"</li> </ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:** <ul> <li>`"TurnOn"`: The specified `actions` will trigger the "TurnOn" directive. The Engine will publish the `SetToggleControllerValue` message, with the `turnOn` attribute set to `true`.</li> <li>`"TurnOff"`: The specified `actions` will trigger the "TurnOff" directive. The Engine will publish the `SetToggleControllerValue` message, with the `turnOn` attribute set to `false`.</li> </ul> |

</details>

### Mode Controller Capability Configuration

<details markdown="1"><summary>Click to expand or collapse description</summary>

See ["Alexa.ModeController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-modecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```
{
    "type": "AlexaInterface",
    "interface": "Alexa.ModeController",
    "version": "3",
    "instance": "{{STRING}}",
    "capabilityResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "{{STRING}}"
                }
            },
            ...
        ]
    },
    "properties": {
        "supported": [
            {
                "name": "mode"
            }
        ],
        "proactivelyReported": false,
        "retrievable": false
    },
    "configuration": {
        "ordered": {{BOOLEAN}},
        "supportedModes": [
            {
                "value": "{{STRING}}",
                "modeResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "{{STRING}}"
                            }
                        }
                    ]
                }
            },
            ...
        ]
    },
    "semantics": {
        "actionMappings": [
            {
                "@type": "ActionsToDirective",
                "actions": ["{{STRING}}", ...],
                "directive": {
                    "name": "{{STRING}}",
                    "payload": {{OBJECT}}
                }
            },
            ...
        ]
    }
}
```

**Object Parameters**

| Property | Type | Required | Description |
|-|-|-|-|
| instance | string | Yes | The identifier of this instance of Alexa.ModeController on this endpoint. |
| capabilityResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this instance of this capability on this endpoint. <br><br>**Note:** Only `“asset”` type labels are supported. |
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-about-assets) for more details.  |
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "mode") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "mode") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |
| configuration.<br>ordered | boolean | Yes | Whether the modes of this capability instance are ordered, enabling iteration through them using the "AdjustMode" directive. |
| configuration.<br>supportedModes | list | Yes | A list of objects describing the available modes of this capability instance. If `ordered` is true, the order of the objects in this list implies the ordering of the modes. |
| configuration.<br>supportedModes[i].<br>value | string | Yes | The identifier of this mode on this capability instance. |
| configuration.<br>supportedModes[i].<br>modeResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this mode.<br><br>**Note:** Only `“asset”` type labels are supported. |
| semantics | object | No | Semantic annotations that enable mapping user utterances with directives targeting this capability instance.<br><br>**Note:** `semantics.stateMappings` is not supported. |
| semantics.<br>actionMappings[i].<br>actions[j] | string | Yes, if `semantics` is present | The identifiers of the utterances that should trigger the specified directive.<br><br>**Accepted values:** <ul><br><li>`"Alexa.Actions.Open"`: "open {endpoint}"</li><li>`"Alexa.Actions.Close"`: "close {endpoint}"</li><li>`"Alexa.Actions.Raise"`: "raise {endpoint}"</li><li>`"Alexa.Actions.Lower"`: "lower {endpoint}"</li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:**<br><ul><br><li>`"SetMode"`: The specified actions will trigger the "SetMode" directive with the specified `payload`. The Engine will publish the `SetModeControllerValue` message.</li><li>`"AdjustMode"`: The specified actions will trigger the "AdjustMode" directive with the specified `payload`. The Engine will publish the `AdjustModeControllerValue` message. "AdjustMode" is accepted only if this capability instance is `ordered`. </li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>payload | object | Yes, if `semantics` is present | If `name` is “SetMode”, this is the “SetMode” directive payload object that contains the “mode” property and the corresponding value from `configuration.supportedModes[].value`.<br><br>If `name` is “AdjustMode”, this is the “AdjustMode” directive payload object that contains the “modeDelta” field and the corresponding number of modes to advance. |

</details>

### Range Controller Capability Configuration

<details markdown="1"><summary>Click to expand or collapse description</summary>

See ["Alexa.RangeController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-rangecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```
{
    "type": "AlexaInterface",
    "interface": "Alexa.RangeController",
    "version": "3",
    "instance": "{{STRING}}",
    "capabilityResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "{{STRING}}"
                }
            },
            ...
        ]
    },
    "properties": {
        "supported": [
            {
                "name": "rangeValue"
            }
        ],
        "proactivelyReported": false,
        "retrievable": false
    },
    "configuration": {
        "supportedRange": {
            "minimumValue": {{LONG}},
            "maximumValue": {{LONG}},
            "precision": {{LONG}}
        },
        "unitOfMeasure": "{{STRING}}",
        "presets": [
            {
                "rangeValue": {{LONG}},
                "presetResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "{{STRING}}"
                            }
                        },
                        ...
                    ]
                }
            },
            ...
        ],
        "semantics": {
            "actionMappings": [
                {
                    "@type": "ActionsToDirective",
                    "actions": ["{{STRING}}", ...],
                    "directive": {
                        "name": "{{STRING}}",
                        "payload": {{OBJECT}}
                    }
                },
                ...
            ]
        }
    }
}
```

**Object Parameters**

| Property | Type | Required | Description |
|-|-|-|-|
| instance | string | Yes | The identifier of this instance of Alexa.RangeController on this endpoint. |
| capabilityResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this instance of this capability on this endpoint. <br><br>**Note:** Only `“asset”` type labels are supported. |
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-about-assets) for more details.  |
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "rangeValue") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "rangeValue") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |
| configuration.<br>supportedRange.<br>minimumValue | long | Yes | The minimum value of the range this capability instance supports. |
| configuration.<br>supportedRange.<br>maximumValue | long | Yes | The maximum value of the range this capability instance supports. |
| configuration.<br>supportedRange.<br>precision | long | Yes | The amount by which the set value changes when iterating through the range. For example, if a user asks Alexa to increase the value but doesn't specify by how much, this value will be used. |
| configuration.<br>unitOfMeasure | string | No | The unit of measure for the range. |
| configuration.<br>presets | list | Yes | A list of objects describing values that can be invoked by name. For example, a `rangeValue` of 10 might be configured as the "high" preset. |
| configuration.<br>presets[i].<br>rangeValue | long | Yes | The value within the `supportedRange` that has an associated named preset. |
| configuration.<br>presets[i].<br>presetResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this preset.<br><br>**Note:** Only `“asset”` type labels are supported. |
| semantics | object | No | Semantic annotations that enable mapping user utterances with directives targeting this capability instance.<br><br>**Note:** `semantics.stateMappings` is not supported. |
| semantics.<br>actionMappings[i].<br>actions[j] | string | Yes, if `semantics` is present | The identifiers of the utterances that should trigger the specified directive.<br><br>**Accepted values:** <ul><li>`"Alexa.Actions.Open"`: "open {endpoint}"</li><li>`"Alexa.Actions.Close"`: "close {endpoint}"</li><li>`"Alexa.Actions.Raise"`: "raise {endpoint}"</li><li>`"Alexa.Actions.Lower"`: "lower {endpoint}"</li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:**<br><ul><li>`"SetRangeValue"`: The specified actions will trigger the "SetRangeValue" directive with the specified `payload`. The Engine will publish the `SetRangeControllerValue` message.</li><li>`"AdjustRangeValue"`: The specified actions will trigger the "AdjustRangeValue" directive with the specified `payload`. The Engine will publish the `AdjustRangeControllerValue` message.</li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>payload | object | Yes, if `semantics` is present | If `name` is “SetRangeValue”, this is the “SetRangeValue” directive payload object that contains the "rangeValue" property and the corresponding value between `configuration.supportedRange.minumumValue` and `configuration.supportedRange.maximumValue`.<br><br>If `name` is “AdjustRangeValue”, this is the “AdjustRangeValue” directive payload object that contains the “rangeValueDelta” field. |

</details>

### Additional Notes about Assets

The `Car Control` module provides the full list of asset IDs available in the default automotive catalog of assets in the [CarControlAssets.h](https://github.com/alexa/alexa-auto-sdk/blob/master/modules/car-control/platform/include/AACE/CarControl/CarControlAssets.h) header file, which defines string constants for these asset IDs to be used when constructing configuration programmatically. The values of the constants are the same asset IDs that you should use if you construct your configuration in a JSON file.

This module also provides a [reference JSON file](https://github.com/alexa/alexa-auto-sdk/blob/master/modules/car-control/assets/assets-1P.json) with sample definitions of the assets in the automotive catalog. Your implementation does not need to duplicate these asset definitions or specify the path to this file because the definitions also exist in the Alexa cloud. This copy of the file is a reference for you to see the synonyms and translations for the available assets.

>**Note:** Because the actual asset definitions are defined in the Alexa cloud, this reference file may be outdated or missing translations.

The automotive catalog of assets defines assets for every feature officially supported by car control. The majority of your configuration will use these asset IDs. Amazon recommends that you not create new, custom assets for features that already exist in the default catalog. However, if your vehicle has a feature that cannot be described using the default assets (e.g., an endpoint with a proprietary name), create an additional JSON file defining a complementary set of assets to use alongside the default catalog. The format of this file must follow the same schema as the [reference default assets JSON](https://github.com/alexa/alexa-auto-sdk/blob/master/modules/car-control/assets/assets-1P.json), and the definitions must include entries for each of the locales supported in the default catalog. Prefix every `assetId` in this file with `"My."`, and specify the path to the file in the optional `aace.carControl.assets.customAssetsPath` field of configuration.

>**Note for LVC:** When using Local Voice Control and car control custom assets, there are two distinct configurations — the Auto SDK Engine and the LVC app — that require the path to the custom assets definition file. See the below subsections for details for this configuration on Linux or Android.

##### (Local Voice Control) Custom Assets for Linux Integration
The default LVC app configuration for Linux expects any custom assets to be defined in a file called `assets.json` located at `/opt/LVC/data/led-service/assets/assets.json`. Use this path when you configure the `aace.carControl.assets.customAssetsPath` field in the `Car Control` module configuration.

##### (Local Voice Control) Custom Assets for Android Integration
Local Voice Control Android integrations using the LVC APK implement the `ILVCClient` interface to configure Local Voice Control in the LVC APK (See the LVC extension documentation for more details). The "CarControl.CustomAssetsFilePath" field of the `ILVCClient.getConfiguration()` configuration schema specifies a path to the custom assets definition file, which must be accessible to the processes running the LVC APK services.

When you integrate with AACS, you do not need to provide the `CarControl.CustomAssetsFilePath` field in any AACS configuration message; instead, your application should directly share permissions to the custom assets definition file using the AACS file sharing protocol. AACS will create a local copy of the file and use the path to its local copy to configure the LVC APK.

### Sample Configuration

The `Car Control` module provides a [sample JSON file](https://github.com/alexa/alexa-auto-sdk/blob/master/modules/car-control/assets/CarControlConfig.json) to configure the Auto SDK Engine with a vehicle fully equipped for every use case officially supported for car control. This file models each supported endpoint with a configuration of capabilities and zones that ensures all supported utterances for that endpoint work as expected. It is recommended that you construct the configuration for your application by selecting the parts of this sample that describe features supported by your vehicle. Make adjustments to the endpoints, such as modifying modes and range settings, as needed.

### Configuration for Linux Integration
If your implementation constructs the `Car Control` module `EngineConfiguration` programmatically rather than with a JSON file, see the following example usage of the [aace::carControl::config::CarControlConfiguration](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1car_control_1_1config_1_1_car_control_configuration.html) builder class that produces the same fully-equipped vehicle as the sample file:

<details markdown="1"><summary>Click to expand or collapse CarControlConfiguration C++ sample code</summary>

```c++
#include <AACE/CarControl/CarControlAssets.h>

using namespace aace::carControl::config;
using namespace aace::carControl::assets;

// Auto SDK Engine configuration
std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configuration;

auto config = aace::carControl::config::CarControlConfiguration::create();

config
    ->createZone("zone.default")
        .addAssetId(alexa::location::ALL)
        .addMembers({
            "climatecontrol",
            "default.ac",
            "default.fan",
            "default.vent",
            "default.heater",
            "default.light",
            "default.reading.light",
            "driver.cupholder",
            "driver.armrest",
            "driver.seat",
            "driver.window",
            "front.windshield",
            "front.foglight",
            "front.wipers"
        })
    .createZone("zone.driver")
        .addAssetId(alexa::location::DRIVER)
        .addAssetId(alexa::location::FRONT_LEFT)
        .addMembers({
            "driver.fan",
            "driver.vent",
            "driver.heater",
            "driver.light",
            "driver.reading.light",
            "driver.cupholder",
            "driver.armrest",
            "driver.seat",
            "driver.window"
        })
    .createZone("zone.passenger")
        .addAssetId(alexa::location::PASSENGER)
        .addAssetId(alexa::location::FRONT_RIGHT)
        .addMembers({
            "passenger.fan",
            "passenger.vent",
            "passenger.heater",
            "passenger.light",
            "passenger.reading.light",
            "passenger.cupholder",
            "passenger.armrest",
            "passenger.seat",
            "passenger.window" 
        })
    .createZone("zone.front")
        .addAssetId(alexa::location::FRONT)
        .addMembers({
            "front.ac",
            "front.fan",
            "front.vent",
            "front.light",
            "front.reading.light",
            "front.cupholder",
            "front.armrest",
            "front.seat",
            "front.windshield",
            "front.foglight",
            "front.wipers",
            "front.window"
        })
    .createZone("zone.rear")
        .addAssetId(alexa::location::REAR)
        .addMembers({
            "rear.ac",
            "rear.fan",
            "rear.vent",
            "rear.light",
            "rear.reading.light",
            "rear.cupholder",
            "rear.armrest",
            "rear.seat",
            "rear.windshield",
            "rear.foglight",
            "rear.wipers",
            "rear.window"
        })
    .createZone("zone.left")
        .addAssetId(alexa::location::LEFT)
        .addMembers({
            "left.heater",
            "driver.seat",
            "driver.vent"
        })
    .createZone("zone.right")
        .addAssetId(alexa::location::RIGHT)
        .addMembers({
            "right.heater",
            "passenger.seat",
            "passenger.vent"
        })
    .createZone("zone.rear.driver")
        .addAssetId(alexa::location::REAR_DRIVER)
        .addAssetId(alexa::location::REAR_LEFT)
        .addMembers({
            "rear.driver.fan",
            "rear.driver.vent",
            "rear.driver.light",
            "rear.driver.reading.light",
            "rear.driver.cupholder",
            "rear.driver.armrest",
            "rear.driver.seat",
            "rear.driver.window"  
        })
    .createZone("zone.rear.passenger")
        .addAssetId(alexa::location::REAR_PASSENGER)
        .addAssetId(alexa::location::REAR_RIGHT)
        .addMembers({
            "rear.passenger.fan",
            "rear.passenger.vent",
            "rear.passenger.light",
            "rear.passenger.reading.light",
            "rear.passenger.cupholder",
            "rear.passenger.armrest",
            "rear.passenger.seat",
            "rear.passenger.window"
        })
    .createZone("zone.secondRow")
        .addAssetId(alexa::location::SECOND_ROW)
        .addMembers({
            "secondRow.fan",
            "secondRow.vent",
            "secondRow.light",
            "secondRow.reading.light",
            "secondRow.cupholder",
            "secondRow.armrest",
            "secondRow.seat",
            "secondRow.window",
            "secondRow.heater"
        })
    .createZone("zone.thirdRow")
        .addAssetId(alexa::location::THIRD_ROW)
        .addMembers({
            "thirdRow.fan",
            "thirdRow.vent",
            "thirdRow.light",
            "thirdRow.reading.light",
            "thirdRow.cupholder",
            "thirdRow.armrest",
            "thirdRow.seat",
            "thirdRow.window",
            "thirdRow.heater"
        })
    .setDefaultZone("zone.default")

    // "Car" 
    .createEndpoint("car")
        .addAssetId(alexa::device::CAR)
        .addToggleController("recirculate", false)
            .addAssetId(alexa::setting::AIR_RECIRCULATION)
        .addModeController("recirculatemode", false, false)
            .addAssetId(alexa::setting::AIR_RECIRCULATION)
            .addValue("INSIDE")
                .addAssetId(alexa::value::INSIDE_AIR)
            .addValue("OUTSIDE")
                .addAssetId(alexa::value::OUTSIDE_AIR)
            .addValue("AUTO")
                .addAssetId(alexa::setting::AUTO)
        .addToggleController("climate.sync", false)
            .addAssetId(alexa::setting::CLIMATE_SYNC)
        .addModeController("driveMode", false, false)
            .addAssetId(alexa::setting::DRIVE_MODE)
            .addValue("ECO")
                .addAssetId(alexa::setting::ECONOMY)
            .addValue("COMFORT")
                .addAssetId(alexa::value::COMFORT)
            .addValue("SPORT")
                .addAssetId(alexa::value::SPORT)
            .addValue("SPORTPLUS")
                .addAssetId(alexa::value::SPORT_PLUS)
        .addToggleController("towingMode", false)
            .addAssetId(alexa::setting::TOWING_MODE)
        .addToggleController("hillAssist", false)
            .addAssetId(alexa::setting::HILL_ASSIST)
        .addToggleController("windowLock", false)
            .addAssetId(alexa::setting::WINDOW_LOCK)
        .addToggleController("autoBrakeHold", false)
            .addAssetId(alexa::setting::AUTO_BRAKE_HOLD)
    
    // Ambient Light
    .createEndpoint("ambient.light")
        .addAssetId(alexa::device::AMBIENT_LIGHT)
        .addPowerController(false)
        .addModeController("color", false, false)
            .addAssetId(alexa::setting::COLOR)
            .addAssetId(alexa::setting::MODE)
            .addValue("RED")
                .addAssetId(alexa::color::RED)
            .addValue("BLUE")
                .addAssetId(alexa::color::BLUE)
            .addValue("GREEN")
                .addAssetId(alexa::color::GREEN)
            .addValue("WHITE")
                .addAssetId(alexa::color::WHITE)
            .addValue("ORANGE")
                .addAssetId(alexa::color::ORANGE)
            .addValue("YELLOW")
                .addAssetId(alexa::color::YELLOW)
            .addValue("INDIGO")
                .addAssetId(alexa::color::INDIGO)
            .addValue("VIOLET")
                .addAssetId(alexa::color::VIOLET)

    // Air Conditioner
    .createEndpoint("default.ac")
        .addAssetId(alexa::device::AIR_CONDITIONER)
        .addPowerController(false)
        .addModeController("mode", false, false)
            .addAssetId(alexa::setting::MODE)
            .addValue("ECONOMY")
                .addAssetId(alexa::setting::ECONOMY)
            .addValue("AUTOMATIC")
                .addAssetId(alexa::setting::AUTO)
            .addValue("MANUAL")
                .addAssetId(alexa::setting::MANUAL)
        .addModeController("intensity", false, true)
            .addAssetId(alexa::setting::INTENSITY)
            .addValue("LOW")
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addValue("MEDIUM")
                .addAssetId(alexa::value::MEDIUM)
            .addValue("HIGH")
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Front Air Conditioner
    .createEndpoint("front.ac")
        .addAssetId(alexa::device::AIR_CONDITIONER)
        .addPowerController(false)
        .addModeController("mode", false, false)
            .addAssetId(alexa::setting::MODE)
            .addValue("ECONOMY")
                .addAssetId(alexa::setting::ECONOMY)
            .addValue("AUTOMATIC")
                .addAssetId(alexa::setting::AUTO)
            .addValue("MANUAL")
                .addAssetId(alexa::setting::MANUAL)
        .addModeController("intensity", false, true)
            .addAssetId(alexa::setting::INTENSITY)
            .addValue("LOW")
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addValue("MEDIUM")
                .addAssetId(alexa::value::MEDIUM)
            .addValue("HIGH")
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Rear Air Conditioner
    .createEndpoint("rear.ac")
        .addAssetId(alexa::device::AIR_CONDITIONER)
        .addPowerController(false)
        .addModeController("mode", false, false)
            .addAssetId(alexa::setting::MODE)
            .addValue("ECONOMY")
                .addAssetId(alexa::setting::ECONOMY)
            .addValue("AUTOMATIC")
                .addAssetId(alexa::setting::AUTO)
            .addValue("MANUAL")
                .addAssetId(alexa::setting::MANUAL)
        .addModeController("intensity", false, true)
            .addAssetId(alexa::setting::INTENSITY)
            .addValue("LOW")
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addValue("MEDIUM")
                .addAssetId(alexa::value::MEDIUM)
            .addValue("HIGH")
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Fan
    .createEndpoint("default.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Driver Fan
    .createEndpoint("driver.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Passenger Fan
    .createEndpoint("passenger.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Front Fan
    .createEndpoint("front.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Rear Fan
    .createEndpoint("rear.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Rear Driver Fan
    .createEndpoint("rear.driver.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Rear Passenger Fan
    .createEndpoint("rear.passenger.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Second Row Fan
    .createEndpoint("secondRow.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Third Row Fan
    .createEndpoint("thirdRow.fan")
        .addAssetId(alexa::device::FAN)
        .addPowerController(false)
        .addRangeController("speed", false, 1, 10, 1)
            .addAssetId(alexa::setting::FAN_SPEED)
            .addAssetId(alexa::setting::SPEED)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Vent
    .createEndpoint("default.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)
    
    // Driver Vent
    .createEndpoint("driver.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Passenger Vent
    .createEndpoint("passenger.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Front Vent
    .createEndpoint("front.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Rear Vent
    .createEndpoint("rear.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Rear Driver Vent
    .createEndpoint("rear.driver.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)
        
    // Rear Passenger Vent
    .createEndpoint("rear.passenger.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Second Row Vent
    .createEndpoint("secondRow.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)

    // Third Row Vent
    .createEndpoint("thirdRow.vent")
        .addAssetId(alexa::device::VENT)
        .addPowerController(false)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("BODY")
                .addAssetId(alexa::setting::BODY_VENTS)
            .addValue("FLOOR")
                .addAssetId(alexa::setting::FLOOR_VENTS)
            .addValue("WINDSHIELD")
                .addAssetId(alexa::setting::WINDSHIELD_VENTS)
            .addValue("MIX")
                .addAssetId(alexa::setting::MIX_VENTS)
    
    // Climate Control
    .createEndpoint("climatecontrol")
        .addAssetId(alexa::device::CLIMATE_CONTROL)
        .addAssetId(alexa::setting::AUTO)
        .addPowerController(false)

    // Heater
    .createEndpoint("default.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Driver Heater
    .createEndpoint("driver.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
    
    // Passenger Heater
    .createEndpoint("passenger.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Left Heater
    .createEndpoint("left.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Right Heater
    .createEndpoint("right.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Second Row Heater
    .createEndpoint("secondRow.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Third Row Heater
    .createEndpoint("thirdRow.heater")
        .addAssetId(alexa::device::HEATER)
        .addAssetId(alexa::device::COOLER)
        .addPowerController(false)
        .addRangeController("temperature", false, 60, 90, 1, alexa::unit::FAHRENHEIT)
            .addAssetId(alexa::setting::TEMPERATURE)
            .addAssetId(alexa::setting::HEAT)
            .addPreset(60)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(75)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(90)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Light
    .createEndpoint("default.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Driver Light
    .createEndpoint("driver.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Passenger Light
    .createEndpoint("passenger.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Front Light
    .createEndpoint("front.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Rear Light
    .createEndpoint("rear.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Rear Driver Light
    .createEndpoint("rear.driver.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Rear Passenger Light
    .createEndpoint("rear.passenger.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Second Row Light
    .createEndpoint("secondRow.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Third Row Light
    .createEndpoint("thirdRow.light")
        .addAssetId(alexa::device::LIGHT)
        .addAssetId(alexa::device::DOME_LIGHT)
        .addAssetId(alexa::device::CABIN_LIGHT)
        .addPowerController(false)

    // Reading Light
    .createEndpoint("default.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Driver Reading Light
    .createEndpoint("driver.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Passenger Reading Light
    .createEndpoint("passenger.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Front Reading Light
    .createEndpoint("front.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Rear Reading Light
    .createEndpoint("rear.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Rear Driver Reading Light
    .createEndpoint("rear.driver.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Rear Passenger Reading Light
    .createEndpoint("rear.passenger.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Second Row Reading Light
    .createEndpoint("secondRow.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Third Row Reading Light
    .createEndpoint("thirdRow.reading.light")
        .addAssetId(alexa::device::READING_LIGHT)
        .addPowerController(false)

    // Driver Cupholder
    .createEndpoint("driver.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Passenger Cupholder
    .createEndpoint("passenger.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Front Cupholder
    .createEndpoint("front.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Rear Cupholder
    .createEndpoint("rear.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Rear Driver Cupholder
    .createEndpoint("rear.driver.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Rear Passenger Cupholder
    .createEndpoint("rear.passenger.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Second Row Cupholder
    .createEndpoint("secondRow.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Third Row Cupholder
    .createEndpoint("thirdRow.cupholder")
        .addAssetId(alexa::device::CUP_HOLDER)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
        .addToggleController("cooler", false)
            .addAssetId(alexa::device::COOLER)
            .addAssetId(alexa::setting::COOLING)

    // Driver Armrest
    .createEndpoint("driver.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Passenger Armrest
    .createEndpoint("passenger.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Front Armrest
    .createEndpoint("front.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Rear Armrest
    .createEndpoint("rear.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Rear Driver Armrest
    .createEndpoint("rear.driver.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Rear Passenger Armrest
    .createEndpoint("rear.passenger.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Second Row Armrest
    .createEndpoint("secondRow.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Third Row Armrest
    .createEndpoint("thirdRow.armrest")
        .addAssetId(alexa::device::ARMREST)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Driver Seat
    .createEndpoint("driver.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Passenger Seat
    .createEndpoint("passenger.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Front Seat
    .createEndpoint("front.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)
    
    // Rear Seat
    .createEndpoint("rear.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Rear Driver Seat
    .createEndpoint("rear.driver.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Rear Passenger Seat
    .createEndpoint("rear.passenger.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Second Row Seat
    .createEndpoint("secondRow.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Third Row Seat
    .createEndpoint("thirdRow.seat")
        .addAssetId(alexa::device::SEAT)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
        .addRangeController("heaterintensity", false, 1, 3, 1)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)
            .addAssetId(alexa::device::SEAT_HEATER)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(2)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(3)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addToggleController("vent", false)
            .addAssetId(alexa::device::VENT)
        .addModeController("position", false, true)
            .addAssetId(alexa::setting::STORED_POSITION)
            .addValue("ONE")
                .addAssetId(alexa::value::POSITION_ONE)
            .addValue("TWO")
                .addAssetId(alexa::value::POSITION_TWO)
            .addValue("THREE")
                .addAssetId(alexa::value::POSITION_THREE)

    // Front Window
    .createEndpoint("front.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Rear Window
    .createEndpoint("rear.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Driver Window
    .createEndpoint("driver.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Passenger Window
    .createEndpoint("passenger.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Rear Driver Window
    .createEndpoint("rear.driver.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Rear Passenger Window
    .createEndpoint("rear.passenger.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Second Row Window
    .createEndpoint("secondRow.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Third Row Window
    .createEndpoint("thirdRow.window")
        .addAssetId(alexa::device::WINDOW)
        .addRangeController("height", false, 0, 10, 1)
            .addAssetId(alexa::setting::HEIGHT)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)

    // Front Windshield
    .createEndpoint("front.windshield")
        .addAssetId(alexa::device::WINDOW)
        .addAssetId(alexa::device::WINDSHIELD)
        .addToggleController("defroster", false)
            .addAssetId(alexa::setting::DEFROST)
            .addAssetId(alexa::setting::WINDSHIELD_VENTS)

    // Rear Windshield
    .createEndpoint("rear.windshield")
        .addAssetId(alexa::device::WINDOW)
        .addAssetId(alexa::device::WINDSHIELD)
        .addToggleController("defroster", false)
            .addAssetId(alexa::setting::DEFROST)
            .addAssetId(alexa::setting::WINDSHIELD_VENTS)

    // Front Foglight
    .createEndpoint("front.foglight")
        .addAssetId(alexa::device::FOG_LIGHT)
        .addPowerController(false)

    // Rear Foglight
    .createEndpoint("rear.foglight")
        .addAssetId(alexa::device::FOG_LIGHT)
        .addPowerController(false)

    // Hazard Light
    .createEndpoint("hazardlight")
        .addAssetId(alexa::device::HAZARD_LIGHTS)
        .addAssetId(alexa::device::PARKING_LIGHTS)
        .addPowerController(false)

    // Front Wipers
    .createEndpoint("front.wipers")
        .addAssetId(alexa::device::WINDSHIELD_WIPERS)
        .addPowerController(false)
        .addModeController("speed", false, true)
            .addAssetId(alexa::setting::SPEED)
            .addValue("LOW")
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addValue("MEDIUM")
                .addAssetId(alexa::value::MEDIUM)
            .addValue("HIGH")
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Rear Wipers
    .createEndpoint("rear.wipers")
        .addAssetId(alexa::device::WINDSHIELD_WIPERS)
        .addPowerController(false)
        .addModeController("speed", false, true)
            .addAssetId(alexa::setting::SPEED)
            .addValue("LOW")
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addValue("MEDIUM")
                .addAssetId(alexa::value::MEDIUM)
            .addValue("HIGH")
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // Sunroof
    .createEndpoint("sunroof")
        .addAssetId(alexa::device::SUNROOF)
        .addAssetId(alexa::device::MOONROOF)
        .addRangeController("sunroof.position", false, 0, 10, 1)
            .addAssetId(alexa::setting::POSITION)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)

    // Sunshade
    .createEndpoint("sunshade")
        .addAssetId(alexa::device::SUNSHADE)
        .addRangeController("position", false, 0, 10, 1)
            .addAssetId(alexa::setting::POSITION)
            .addPreset(10)
                .addAssetId(alexa::value::FULL)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
                .addAssetId(alexa::value::HALF)
            .addActionSetRange({action::OPEN, action::LOWER}, 0)
            .addActionSetRange({action::CLOSE, action::RAISE}, 10)

    // HUD
    .createEndpoint("hud")
        .addAssetId(alexa::device::HUD)
        .addToggleController("power", false)
            .addAssetId(alexa::device::HUD)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})
        .addRangeController("brightness", false, 1, 10, 1)
            .addAssetId(alexa::setting::BRIGHTNESS)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)

    // IVI
    .createEndpoint("ivi")
        .addAssetId(alexa::device::DISPLAY_SCREEN)
        .addAssetId(alexa::device::INFO_SCREEN)
        .addToggleController("power", false)
            .addAssetId(alexa::device::DISPLAY_SCREEN)
            .addAssetId(alexa::device::INFO_SCREEN)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})
        .addRangeController("brightness", false, 1, 10, 1)
            .addAssetId(alexa::setting::BRIGHTNESS)
            .addPreset(1)
                .addAssetId(alexa::value::LOW)
                .addAssetId(alexa::value::MINIMUM)
            .addPreset(5)
                .addAssetId(alexa::value::MEDIUM)
            .addPreset(10)
                .addAssetId(alexa::value::HIGH)
                .addAssetId(alexa::value::MAXIMUM)
        .addModeController("autobrightness", false, false)
            .addAssetId(alexa::setting::BRIGHTNESS)
            .addValue("OPTIMAL")
                .addAssetId(alexa::value::OPTIMAL)
            .addValue("AUTO")
                .addAssetId(alexa::setting::AUTO)

    // Dynamics Coordinator Page
    .createEndpoint("dynamicsCoordinatorPage")
        .addAssetId(alexa::value::DYNAMIC_COORDINATOR_PAGE)
        .addToggleController("dynamicsCoordinator.screen", false)
            .addAssetId(alexa::value::DYNAMIC_COORDINATOR_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Performance Page
    .createEndpoint("performancePage")
        .addAssetId(alexa::value::PERFORMANCE_PAGE)
        .addToggleController("performance.screen", false)
            .addAssetId(alexa::value::PERFORMANCE_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Home Page
    .createEndpoint("homepage")
        .addAssetId(alexa::value::HOME_PAGE)
        .addToggleController("home.screen", false)
            .addAssetId(alexa::value::HOME_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Bluetooth Page
    .createEndpoint("bluetoothPage")
        .addAssetId(alexa::value::BLUETOOTH_PAGE)
        .addToggleController("bluetooth.screen", false)
            .addAssetId(alexa::value::BLUETOOTH_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Radio Page
    .createEndpoint("radioPage")
        .addAssetId(alexa::value::RADIO_PAGE)
        .addToggleController("radio.screen", false)
            .addAssetId(alexa::value::RADIO_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Settings Page
    .createEndpoint("settingsPage")
        .addAssetId(alexa::value::SETTINGS_PAGE)
        .addToggleController("settings.screen", false)
            .addAssetId(alexa::value::SETTINGS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Controls Page
    .createEndpoint("controlsPage")
        .addAssetId(alexa::value::CONTROLS_PAGE)
        .addToggleController("controls.screen", false)
            .addAssetId(alexa::value::CONTROLS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Navigation Page
    .createEndpoint("navigationPage")
        .addAssetId(alexa::value::NAVIGATION_PAGE)
        .addToggleController("navigation.screen", false)
            .addAssetId(alexa::value::NAVIGATION_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // GPS Page
    .createEndpoint("gpsPage")
        .addAssetId(alexa::value::GPS_PAGE)
        .addToggleController("gps.screen", false)
            .addAssetId(alexa::value::GPS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Service Page
    .createEndpoint("servicePage")
        .addAssetId(alexa::value::SERVICE_PAGE)
        .addToggleController("service.screen", false)
            .addAssetId(alexa::value::SERVICE_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Satellite Radio Page
    .createEndpoint("satelliteRadioPage")
        .addAssetId(alexa::value::SATELLITE_RADIO_PAGE)
        .addToggleController("satelliteRadio.screen", false)
            .addAssetId(alexa::value::SATELLITE_RADIO_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Information Page
    .createEndpoint("informationPage")
        .addAssetId(alexa::value::INFORMATION_PAGE)
        .addToggleController("information.screen", false)
            .addAssetId(alexa::value::INFORMATION_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Vehicle Status Page
    .createEndpoint("vehicleStatusPage")
        .addAssetId(alexa::value::VEHICLE_STATUS_PAGE)
        .addToggleController("vehicleStatus.screen", false)
            .addAssetId(alexa::value::VEHICLE_STATUS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Multimedia Page
    .createEndpoint("multimediaPage")
        .addAssetId(alexa::value::MULTIMEDIA_PAGE)
        .addAssetId(alexa::value::MUSIC_PAGE)
        .addToggleController("multimedia.screen", false)
            .addAssetId(alexa::value::MULTIMEDIA_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Telephone Page
    .createEndpoint("telephonePage")
        .addAssetId(alexa::value::TELEPHONE_PAGE)
        .addToggleController("telephone.screen", false)
            .addAssetId(alexa::value::TELEPHONE_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Contacts Page
    .createEndpoint("contactsPage")
        .addAssetId(alexa::value::CONTACTS_PAGE)
        .addToggleController("contacts.screen", false)
            .addAssetId(alexa::value::CONTACTS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Alerts Page
    .createEndpoint("alertsPage")
        .addAssetId(alexa::value::ALERTS_PAGE)
        .addToggleController("alerts.screen", false)
            .addAssetId(alexa::value::ALERTS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})

    // Notifications Page
    .createEndpoint("notificationsPage")
        .addAssetId(alexa::value::NOTIFICATIONS_PAGE)
        .addToggleController("notifications.screen", false)
            .addAssetId(alexa::value::NOTIFICATIONS_PAGE)
            .addActionTurnOff({action::CLOSE})
            .addActionTurnOn({action::OPEN})
    
    // 360 Camera
    .createEndpoint("360Camera")
        .addAssetId(alexa::device::CAMERA_360)
        .addAssetId(alexa::device::AVM_CAMERA)
        .addPowerController(false)
        .addModeController("direction", false, true)
            .addAssetId(alexa::setting::DIRECTION)
            .addValue("FRONT")
                .addAssetId(alexa::location::FRONT)
            .addValue("REAR")
                .addAssetId(alexa::location::REAR)
            .addValue("DRIVER")
                .addAssetId(alexa::location::DRIVER)
            .addValue("PASSENGER")
                .addAssetId(alexa::location::PASSENGER)
            .addValue("AUTO")
                .addAssetId(alexa::setting::AUTO)

    // Steering Wheel
    .createEndpoint("steeringWheel")
        .addAssetId(alexa::device::STEERING_WHEEL)
        .addToggleController("heater", false)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::setting::HEAT)

    // Hood
    .createEndpoint("hood")
        .addAssetId(alexa::device::HOOD)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("OPEN")
                .addAssetId(alexa::value::OPEN)
            .addValue("CLOSED")
                .addAssetId(alexa::value::CLOSED)
            .addActionSetMode({action::CLOSE}, "CLOSED")
            .addActionSetMode({action::OPEN}, "OPEN")

    // Trunk
    .createEndpoint("trunk")
        .addAssetId(alexa::device::TRUNK)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("OPEN")
                .addAssetId(alexa::value::OPEN)
            .addValue("CLOSED")
                .addAssetId(alexa::value::CLOSED)
            .addActionSetMode({action::CLOSE}, "CLOSED")
            .addActionSetMode({action::OPEN}, "OPEN")

    // Charge Door
    .createEndpoint("chargedoor")
        .addAssetId(alexa::device::CHARGE_DOOR)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("OPEN")
                .addAssetId(alexa::value::OPEN)
            .addValue("CLOSED")
                .addAssetId(alexa::value::CLOSED)
            .addActionSetMode({action::CLOSE}, "CLOSED")
            .addActionSetMode({action::OPEN}, "OPEN")

    // Gas Door
    .createEndpoint("gasdoor")
        .addAssetId(alexa::device::GAS_DOOR)
        .addModeController("position", false, false)
            .addAssetId(alexa::setting::POSITION)
            .addValue("OPEN")
                .addAssetId(alexa::value::OPEN)
            .addValue("CLOSED")
                .addAssetId(alexa::value::CLOSED)
            .addActionSetMode({action::CLOSE}, "CLOSED")
            .addActionSetMode({action::OPEN}, "OPEN");

configuration.push_back(config);

```
</details>

### Configuration for Android Integration

To use the `Car Control` module Engine configuration with AACS, use *"aacs.carControl"* instead of *"aace.carControl"* in your AACS configuration file:

```
{
    "aacs.carControl": {
        "endpoints": [
            // list of endpoint definitions
        ],
        "zones": [
            // list of zone definitions
        ],
        "defaultZoneID": "{{STRING}}",
        "assets": {
            "customAssetsPath": "{{STRING}}"
        },
    }
}

```

<details markdown="1"><summary>Click to expand or collapse details for Android integration without AACS</summary>
<br>
AACS is the recommended way to integrate Auto SDK for Android. However, if your integration does not use AACS, you can use the Java factory class [com.amazon.aace.carControl.CarControlConfiguration](https://gitlab.automotive.alexa.a2z.com/alexa-auto-hut/aac-sdk/-/blob/3.3/platforms/android/modules/car-control/src/main/java/com/amazon/aace/carControl/CarControlConfiguration.java) to programmatically construct the `EngineConfiguration` in the proper format, as shown in the example below.

###### CarControlConfiguration Java sample code
```java
import com.amazon.aace.carControl.CarControlAssets;
import com.amazon.aace.carControl.CarControlConfiguration;
import com.amazon.aace.core.config.EngineConfiguration;

// Auto SDK Engine configuration
List<EngineConfiguration> configuration = new ArrayList<>();

CarControlConfiguration config = CarControlConfiguration.create();

config.createZone("zone.default")
    .addAssetId(CarControlAssets.Location.ALL)
    .addMembers(new String[] {
        "climatecontrol",
        "default.ac",
        "default.fan",
        "default.vent",
        "default.heater",
        "default.light",
        "default.reading.light",
        "driver.cupholder",
        "driver.armrest",
        "driver.seat",
        "driver.window",
        "front.windshield",
        "front.foglight",
        "front.wipers"
    });
config.createZone("zone.driver")
    .addAssetId(CarControlAssets.Location.DRIVER)
    .addAssetId(CarControlAssets.Location.FRONT_LEFT)
    .addMembers(new String[] {
        "driver.fan",
        "driver.vent",
        "driver.heater",
        "driver.light",
        "driver.reading.light",
        "driver.cupholder",
        "driver.armrest",
        "driver.seat",
        "driver.window"
    });
config.createZone("zone.passenger")
    .addAssetId(CarControlAssets.Location.PASSENGER)
    .addAssetId(CarControlAssets.Location.FRONT_RIGHT)
    .addMembers(new String[] {
        "passenger.fan",
        "passenger.vent",
        "passenger.heater",
        "passenger.light",
        "passenger.reading.light",
        "passenger.cupholder",
        "passenger.armrest",
        "passenger.seat",
        "passenger.window" 
    });
config.createZone("zone.front")
    .addAssetId(CarControlAssets.Location.FRONT)
    .addMembers(new String[] {
        "front.ac",
        "front.fan",
        "front.vent",
        "front.light",
        "front.reading.light",
        "front.cupholder",
        "front.armrest",
        "front.seat",
        "front.windshield",
        "front.foglight",
        "front.wipers",
        "front.window"
    });
config.createZone("zone.rear")
    .addAssetId(CarControlAssets.Location.REAR)
    .addMembers(new String[] {
        "rear.ac",
        "rear.fan",
        "rear.vent",
        "rear.light",
        "rear.reading.light",
        "rear.cupholder",
        "rear.armrest",
        "rear.seat",
        "rear.windshield",
        "rear.foglight",
        "rear.wipers",
        "rear.window"
    });
config.createZone("zone.left")
    .addAssetId(CarControlAssets.Location.LEFT)
    .addMembers(new String[] {
        "left.heater",
        "driver.seat",
        "driver.vent"
    });
config.createZone("zone.right")
    .addAssetId(CarControlAssets.Location.RIGHT)
    .addMembers(new String[] {
        "right.heater",
        "passenger.seat",
        "passenger.vent"
    });
config.createZone("zone.rear.driver")
    .addAssetId(CarControlAssets.Location.REAR_DRIVER)
    .addAssetId(CarControlAssets.Location.REAR_LEFT)
    .addMembers(new String[] {
        "rear.driver.fan",
        "rear.driver.vent",
        "rear.driver.light",
        "rear.driver.reading.light",
        "rear.driver.cupholder",
        "rear.driver.armrest",
        "rear.driver.seat",
        "rear.driver.window"  
    });
config.createZone("zone.rear.passenger")
    .addAssetId(CarControlAssets.Location.REAR_PASSENGER)
    .addAssetId(CarControlAssets.Location.REAR_RIGHT)
    .addMembers(new String[] {
        "rear.passenger.fan",
        "rear.passenger.vent",
        "rear.passenger.light",
        "rear.passenger.reading.light",
        "rear.passenger.cupholder",
        "rear.passenger.armrest",
        "rear.passenger.seat",
        "rear.passenger.window"
    });
config.createZone("zone.secondRow")
    .addAssetId(CarControlAssets.Location.SECOND_ROW)
    .addMembers(new String[] {
        "secondRow.fan",
        "secondRow.vent",
        "secondRow.light",
        "secondRow.reading.light",
        "secondRow.cupholder",
        "secondRow.armrest",
        "secondRow.seat",
        "secondRow.window",
        "secondRow.heater"
    });
config.createZone("zone.thirdRow")
    .addAssetId(CarControlAssets.Location.THIRD_ROW)
    .addMembers(new String[] {
        "thirdRow.fan",
        "thirdRow.vent",
        "thirdRow.light",
        "thirdRow.reading.light",
        "thirdRow.cupholder",
        "thirdRow.armrest",
        "thirdRow.seat",
        "thirdRow.window",
        "thirdRow.heater"
    });
config.setDefaultZone("zone.default");

// "Car" 
config.createEndpoint("car")
    .addAssetId(CarControlAssets.Device.CAR)
    .addToggleController("recirculate", false)
        .addAssetId(CarControlAssets.Setting.AIR_RECIRCULATION)
    .addModeController("recirculatemode", false, false)
        .addAssetId(CarControlAssets.Setting.AIR_RECIRCULATION)
        .addValue("INSIDE")
            .addAssetId(CarControlAssets.Value.INSIDE_AIR)
        .addValue("OUTSIDE")
            .addAssetId(CarControlAssets.Value.OUTSIDE_AIR)
        .addValue("AUTO")
            .addAssetId(CarControlAssets.Setting.AUTO)
    .addToggleController("climate.sync", false)
        .addAssetId(CarControlAssets.Setting.CLIMATE_SYNC)
    .addModeController("driveMode", false, false)
        .addAssetId(CarControlAssets.Setting.DRIVE_MODE)
        .addValue("ECO")
            .addAssetId(CarControlAssets.Setting.ECONOMY)
        .addValue("COMFORT")
            .addAssetId(CarControlAssets.Value.COMFORT)
        .addValue("SPORT")
            .addAssetId(CarControlAssets.Value.SPORT)
        .addValue("SPORTPLUS")
            .addAssetId(CarControlAssets.Value.SPORT_PLUS)
    .addToggleController("towingMode", false)
        .addAssetId(CarControlAssets.Setting.TOWING_MODE)
    .addToggleController("hillAssist", false)
        .addAssetId(CarControlAssets.Setting.HILL_ASSIST)
    .addToggleController("windowLock", false)
        .addAssetId(CarControlAssets.Setting.WINDOW_LOCK)
    .addToggleController("autoBrakeHold", false)
        .addAssetId(CarControlAssets.Setting.AUTO_BRAKE_HOLD);

// Ambient Light
config.createEndpoint("ambient.light")
    .addAssetId(CarControlAssets.Device.AMBIENT_LIGHT)
    .addPowerController(false)
    .addModeController("color", false, false)
        .addAssetId(CarControlAssets.Setting.COLOR)
        .addAssetId(CarControlAssets.Setting.MODE)
        .addValue("RED")
            .addAssetId(CarControlAssets.Color.RED)
        .addValue("BLUE")
            .addAssetId(CarControlAssets.Color.BLUE)
        .addValue("GREEN")
            .addAssetId(CarControlAssets.Color.GREEN)
        .addValue("WHITE")
            .addAssetId(CarControlAssets.Color.WHITE)
        .addValue("ORANGE")
            .addAssetId(CarControlAssets.Color.ORANGE)
        .addValue("YELLOW")
            .addAssetId(CarControlAssets.Color.YELLOW)
        .addValue("INDIGO")
            .addAssetId(CarControlAssets.Color.INDIGO)
        .addValue("VIOLET")
            .addAssetId(CarControlAssets.Color.VIOLET);

// Air Conditioner
config.createEndpoint("default.ac")
    .addAssetId(CarControlAssets.Device.AIR_CONDITIONER)
    .addPowerController(false)
    .addModeController("mode", false, false)
        .addAssetId(CarControlAssets.Setting.MODE)
        .addValue("ECONOMY")
            .addAssetId(CarControlAssets.Setting.ECONOMY)
        .addValue("AUTOMATIC")
            .addAssetId(CarControlAssets.Setting.AUTO)
        .addValue("MANUAL")
            .addAssetId(CarControlAssets.Setting.MANUAL)
    .addModeController("intensity", false, true)
        .addAssetId(CarControlAssets.Setting.INTENSITY)
        .addValue("LOW")
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addValue("MEDIUM")
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addValue("HIGH")
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Front Air Conditioner
config.createEndpoint("front.ac")
    .addAssetId(CarControlAssets.Device.AIR_CONDITIONER)
    .addPowerController(false)
    .addModeController("mode", false, false)
        .addAssetId(CarControlAssets.Setting.MODE)
        .addValue("ECONOMY")
            .addAssetId(CarControlAssets.Setting.ECONOMY)
        .addValue("AUTOMATIC")
            .addAssetId(CarControlAssets.Setting.AUTO)
        .addValue("MANUAL")
            .addAssetId(CarControlAssets.Setting.MANUAL)
    .addModeController("intensity", false, true)
        .addAssetId(CarControlAssets.Setting.INTENSITY)
        .addValue("LOW")
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addValue("MEDIUM")
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addValue("HIGH")
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Rear Air Conditioner
config.createEndpoint("rear.ac")
    .addAssetId(CarControlAssets.Device.AIR_CONDITIONER)
    .addPowerController(false)
    .addModeController("mode", false, false)
        .addAssetId(CarControlAssets.Setting.MODE)
        .addValue("ECONOMY")
            .addAssetId(CarControlAssets.Setting.ECONOMY)
        .addValue("AUTOMATIC")
            .addAssetId(CarControlAssets.Setting.AUTO)
        .addValue("MANUAL")
            .addAssetId(CarControlAssets.Setting.MANUAL)
    .addModeController("intensity", false, true)
        .addAssetId(CarControlAssets.Setting.INTENSITY)
        .addValue("LOW")
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addValue("MEDIUM")
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addValue("HIGH")
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Fan
config.createEndpoint("default.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Driver Fan
config.createEndpoint("driver.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Passenger Fan
config.createEndpoint("passenger.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Front Fan
config.createEndpoint("front.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Rear Fan
config.createEndpoint("rear.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Rear Driver Fan
config.createEndpoint("rear.driver.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Rear Passenger Fan
config.createEndpoint("rear.passenger.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Second Row Fan
config.createEndpoint("secondRow.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Third Row Fan
config.createEndpoint("thirdRow.fan")
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(false)
    .addRangeController("speed", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Vent
config.createEndpoint("default.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Driver Vent
config.createEndpoint("driver.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Passenger Vent
config.createEndpoint("passenger.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Front Vent
config.createEndpoint("front.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Rear Vent
config.createEndpoint("rear.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Rear Driver Vent
config.createEndpoint("rear.driver.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);
    
// Rear Passenger Vent
config.createEndpoint("rear.passenger.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Second Row Vent
config.createEndpoint("secondRow.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Third Row Vent
config.createEndpoint("thirdRow.vent")
    .addAssetId(CarControlAssets.Device.VENT)
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("BODY")
            .addAssetId(CarControlAssets.Setting.BODY_VENTS)
        .addValue("FLOOR")
            .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
        .addValue("WINDSHIELD")
            .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
        .addValue("MIX")
            .addAssetId(CarControlAssets.Setting.MIX_VENTS);

// Climate Control
config.createEndpoint("climatecontrol")
    .addAssetId(CarControlAssets.Device.CLIMATE_CONTROL)
    .addAssetId(CarControlAssets.Setting.AUTO)
    .addPowerController(false);

// Heater
config.createEndpoint("default.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Driver Heater
config.createEndpoint("driver.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Passenger Heater
config.createEndpoint("passenger.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Left Heater
config.createEndpoint("left.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Right Heater
config.createEndpoint("right.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Second Row Heater
config.createEndpoint("secondRow.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Third Row Heater
config.createEndpoint("thirdRow.heater")
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(false)
    .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(75)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(90)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Light
config.createEndpoint("default.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Driver Light
config.createEndpoint("driver.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Passenger Light
config.createEndpoint("passenger.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Front Light
config.createEndpoint("front.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Rear Light
config.createEndpoint("rear.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Rear Driver Light
config.createEndpoint("rear.driver.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Rear Passenger Light
config.createEndpoint("rear.passenger.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Second Row Light
config.createEndpoint("secondRow.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Third Row Light
config.createEndpoint("thirdRow.light")
    .addAssetId(CarControlAssets.Device.LIGHT)
    .addAssetId(CarControlAssets.Device.DOME_LIGHT)
    .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
    .addPowerController(false);

// Reading Light
config.createEndpoint("default.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Driver Reading Light
config.createEndpoint("driver.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Passenger Reading Light
config.createEndpoint("passenger.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Front Reading Light
config.createEndpoint("front.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Rear Reading Light
config.createEndpoint("rear.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Rear Driver Reading Light
config.createEndpoint("rear.driver.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Rear Passenger Reading Light
config.createEndpoint("rear.passenger.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Second Row Reading Light
config.createEndpoint("secondRow.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Third Row Reading Light
config.createEndpoint("thirdRow.reading.light")
    .addAssetId(CarControlAssets.Device.READING_LIGHT)
    .addPowerController(false);

// Driver Cupholder
config.createEndpoint("driver.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Passenger Cupholder
config.createEndpoint("passenger.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Front Cupholder
config.createEndpoint("front.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Rear Cupholder
config.createEndpoint("rear.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Rear Driver Cupholder
config.createEndpoint("rear.driver.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Rear Passenger Cupholder
config.createEndpoint("rear.passenger.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Second Row Cupholder
config.createEndpoint("secondRow.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Third Row Cupholder
config.createEndpoint("thirdRow.cupholder")
    .addAssetId(CarControlAssets.Device.CUP_HOLDER)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
    .addToggleController("cooler", false)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addAssetId(CarControlAssets.Setting.COOLING);

// Driver Armrest
config.createEndpoint("driver.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Passenger Armrest
config.createEndpoint("passenger.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Front Armrest
config.createEndpoint("front.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Rear Armrest
config.createEndpoint("rear.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Rear Driver Armrest
config.createEndpoint("rear.driver.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Rear Passenger Armrest
config.createEndpoint("rear.passenger.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Second Row Armrest
config.createEndpoint("secondRow.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Third Row Armrest
config.createEndpoint("thirdRow.armrest")
    .addAssetId(CarControlAssets.Device.ARMREST)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Driver Seat
config.createEndpoint("driver.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Passenger Seat
config.createEndpoint("passenger.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Front Seat
config.createEndpoint("front.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Rear Seat
config.createEndpoint("rear.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Rear Driver Seat
config.createEndpoint("rear.driver.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Rear Passenger Seat
config.createEndpoint("rear.passenger.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Second Row Seat
config.createEndpoint("secondRow.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Third Row Seat
config.createEndpoint("thirdRow.seat")
    .addAssetId(CarControlAssets.Device.SEAT)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
    .addRangeController("heaterintensity", false, 1, 3, 1, "")
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addAssetId(CarControlAssets.Device.SEAT_HEATER)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(2)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(3)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addToggleController("vent", false)
        .addAssetId(CarControlAssets.Device.VENT)
    .addModeController("position", false, true)
        .addAssetId(CarControlAssets.Setting.STORED_POSITION)
        .addValue("ONE")
            .addAssetId(CarControlAssets.Value.POSITION_ONE)
        .addValue("TWO")
            .addAssetId(CarControlAssets.Value.POSITION_TWO)
        .addValue("THREE")
            .addAssetId(CarControlAssets.Value.POSITION_THREE);

// Front Window
config.createEndpoint("front.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Rear Window
config.createEndpoint("rear.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Driver Window
config.createEndpoint("driver.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Passenger Window
config.createEndpoint("passenger.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Rear Driver Window
config.createEndpoint("rear.driver.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Rear Passenger Window
config.createEndpoint("rear.passenger.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Second Row Window
config.createEndpoint("secondRow.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Third Row Window
config.createEndpoint("thirdRow.window")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addRangeController("height", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.HEIGHT)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

// Front Windshield
config.createEndpoint("front.windshield")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addAssetId(CarControlAssets.Device.WINDSHIELD)
    .addToggleController("defroster", false)
        .addAssetId(CarControlAssets.Setting.DEFROST)
        .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS);

// Rear Windshield
config.createEndpoint("rear.windshield")
    .addAssetId(CarControlAssets.Device.WINDOW)
    .addAssetId(CarControlAssets.Device.WINDSHIELD)
    .addToggleController("defroster", false)
        .addAssetId(CarControlAssets.Setting.DEFROST)
        .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS);

// Front Foglight
config.createEndpoint("front.foglight")
    .addAssetId(CarControlAssets.Device.FOG_LIGHT)
    .addPowerController(false);

// Rear Foglight
config.createEndpoint("rear.foglight")
    .addAssetId(CarControlAssets.Device.FOG_LIGHT)
    .addPowerController(false);

// Hazard Light
config.createEndpoint("hazardlight")
    .addAssetId(CarControlAssets.Device.HAZARD_LIGHTS)
    .addAssetId(CarControlAssets.Device.PARKING_LIGHTS)
    .addPowerController(false);

// Front Wipers
config.createEndpoint("front.wipers")
    .addAssetId(CarControlAssets.Device.WINDSHIELD_WIPERS)
    .addPowerController(false)
    .addModeController("speed", false, true)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addValue("LOW")
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addValue("MEDIUM")
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addValue("HIGH")
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Rear Wipers
config.createEndpoint("rear.wipers")
    .addAssetId(CarControlAssets.Device.WINDSHIELD_WIPERS)
    .addPowerController(false)
    .addModeController("speed", false, true)
        .addAssetId(CarControlAssets.Setting.SPEED)
        .addValue("LOW")
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addValue("MEDIUM")
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addValue("HIGH")
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// Sunroof
config.createEndpoint("sunroof")
    .addAssetId(CarControlAssets.Device.SUNROOF)
    .addAssetId(CarControlAssets.Device.MOONROOF)
    .addRangeController("sunroof.position", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10);

// Sunshade
config.createEndpoint("sunshade")
    .addAssetId(CarControlAssets.Device.SUNSHADE)
    .addRangeController("position", false, 0, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.FULL)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
            .addAssetId(CarControlAssets.Value.HALF)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0)
        .addActionSetRange(new String[] {CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10);

// HUD
config.createEndpoint("hud")
    .addAssetId(CarControlAssets.Device.HUD)
    .addToggleController("power", false)
        .addAssetId(CarControlAssets.Device.HUD)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN})
    .addRangeController("brightness", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.BRIGHTNESS)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM);

// IVI
config.createEndpoint("ivi")
    .addAssetId(CarControlAssets.Device.DISPLAY_SCREEN)
    .addAssetId(CarControlAssets.Device.INFO_SCREEN)
    .addToggleController("power", false)
        .addAssetId(CarControlAssets.Device.DISPLAY_SCREEN)
        .addAssetId(CarControlAssets.Device.INFO_SCREEN)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN})
    .addRangeController("brightness", false, 1, 10, 1, "")
        .addAssetId(CarControlAssets.Setting.BRIGHTNESS)
        .addPreset(1)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
    .addModeController("autobrightness", false, false)
        .addAssetId(CarControlAssets.Setting.BRIGHTNESS)
        .addValue("OPTIMAL")
            .addAssetId(CarControlAssets.Value.OPTIMAL)
        .addValue("AUTO")
            .addAssetId(CarControlAssets.Setting.AUTO);

// Dynamics Coordinator Page
config.createEndpoint("dynamicsCoordinatorPage")
    .addAssetId(CarControlAssets.Value.DYNAMIC_COORDINATOR_PAGE)
    .addToggleController("dynamicsCoordinator.screen", false)
        .addAssetId(CarControlAssets.Value.DYNAMIC_COORDINATOR_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Performance Page
config.createEndpoint("performancePage")
    .addAssetId(CarControlAssets.Value.PERFORMANCE_PAGE)
    .addToggleController("performance.screen", false)
        .addAssetId(CarControlAssets.Value.PERFORMANCE_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Home Page
config.createEndpoint("homepage")
    .addAssetId(CarControlAssets.Value.HOME_PAGE)
    .addToggleController("home.screen", false)
        .addAssetId(CarControlAssets.Value.HOME_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Bluetooth Page
config.createEndpoint("bluetoothPage")
    .addAssetId(CarControlAssets.Value.BLUETOOTH_PAGE)
    .addToggleController("bluetooth.screen", false)
        .addAssetId(CarControlAssets.Value.BLUETOOTH_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Radio Page
config.createEndpoint("radioPage")
    .addAssetId(CarControlAssets.Value.RADIO_PAGE)
    .addToggleController("radio.screen", false)
        .addAssetId(CarControlAssets.Value.RADIO_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Settings Page
config.createEndpoint("settingsPage")
    .addAssetId(CarControlAssets.Value.SETTINGS_PAGE)
    .addToggleController("settings.screen", false)
        .addAssetId(CarControlAssets.Value.SETTINGS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Controls Page
config.createEndpoint("controlsPage")
    .addAssetId(CarControlAssets.Value.CONTROLS_PAGE)
    .addToggleController("controls.screen", false)
        .addAssetId(CarControlAssets.Value.CONTROLS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Navigation Page
config.createEndpoint("navigationPage")
    .addAssetId(CarControlAssets.Value.NAVIGATION_PAGE)
    .addToggleController("navigation.screen", false)
        .addAssetId(CarControlAssets.Value.NAVIGATION_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// GPS Page
config.createEndpoint("gpsPage")
    .addAssetId(CarControlAssets.Value.GPS_PAGE)
    .addToggleController("gps.screen", false)
        .addAssetId(CarControlAssets.Value.GPS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Service Page
config.createEndpoint("servicePage")
    .addAssetId(CarControlAssets.Value.SERVICE_PAGE)
    .addToggleController("service.screen", false)
        .addAssetId(CarControlAssets.Value.SERVICE_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Satellite Radio Page
config.createEndpoint("satelliteRadioPage")
    .addAssetId(CarControlAssets.Value.SATELLITE_RADIO_PAGE)
    .addToggleController("satelliteRadio.screen", false)
        .addAssetId(CarControlAssets.Value.SATELLITE_RADIO_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Information Page
config.createEndpoint("informationPage")
    .addAssetId(CarControlAssets.Value.INFORMATION_PAGE)
    .addToggleController("information.screen", false)
        .addAssetId(CarControlAssets.Value.INFORMATION_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Vehicle Status Page
config.createEndpoint("vehicleStatusPage")
    .addAssetId(CarControlAssets.Value.VEHICLE_STATUS_PAGE)
    .addToggleController("vehicleStatus.screen", false)
        .addAssetId(CarControlAssets.Value.VEHICLE_STATUS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Multimedia Page
config.createEndpoint("multimediaPage")
    .addAssetId(CarControlAssets.Value.MULTIMEDIA_PAGE)
    .addAssetId(CarControlAssets.Value.MUSIC_PAGE)
    .addToggleController("multimedia.screen", false)
        .addAssetId(CarControlAssets.Value.MULTIMEDIA_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Telephone Page
config.createEndpoint("telephonePage")
    .addAssetId(CarControlAssets.Value.TELEPHONE_PAGE)
    .addToggleController("telephone.screen", false)
        .addAssetId(CarControlAssets.Value.TELEPHONE_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Contacts Page
config.createEndpoint("contactsPage")
    .addAssetId(CarControlAssets.Value.CONTACTS_PAGE)
    .addToggleController("contacts.screen", false)
        .addAssetId(CarControlAssets.Value.CONTACTS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Alerts Page
config.createEndpoint("alertsPage")
    .addAssetId(CarControlAssets.Value.ALERTS_PAGE)
    .addToggleController("alerts.screen", false)
        .addAssetId(CarControlAssets.Value.ALERTS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// Notifications Page
config.createEndpoint("notificationsPage")
    .addAssetId(CarControlAssets.Value.NOTIFICATIONS_PAGE)
    .addToggleController("notifications.screen", false)
        .addAssetId(CarControlAssets.Value.NOTIFICATIONS_PAGE)
        .addActionTurnOff(new String[] {CarControlConfiguration.Action.CLOSE})
        .addActionTurnOn(new String[] {CarControlConfiguration.Action.OPEN});

// 360 Camera
config.createEndpoint("360Camera")
    .addAssetId(CarControlAssets.Device.CAMERA_360)
    .addAssetId(CarControlAssets.Device.AVM_CAMERA)
    .addPowerController(false)
    .addModeController("direction", false, true)
        .addAssetId(CarControlAssets.Setting.DIRECTION)
        .addValue("FRONT")
            .addAssetId(CarControlAssets.Location.FRONT)
        .addValue("REAR")
            .addAssetId(CarControlAssets.Location.REAR)
        .addValue("DRIVER")
            .addAssetId(CarControlAssets.Location.DRIVER)
        .addValue("PASSENGER")
            .addAssetId(CarControlAssets.Location.PASSENGER)
        .addValue("AUTO")
            .addAssetId(CarControlAssets.Setting.AUTO);

// Steering Wheel
config.createEndpoint("steeringWheel")
    .addAssetId(CarControlAssets.Device.STEERING_WHEEL)
    .addToggleController("heater", false)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Setting.HEAT);

// Hood
config.createEndpoint("hood")
    .addAssetId(CarControlAssets.Device.HOOD)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("OPEN")
            .addAssetId(CarControlAssets.Value.OPEN)
        .addValue("CLOSED")
            .addAssetId(CarControlAssets.Value.CLOSED)
        .addActionSetMode(new String[] {CarControlConfiguration.Action.CLOSE}, "CLOSED")
        .addActionSetMode(new String[] {CarControlConfiguration.Action.OPEN}, "OPEN");

// Trunk
config.createEndpoint("trunk")
    .addAssetId(CarControlAssets.Device.TRUNK)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("OPEN")
            .addAssetId(CarControlAssets.Value.OPEN)
        .addValue("CLOSED")
            .addAssetId(CarControlAssets.Value.CLOSED)
        .addActionSetMode(new String[] {CarControlConfiguration.Action.CLOSE}, "CLOSED")
        .addActionSetMode(new String[] {CarControlConfiguration.Action.OPEN}, "OPEN");

// Charge Door
config.createEndpoint("chargedoor")
    .addAssetId(CarControlAssets.Device.CHARGE_DOOR)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("OPEN")
            .addAssetId(CarControlAssets.Value.OPEN)
        .addValue("CLOSED")
            .addAssetId(CarControlAssets.Value.CLOSED)
        .addActionSetMode(new String[] {CarControlConfiguration.Action.CLOSE}, "CLOSED")
        .addActionSetMode(new String[] {CarControlConfiguration.Action.OPEN}, "OPEN");

// Gas Door
config.createEndpoint("gasdoor")
    .addAssetId(CarControlAssets.Device.GAS_DOOR)
    .addModeController("position", false, false)
        .addAssetId(CarControlAssets.Setting.POSITION)
        .addValue("OPEN")
            .addAssetId(CarControlAssets.Value.OPEN)
        .addValue("CLOSED")
            .addAssetId(CarControlAssets.Value.CLOSED)
        .addActionSetMode(new String[] {CarControlConfiguration.Action.CLOSE}, "CLOSED")
        .addActionSetMode(new String[] {CarControlConfiguration.Action.OPEN}, "OPEN");


configuration.add(config);

```


</details>

## Using the Car Control Module AASB Messages

The Auto SDK Engine provides an AASB message interface with topic `CarControl` for you to handle the car control directives from Alexa. The messages include an `endpointId` to identify the connected endpoint that Alexa identified to match the user's intent. For directives targeting primitive capability instances, the message includes the `instanceId` as well. The `endpointId` and `instanceId` match the configured IDs from `aace.carControl.endpoints[i].endpointId` and `aace.carControl.endpoints[i].capabilities[j].instance`, respectively.

### Changing the power state of an endpoint

When the user requests Alexa to turn an endpoint on or off, the Engine publishes a [`SetControllerValue` message for power state](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervalue). Your application must power on or off the endpoint and publish [`SetControllerReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervaluereply) in response.

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Turning on AC</summary>
<br/>

![Turning On Endpoint](./diagrams/turning_on_endpoint.png)

</details>

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Turning off AC</summary>
<br/>

![Turning Off Endpoint](./diagrams/turning_off_endpoint.png)

</details>

### Toggling an endpoint property

When the user requests Alexa to turn on or off a named property of an endpoint, the Engine publishes a [`SetControllerValue` message for toggle of the setting](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervalue_2). Your application must turn on or off the property and publish [`SetControllerValueReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervaluereply) in response.

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Turning on the rear windshield defroster</summary>
<br/>

![Turning On Toggle State Setting](./diagrams/turning_on_toggle_state_of_setting.png)

</details>

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Turning off the rear windshield defroster</summary>
<br/>

![Turning Off Toggle State Setting](./diagrams/turning_off_toggle_state_of_setting.png)

</details>

### Changing the mode of an endpoint property

When the user requests Alexa to set the mode of a named property of an endpoint to a specific value, the Engine publishes a [`SetControllerValue` message for mode of the setting](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervalue_3). Your application must set the mode of the property and publish [SetControllerValueReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervaluereply) in response.

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Setting the AC intensity to minimum</summary>
<br/>

![Setting Mode](./diagrams/setting_mode_of_a_setting.png)

</details>

When the user requests Alexa to increase or decrease the mode of a named property of an endpoint, the Engine publishes an [`AdjustControllerValue` message for mode of the setting](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#adjustcontrollervalue_1). Your application must adjust the mode of the property and publish [`AdjustControllerValueReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#adjustcontrollervaluereply) in response.

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Increasing the AC intensity</summary>
<br/>

![Adjust Mode](./diagrams/adjusting_mode_of_a_setting.png)

</details>

### Changing the numeric setting of an endpoint property

When the user requests Alexa to set the numeric setting of a named property of an endpoint to a specific value, the Engine publishes a [`SetControllerValue` message for the range setting](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervalue_1). Your application must set the value of the property and publish [`SetControllerValueReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#setcontrollervaluereply) in response.


<details markdown="1"><summary>Click to expand or collapse sequence diagram: Setting the temperature to 70</summary>
<br/>

![Setting Value](./diagrams/setting_value_of_a_setting.png)

</details>

When the user requests Alexa to adjust (increment or decrement) the numeric setting of a named property of an endpoint by a delta value, the Engine publishes a [`AdjustControllerValue` message for the range setting](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#adjustcontrollervalue). Your application must adjust the value of the property and publish [`AdjustControllerValueReply` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/car-control/CarControl/index.html#adjustcontrollervaluereply) in response.

<details markdown="1"><summary>Click to expand or collapse sequence diagram: Increasing the temperature by 4</summary>
<br/>

![Adjust Value](./diagrams/adjusting_value_of_a_setting.png)

</details>

## Integrating the Car Control Module Into Your Application

### C++ MessageBroker Integration

Use the Engine's `MessageBroker` to subscribe to "CarControl" AASB messages and publish replies.

<details markdown="1"><summary>Click to expand or collapse C++ sample code</summary>

```c++
#include <AACE/CarControl/CarControlConfiguration.h>
#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/CarControl/CarControl/AdjustControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustModeControllerValueMessage.h>

#include <AASB/Message/CarControl/CarControl/SetControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetModeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetPowerControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetToggleControllerValueMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class MyCarControlHandler {

    static const std::string AASB_TOPIC_CAR_CONTROL("CarControl");
    static const std::string AASB_ACTION_SET_CONTROLLER_VALUE("SetControllerValue");
    static const std::string AASB_ACTION_ADJUST_CONTROLLER_VALUE("AdjustControllerValue");

    // Subscribe to "CarControl" messages from the Engine
    void MyCarControlHandler::subscribeToAASBMessages() {
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleSetControllerValueMessage(message); },
            AASB_TOPIC_CAR_CONTROL,
            AASB_ACTION_SET_CONTROLLER_VALUE);

        m_messageBroker->subscribe(
            [=](const std::string& message) { handleAdjustControllerValueMessage(message); },
            AASB_TOPIC_CAR_CONTROL,
            AASB_ACTION_ADJUST_CONTROLLER_VALUE);
    }

    // Handle the messages from the Engine for "SetControllerValue" action
    void MyCarControlHandler::handleSetControllerValueMessage(const std::string& message) {
        json msgJson = json::parse(message);
        
        std::string capabilityType = msgJson["payload"]["capabilityType"];
        if (capabilityType.compare("POWER") == 0) {
            SetPowerControllerValueMessage msg = json::parse(message);
            setPowerControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.turnOn);
        } else if (capabilityType.compare("TOGGLE") == 0) {
            SetToggleControllerValueMessage msg = json::parse(message);
            setToggleControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.turnOn);
        } else if (capabilityType.compare("RANGE") == 0) {
            SetRangeControllerValueMessage msg = json::parse(message);
            setRangeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.value);
        } else if (capabilityType.compare("MODE") == 0) {
            SetModeControllerValueMessage msg = json::parse(message);
            setModeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.value);
        } else {
            // Error. Unsupported controller type in message
        }
    }

    void MyCarControlHandler::setPowerControllerValue(
        const std::string& messageId, 
        const std::string& endpointId, 
        bool turnOn) {
        if (turnOn) {
            // Power on the endpoint represented by endpointId.
            // When complete, call sendSetControllerValueMessageReply() with messageId and the result
        } else {
            // Power off the endpoint represented by endpointId.
            // When complete, call sendSetControllerValueMessageReply() with messageId and the result
        }
    }

    void MyCarControlHandler::setToggleControllerValue(
        const std::string& messageId, 
        const std::string& endpointId, 
        const std::string& instanceId, 
        bool turnOn) {
        if (turnOn) {
            // Turn on the endpoint property represented by endpointId and instanceId.
            // When complete, call sendSetControllerValueMessageReply() with messageId and the result
        } else {
            // Turn off the endpoint property represented by endpointId and instanceId.
            // When complete, call sendSetControllerValueMessageReply() with messageId and the result
        }
    }

    void MyCarControlHandler::setRangeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        double value) {
        // Set the numeric setting of the property represented by endpointId and instanceId to the specified value.
        // When complete, call sendSetControllerValueMessageReply() with messageId and the result   
    }

    void MyCarControlHandler::setModeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        const std::string& value) {
        // Set the mode of the property represented by endpointId and instanceId to the specified value.
        // When complete, call sendSetControllerValueMessageReply() with messageId and the result
    }

    void MyCarControlHandler::sendSetControllerValueMessageReply(const std::string& messageId, bool successful) {
        SetControllerValueMessageReply msg;
        msg.header.messageDescription.replyToId = messageId;
        msg.payload.success = successful;
        m_messageBroker->publish(msg.toString());
    }

    // Handle the messages from the Engine for "AdjustControllerValue" action
    void MyCarControlHandler::handleAdjustControllerValueMessage(const std::string& message) {
        json msgJson = json::parse(message);
        
        std::string capabilityType = msgJson["payload"]["capabilityType"];
        if (capabilityType.compare("RANGE") == 0) {
            AdjustRangeControllerValueMessage msg = json::parse(message);
            adjustRangeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.delta);
        } else if (capabilityType.compare("MODE") == 0) {
            AdjustModeControllerValueMessage msg = json::parse(message);
            adjustModeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.delta);
        } else {
            // Error. Unsupported controller type in message
        }
    }

    void MyCarControlHandler::adjustRangeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        double delta) {
        // Adjust the numeric setting of the property represented by endpointId and instanceId by the specified delta.
        // When complete, call sendAdjustControllerValueMessageReply() with messageId and the result
    }

    void MyCarControlHandler::adjustModeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        double delta) {
        // Adjust the mode of the property represented by endpointId and instanceId by the specified delta.
        // When complete, call sendAdjustControllerValueMessageReply() with messageId and the result
    }

    void MyCarControlHandler::sendAdjustControllerValueMessageReply(const std::string& messageId, bool successful) {
        AdjustControllerValueMessageReply msg;
        msg.header.messageDescription.replyToId = messageId;
        msg.payload.success = successful;
        m_messageBroker->publish(msg.toString());
    }


```
</details>

### AACS Android integration

The Alexa Auto Client Service (AACS) provides the `AACS Car Control Library` to integrate the Auto SDK `Car Control` module on Android. See the [AACS Car Control Library documentation](https://alexa.github.io/alexa-auto-sdk/docs/android/aacs/app-components/alexa-auto-carcontrol/) for more information.
