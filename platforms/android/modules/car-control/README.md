# Car Control Module

The Alexa Auto SDK Car Control module enables you to build a custom experience that allows users to use Alexa to voice-control vehicle features.

**Table of Contents:**

* [Conceptual Overview](#conceptual-overview)
    * [Endpoints](#endpoints-overview)
    * [Capabilities](#capabilities-overview)
    * [Zones](#zones-overview)
    * [Assets](#assets-overview)
* [Configuring the Car Control Module](#configuring-the-car-control-module)
    * [Configuration Schema Overview](#config-schema)
    * [Power Controller Capability Configuration](#power-controller-config)
    * [Toggle Controller Capability Configuration](#toggle-controller-config)
    * [Mode Controller Capability Configuration](#mode-controller-config)
    * [Range Controller Capability Configuration](#range-controller-config)
    * [Additional Notes about Assets](#additional-notes-assets)
    * [Sample Configuration](#sample-config)
* [Implementing the Car Control Platform Interface](#implementing-platform-interface)

## Conceptual Overview <a id ="conceptual-overview"></a>

### Endpoints <a id ="endpoints-overview"></a>

The head unit device acting as an Alexa Auto SDK client is an "endpoint" that connects to the Alexa service. Other Auto SDK modules, such as [Alexa](../alexa/README.md), configure capabilities on this "root" or "default" endpoint because the capabilities pertain to the head unit itself. The Car Control module enables the default Auto SDK client endpoint to act as a proxy to receive events and directives on behalf of connected endpoints. You can configure a separate endpoint for every vehicle component that the head unit can control through device-level connections. This enables the user to target individual vehicle components directly with utterances like "Alexa, turn on the AC" or "Alexa, set the temperature to 65."

### Capabilities <a id ="capabilities-overview"></a>

In the utterance "Alexa, turn on the AC", "turn on" corresponds to a specific capability configured for the "AC" endpoint. Defining an endpoint declares a vehicle feature to be controllable, and defining capabilities on the endpoint declares *how* the endpoint can be controlled. 

Car Control supports four capability interfaces that can be declared alone or in combination for a particular endpoint to model its individual control experience:

* **Power Controller** controls the overall power state of an endpoint. For example, configuring an "AC" endpoint with a Power Controller capability enables utterances such as "Alexa, turn on the AC" and "Alexa, power off the AC".
* **Toggle Controller** controls a particular named property of an endpoint that can be turned on and off. For example, configuring a "windshield" endpoint with a "defroster" Toggle Controller capability instance enables utterances such as "Alexa, turn on the windshield defroster." 
* **Mode Controller** controls a particular named property of an endpoint that can be set to a discrete value from a defined set of values. For example, if an ambient light endpoint has red and green color settings, configuring an "ambient light" endpoint with a "color" Mode Controller capability instance enables utterances such as "Alexa, set the ambient light color to red" and "Alexa, change the ambient light to green."
* **Range Controller** controls a particular named property of an endpoint that can be set to a numeric value within a range. For example, if a fan endpoint has a speed property with settings 1 through 3, configuring a "fan" endpoint with a "speed" Range Controller capability instance enables utterances such as "Alexa, set the fan speed to 2." You can configure names, such as "medium", for a range value to enable additional utterances such as "Alexa, set the fan to medium" to set the fan speed setting to 2. 

#### Capability Primitives and Semantic Annotations <a id ="primitives-semantics"></a>
Toggle Controller, Mode Controller, and Range Controller are known as "capability primitives." You can use multiple instances of the same capability primitive interface on an endpoint under different instance names. For example, a heater endpoint might have intensity and position properties that are both best modeled as modes. You can declare an "intensity" Mode Controller instance and a "position" Mode Controller instance on the same "heater" endpoint so the user can target each property separately. 

To provide intuitive experiences for users, capability primitives offer "semantic annotations" for the devices to map specific utterances to the behaviors of capability instances. For example, if the vehicle uses a Range Controller to control a window, a user would prefer to say "Alexa, open the window" over the default utterances of the Range Controller such as "Alexa, set the window height to 0". For any endpoint to which the "open", "close", "raise", or "lower" concepts apply, you can configure the capability primitive instances of the endpoint with a "semantics" object that maps user utterances for these actions to the appropriate capability directives. Each action (e.g., "open") is allowed only once per endpoint since the action expresses intent to control the endpoint as a whole.

The actions specified in configuration are action IDs rather than literal strings, which ensures Alexa recognizes all synonyms and translations for the action in the user utterance. The supported actions are "Alexa.Actions.Open", "Alexa.Actions.Close", "Alexa.Actions.Raise", and "Alexa.Actions.Lower."
    
### Zones <a id ="zones-overview"></a>

Each endpoint can belong to zero, one, or many "zones." Zones, configured with member endpoints, define named regions of the vehicle and allow users to target endpoints by location. Zones are essential for unambiguous targeting of endpoints that have friendly names that overlap with other endpoints. For example, defining "driver" and "passenger" zones and assigning distinct "seat" endpoints to each allows proper control of the "driver seat" and the "passenger seat" independently. 

Assigning one zone in particular as the "default" enables endpoints in this zone to take precedence over endpoints sharing the same friendly name but not in the default zone when the user does not specify a zone in the utterance. This is useful for distinguishing "zoneless" endpoints from "zoned" endpoints with the same name when it is most likely that the user intends to target the "zoneless" one. For example, consider a vehicle with zone IDs "zone.all", "zone.rear", and "zone.left" with a distinct fan endpoint in each zone. If the user says "Alexa, turn on the fan", it is most likely that he wants to turn on the fan that refers to the vehicle as a whole because there is no natural way to specify its location. You can ensure that Alexa will resolve this utterance to the fan in the "all" zone by assigning "zone.all" as the default zone.

Additionally, the default zone is useful for cases in which you have zoned endpoints with overlapping names, but one of the endpoints is a clear "default" to the user. For example, consider a vehicle with zones "zone.all" (assigned as default), "zone.driver", and "zone.passenger". The vehicle has a "driver window" in "zone.driver" and a "passenger window" in "zone.passenger", but Alexa cannot resolve which endpoint is the intended target of the user utterance "Alexa, open the window." However, the user probably means "Alexa, open the *driver* window". You can ensure that Alexa considers the "driver window" as the "default" window by assigning it to "zone.all" as well.

### Assets <a id ="assets-overview"></a>

The definitions of endpoints, capabilities, and zones include "assets." Assets, identified by unique IDs, group a voice-accessible friendly name like "air conditioner" into a named group of synonyms and translations for all supported languages. For example, using the asset with ID "Alexa.Automotive.DeviceName.AirConditioner" in your car control module configuration for an AC endpoint not only enables the user to target the air conditioner with the default phrase "air conditioner", but also with phrases like "air con" and "AC" in English as well as synonyms in other supported locales.

Using assets allows decoupling the many ways of identifying components from the core configuration of the components and enables de-duplication across different components that have overlapping ways to be identified.

The Alexa Auto SDK provides a list of IDs for the "default assets." The default assets comprise an automotive-specific catalog of asset definitions for supported features (including endpoint names, zone names, and capability settings) for car control. The automotive catalog of assets uses IDs with prefix "Alexa.Automotive." You can use these asset IDs in your car control module configuration without the corresponding definitions of friendly names, synonyms, and translations because the definitions are specified internally.

### Summary 

By combining these elements, you can build a [Car Control module configuration](#configuring-the-car-control-module), which the Auto SDK Engine uses to advertise the features available in the vehicle to Alexa, and implement the corresponding [platform interface](#implementing-platform-interface) to map the directives from a user's voice request to the intended target feature.


### Car Control Module Architecture <a id ="car-control-architecture"></a>

The following diagram illustrates the high-level Car Control module architecture:

<p align="center">
<img src="./assets/carcontrol_online.png"/>
</p>

* The Auto SDK Car Control module uses the Car Control configuration and assets to build the infrastructure necessary to operate.
* Car control directives are sent to the Car Control module from the Alexa cloud.

### Car Control Sequence Diagram

The following diagram illustrates a sample car control sequence based on the utterance "Alexa, set the temperature to 70":

<p align="center">
<img src="./assets/carcontrol_sequence_online.png" />
</p>


## Configuring the Car Control Module <a id="configuring-the-car-control-module"></a>
Car control module configuration is vehicle-specific and tells the Auto SDK Engine which vehicle features to advertise to Alexa for control by the user. You must configure the Auto SDK Engine with an `EngineConfiguration` object that describes the vehicle. Like all Auto SDK Engine configuration, you can either define the JSON in a file and construct an `EngineConfiguration` from that file, or you can use the provided `CarControlConfiguration` class to programmatically construct the `EngineConfiguration` in the proper format. The following subsections describe the JSON schema. See the [com.amazon.aace.carControl.CarControlConfiguration](../car-control/src/main/java/com/amazon/aace/carControl/CarControlConfiguration.java) class for details on how to build configuration programmatically.

### Configuration Schema Overview <a id="config-schema"></a>
The `EngineConfiguration` for the Car Control module includes definitions of endpoints with their capabilities, zones with their member endpoints, and an optional path to a JSON file defining additional assets.

**Sample JSON Object**

```jsonc
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
| aace.carControl.<br>endpoints[i].<br>endpointId | string | Yes | The identifier for the endpoint, unique amongst all endpoints in the vehicle. The same `endpointId` is used in the [platform interface](#handling-car-control-functionality) to identify the endpoint targeted by a method invocation.<br>**Note:** Do not use this format for the `endpointId`: <br>\<clientId\>::\<productId\>::\<serialNumber\>::[-\<extEndpoint\>]<br><br>The Engine internally prepends the 3-part device prefix to your specified `endpointId` before sending the configuration to Alexa. Configuring the full ID directly results in duplication and excess characters. |
| aace.carControl.<br>endpoints[i].<br>endpointResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this endpoint.<br><br>**Note:** Only `“asset”` type labels are supported. |
| aace.carControl.<br>endpoints[i].<br>endpointResources.<br>friendlyNames[j].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to the endpoint. <br>The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-assets) for more details.|
| aace.carControl.<br>endpoints[i].<br>capabilities | list | Yes | A list of capability definitions, representing capabilities implemented by the device on behalf of the endpoint, that define how the endpoint can be controlled by the user. <br><br>Each object in this list must be a valid definition for one of the capabilities supported by the car control module:<br>[Alexa.PowerController](#power-controller-config), [Alexa.ToggleController](#toggle-controller-config), [Alexa.ModeController](#mode-controller-config), and [Alexa.RangeController](#range-controller-config) |
| aace.carControl.<br>zones | list | No, but recommended | A list of zone definitions for the named regions in the vehicle. |
| aace.carControl.<br>zones[i].<br>zoneId | string | Yes | The identifier for the zone, unique amongst all zones in the vehicle. |
| aace.carControl.<br>zones[i].<br>zoneResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible ways to refer to this zone.<br><br>**Note:** Only `“asset”` type labels are supported. |
| aace.carControl.<br>zones[i].<br>zoneResources.<br>friendlyNames[j].<br>assetId | string | Yes | The ID of an asset definition that includes the list of strings used to refer to the zone in all supported locales.<br>The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-assets) for more details.  |
| aace.carControl.<br>zones[i].<br>members | list | Yes | A list of endpoints that belong to this zone. |
| aace.carControl.<br>zones[i].<br>members[j].<br>endpointId | string | Yes | The `endpointId` for an endpoint that belongs to this zone. |
| aace.carControl.<br>defaultZoneId | string | No, but recommended | The `zoneId` of the default zone. Endpoints in this zone take precedence when a user utterance does not specify a zone. <br> It is recommended to use a zone that describes the whole vehicle as the default rather than a zone describing a specific region. |
| aace.carControl.<br>assets.customAssetsPath | string<br>(file path) | No | Specifies the path to a JSON file defining additional assets. |


### Power Controller Capability Configuration <a id="power-controller-config"></a>

<details><summary>Click to expand or collapse description</summary>

See ["Alexa.PowerController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-powercontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```jsonc
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


### Toggle Controller Capability Configuration <a id="toggle-controller-config"></a>

<details><summary>Click to expand or collapse description</summary>

See ["Alexa.ToggleController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-togglecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```jsonc
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
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-assets) for more details.  |
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "toggleState") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "powerState") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |
| semantics | object | No | Semantic annotations that enable mapping user utterances with directives targeting this capability instance.<br><br>**Note:** `semantics.stateMappings` is not supported. |
| semantics.<br>actionMappings[i].<br>actions[j] | string | Yes, if `semantics` is present | The identifiers of the utterances that should trigger the specified directive.<br><br>**Accepted values:** <ul> <li>`"Alexa.Actions.Open"`: "open {endpoint}"</li> <li>`"Alexa.Actions.Close"`: "close {endpoint}"</li> <li>`"Alexa.Actions.Raise"`: "raise {endpoint}"</li> <li>`"Alexa.Actions.Lower"`: "lower {endpoint}"</li> </ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:** <ul> <li>`"TurnOn"`: The specified `actions` will trigger the "TurnOn" directive, which will invoke `com.amazon.aace.carControl.CarControl.turnToggleControllerOn()`.</li> <li>`"TurnOff"`: The specified `actions` will trigger the "TurnOff" directive, which will invoke `com.amazon.aace.carControl.CarControl.turnToggleControllerOff()`.</li> </ul> |

</details>

### Mode Controller Capability Configuration <a id="mode-controller-config"></a>

<details><summary>Click to expand or collapse description</summary>

See ["Alexa.ModeController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-modecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```jsonc
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
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-assets) for more details.  |
| properties.<br>proactivelyReported | boolean | Yes | Whether the reportable state properties for this capability (i.e., "mode") can be proactively reported to Alexa via an event.<br><br>**Accepted values:**<br>`false` |
| properties.<br>retrievable | boolean | Yes | Whether the reportable state properties for this capability (i.e., "mode") can be retrieved by Alexa.<br><br>**Accepted values:**<br>`false` |
| configuration.<br>ordered | boolean | Yes | Whether the modes of this capability instance are ordered, enabling iteration through them using the "AdjustMode" directive. |
| configuration.<br>supportedModes | list | Yes | A list of objects describing the available modes of this capability instance. If `ordered` is true, the order of the objects in this list implies the ordering of the modes. |
| configuration.<br>supportedModes[i].<br>value | string | Yes | The identifier of this mode on this capability instance. |
| configuration.<br>supportedModes[i].<br>modeResources.<br>friendlyNames | list | Yes | A list of label objects that describe the possible friendly names for this mode.<br><br>**Note:** Only `“asset”` type labels are supported. |
| semantics | object | No | Semantic annotations that enable mapping user utterances with directives targeting this capability instance.<br><br>**Note:** `semantics.stateMappings` is not supported. |
| semantics.<br>actionMappings[i].<br>actions[j] | string | Yes, if `semantics` is present | The identifiers of the utterances that should trigger the specified directive.<br><br>**Accepted values:** <ul><br><li>`"Alexa.Actions.Open"`: "open {endpoint}"</li><li>`"Alexa.Actions.Close"`: "close {endpoint}"</li><li>`"Alexa.Actions.Raise"`: "raise {endpoint}"</li><li>`"Alexa.Actions.Lower"`: "lower {endpoint}"</li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:**<br><ul><br><li>`"SetMode`: The specified actions will trigger the "SetMode" directive with the specified `payload`. The directive will invoke `com.amazon.aace.carControl.CarControl.setModeControllerValue()`.</li><li>`"AdjustMode`: The specified actions will trigger the "AdjustMode" directive with the specified `payload`. The directive will invoke `com.amazon.aace.carControl.CarControl.adjustModeControllerValue()`. "AdjustMode" is accepted only if this capability instance is `ordered`. </li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>payload | object | Yes, if `semantics` is present | If `name` is “SetMode”, this is the “SetMode” directive payload object that contains the “mode” property and the corresponding value from `configuration.supportedModes[].value`.<br><br>If `name` is “AdjustMode”, this is the “AdjustMode” directive payload object that contains the “modeDelta” field and the corresponding number of modes to advance. |

</details>

### Range Controller Capability Configuration <a id="range-controller-config"></a>

<details><summary>Click to expand or collapse description</summary>

See ["Alexa.RangeController" interface AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alexa-rangecontroller.html) for additional details, but note that only features described in this document are supported for car control.

**Sample JSON Object**

```jsonc
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
| capabilityResources.<br>friendlyNames[i].<br>assetId | string | Yes | The ID of an asset definition that includes the list of localized strings used to refer to this capability instance. The asset ID must be a valid ID from the automotive catalog of default assets or a custom assets definition configured in the file at `aace.carControl.assets.customAssetsPath`. See the ["Additional Notes about Assets" section](#additional-notes-assets) for more details.  |
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
| semantics.<br>actionMappings[i].<br>directive.<br>name | string | Yes, if `semantics` is present | **Accepted values:**<br><ul><li>`"SetRangeValue`: The specified actions will trigger the "SetRangeValue" directive with the specified `payload`. The directive will invoke `com.amazon.aace.carControl.CarControl.setRangeControllerValue()`.</li><li>`"AdjustRangeValue`: The specified actions will trigger the "AdjustRangeValue" directive with the specified `payload`. The directive will invoke `com.amazon.aace.carControl.CarControl.adjustRangeControllerValue()`.</li></ul> |
| semantics.<br>actionMappings[i].<br>directive.<br>payload | object | Yes, if `semantics` is present | If `name` is “SetRangeValue”, this is the “SetRangeValue” directive payload object that contains the "rangeValue" property and the corresponding value between `configuration.supportedRange.minumumValue` and `configuration.supportedRange.maximumValue`.<br><br>If `name` is “AdjustRangeValue”, this is the “AdjustRangeValue” directive payload object that contains the “rangeValueDelta” field. |

</details>

### Additional Notes about Assets <a id="additional-notes-assets"></a>

The Car Control module provides the full list of asset IDs available in the default automotive catalog of assets in the [com.amazon.aace.carControl.CarControlAssets](../car-control/src/main/java/com/amazon/aace/carControl/CarControlAssets.java) class, which defines string constants for these asset IDs to be used when constructing configuration programmatically. The values of the constants are the same asset IDs that you should use if you construct your configuration in a JSON file.

This module also provides the definitions of the assets in the automotive catalog in a [JSON file](../../../../modules/car-control/assets/assets-1P.json) for reference. Your implementation does not need to duplicate these asset definitions or specify the path to this file since the definitions also exist internally. This copy of the file is a reference for you to see the synonyms and translations for the available assets.

The automotive catalog of assets defines assets for every feature officially supported by car control. The majority of your configuration will use these asset IDs, and it is not recommended to redefine new, custom assets for any of the features that already exist in the default catalog. However, if your vehicle has a feature that cannot be described using the default assets (e.g., an endpoint with a proprietary name), you can define an additional JSON file defining a complementary set of assets to use alongside the default catalog. The format of this file must follow the same schema as the [default assets JSON](../../../../modules/car-control/assets/assets-1P.json), and the definitions must include entries for each of the locales supported in the default catalog. Prefix every `assetId` in this file with `"My."`, and specify the path to the file in the optional `aace.carControl.assets.customAssetsPath` field of configuration.

>**Note for hybrid systems with LVC:** If you use custom assets, there are two distinct configurations, the Auto SDK Engine configuration and the LVC APK `ILVCClient` configuration, that expect a path to this file. Either configure the same path in both configurations, ensuring the processes of both applications can access the file, or create a distinct but identical copy for each application if the applications cannot access the files in the other APK.


### Sample Configuration <a id="sample-config"></a>

The Car Control module provides a [sample JSON file](../../../../modules/car-control/assets/CarControlConfig.json) to configure the Auto SDK Engine with a vehicle fully equipped for every use case officially supported for car control. This file models each supported endpoint with a configuration of capabilities and zones that ensures all supported utterances for that endpoint work as expected. It is recommended that you construct your configuration by selecting the parts of this sample that describe features supported by your vehicle. Make adjustments to the endpoints, such as modifying modes and range settings, as needed.

If your implementation constructs the Car Control module `EngineConfiguration` programmatically rather than with a JSON file, see the following example usage of the [com.amazon.aace.carControl.CarControlConfiguration](../car-control/src/main/java/com/amazon/aace/carControl/CarControlConfiguration.java) builder class that produces the same fully-equipped vehicle as the sample file:

<details><summary>Click to expand sample code</summary>

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


## Implementing the Car Control Platform Interface <a id="implementing-platform-interface"></a>

The Auto SDK Engine manages internal state information for your configured car control endpoints and provides a simple interface for you to handle the car control directives from Alexa. To implement the required platform interface handler for car control, extend the [`com.amazon.aace.CarControl`](../car-control/src/main/java/com/amazon/aace/carControl/CarControl.java) class.

The methods in the `CarControl` platform interface include an `endpointId` to identify the connected endpoint that Alexa identified to match the user's intent. For directives targeting primitive capability instances, the method includes the `controllerId` as well. The `endpointId` and `controllerId` match the configured IDs from `aace.carControl.endpoints[i].endpointId` and `aace.carControl.endpoints[i].capabilities[j].instance`, respectively.

Override the methods of `CarControl` to interface with the endpoint capabilities specified as arguments in the method invocations:

```java
import com.amazon.aace.carControl.CarControl;

public class CarControlHandler extends CarControl {

    /// PowerController

    @Override
    public void turnPowerControllerOn(String endpointId) throws Exception {
        // Turn on the endpoint identified by "endpointId". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public void turnPowerControllerOff(String endpointId) throws Exception {
        /// Turn off the endpoint identified by "endpointId". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public boolean isPowerControllerOn(String endpointId) throws Exception {
        // For the endpoint identified by "endpointId", return "true" if the endpoint state is on or "false" if it is off. Throw an Exception if the device cannot perform the requested action.
    }

    /// ToggleController

    @Override
    public void turnToggleControllerOn(String endpointId, String controllerId) throws Exception {
        // For the endpoint identified by "endpointId", turn on the property instance identified by "controllerId". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public void turnToggleControllerOff(String endpointId, String controllerId) throws Exception {
        // For the endpoint identified by "endpointId", turn off the property instance identified by "controllerId". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public boolean isToggleControllerOn(String endpointId, String controllerId) throws Exception {
        // For the endpoint identified by "endpointId" and property instance identified by "controllerId", return "true" if the property is on or "false" if it is not. Throw an Exception if the device cannot perform the requested action.
    }

    /// RangeController

    @Override
    public void setRangeControllerValue(String endpointId, String controllerId, double value) throws Exception {
        // For the endpoint identified by "endpointId", set the property instance identified by "controllerId" to the specified "value". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public void adjustRangeControllerValue(String endpointId, String controllerId, double delta) throws Exception {
        // For the endpoint identified by "endpointId", adjust the property instance identified by "controllerId" by the specified "delta" value. Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public double getRangeControllerValue(String endpointId, String controllerId) throws Exception {
        // For the endpoint identified by "endpointId", return the value of the property instance identified by "controllerId". Throw an Exception if the device cannot perform the requested action.
    }

    /// ModeController

     @Override
    public void setModeControllerValue(String endpointId, String controllerId, String value) throws Exception {
        // For the endpoint identified by "endpointId", set the property instance identified by "controllerId" to the specified "value". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public void adjustModeControllerValue(String endpointId, String controllerId, int delta) throws Exception {
        // For the endpoint identified by "endpointId", adjust the property instance identified by "controllerId" by the specified "delta". Throw an Exception if the device cannot perform the requested action.
    }
    @Override
    public String getModeControllerValue(String endpointId, String controllerId) throws Exception {
        // For the endpoint identified by "endpointId", return the value of the property instance identified by "controllerId". Throw an Exception if the device cannot perform the requested action.
    }
};

...

// Register the platform interface implementation with the Engine
mEngine.registerPlatformInterface(new CarControlHandler());

```
