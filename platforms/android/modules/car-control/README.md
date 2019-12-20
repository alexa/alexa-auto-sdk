# Car Control Module

The Alexa Auto SDK Car Control module enables you to build a custom vehicle-control experience that allows the user to voice-control vehicle features using Alexa.

**Table of Contents:**

* [Overview](#overview)
* [Configuring the Car Control Module](#configuring-the-car-control-module)
* [Handling Car Control Functionality](#handling-car-control-functionality)
* [Configuration Examples](#configuration-examples)

## Overview <a id ="overview"></a>
Using the Car Control module, you can build a custom vehicle-control experience comprised of the following elements:

* **The concept of endpoint control:** Different components or "endpoints" in the vehicle can be controlled separately. Endpoints are identified by the names of the component they control (e.g. "fan", "vent", or "heater"), which allows the user to target the selected endpoint by voice.
* **A simple set of control capabilities:** A component is made controllable when you associate one or more of the predefined control capabilities with the component. You can define the following control capabilities in your vehicle without the need for Amazon to define any additional voice models:

    * **Power Controller** controls the overall power state of an endpoint (for example, "turn on the fan").
    * **Toggle Controller** turns components of endpoints on or off (for example, "turn on the windshield defroster").
    * **Range Controller** sets an endpoint property to a value within a minimum and maximum range (for example, "set the temperature to 65").
    * **Mode Controller** sets an endpoint property to a setting from a list of values (for example, "set the air conditioner to 'automatic'").
* **The concept of zone:** Each component control belongs to a "zone" or to the entire vehicle. A zone is identified by its name (e.g. "driver", "front", or "all").
    >**Note:**  For online-only use cases (not using the optional Local Voice Control (LVC) extension), each component control must belong to the predefined default zone "zone.all". Additional zones are not supported.
* **The concept of asset:** All components are identified using assets, which are the different synonyms that users can use to express the names of the components as they interact with Alexa in all the supported locales.    
    
By combining these elements, you can build a custom configuration that defines the controllable endpoints in the vehicle and the associated control capabilities to match the specific vehicle you wish to control. By configuring the Auto SDK Engine with this [configuration](#programmatic-configuration) and implementing the [platform interface](#handling-car-control-functionality) provided by the Car Control module, you allow the user to access the set of controls defined for the vehicle by voice.

### Car Control Architecture <a id ="car-control-architecture"></a>

The following diagram illustrates the high-level Car Control module architecture:

<p align="center">
<img src="./assets/carcontrol_online.png" />
</p>

* The Auto SDK Car Control module uses the Car Control configuration and assets to build the infrastructure necessary to operate.
* Car control directives are sent to the Car Control module from the Alexa cloud.

### Car Control Sequence Diagram

The following diagram illustrates a sample car control sequence based on the utterance "set temperature to 70":

<p align="center">
<img src="./assets/carcontrol_sequence_online.png" />
</p>

## Configuring the Car Control Module <a id="configuring-the-car-control-module"></a>
To use car control features, you must configure the Auto SDK Engine with an `EngineConfiguration` object that describes the endpoints defined for the vehicle. You can either define the JSON explicitly in a file and construct an `EngineConfiguration` from that file, or you can use the [factory method](#programmatic-configuration) provided by the Car Control module to programmatically construct the `EngineConfiguration` in the proper format. 

The `EngineConfiguration` for the Car Control module includes paths to JSON files that include the default and custom assets, a list of [endpoints](#endpoints) with their capabilities, and a list of [zones](#zones). The format of the configuration is as follows, and the elements comprising this configuration are described in detail below:

```
{
    "aace.carControl": {
        "assets": {
            "defaultAssetsPath": "/path/to/default/assets.json",
            "customAssetsPath": "/path/to/custom/assets.json"
        },
        "endpoints": [
            ...
        ],
        "zones": [
            ...
        ]
    }
}
```

### Endpoints <a id="endpoints"></a>

An `endpoint` describes a component that can be controlled in the vehicle (e.g. fan or heater) and how to access it with voice interaction.

An endpoint description includes

* A unique identifier `endpointId` used in the [platform interface](#handling-car-control-functionality) to identify the endpoint targeted by a method invocation
* A `friendlyNames` array of [assets](#assets) used to name the particular endpoint
* A set of [capabilities](#capabilities) describing the different controllers of the endpoint
* The `zone` to which the endpoint belongs. Each endpoint belongs to only one zone.

 >**Note:**  For online-only implementations without the optional LVC extension, all endpoints must belong to the default "zone.all" zone.
 
<details><summary>Example endpoint definition (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "all.fan",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.Fan"
                }
            }
        ]
    },
    "capabilities": [
        ...
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

### Zones <a id="zones"></a>

>**Important:**  Online-only implementations without the optional LVC extension support only the default "zone.all" zone. The Engine configuration will fail if you define other zones in the EngineConfiguration object for an online-only configuration. The remainder of this section applies only to hybrid systems with LVC.

`Zones` introduce location granularity to different endpoints in specific parts of the vehicle and describe how to access them by voice. For example, a vehicle can provide the user with the ability to control the fan for the entire vehicle, for the driver, for the passenger, and for the rear seats independently. 

A zone description includes

* A unique identifier `zoneId` used in the endpoint definition
* A `friendlyNames` array of [assets](#assets)  used to name the particular zone

<details><summary>Example zone definition (click to expand or collapse)</summary>
<p>

```json
{
    "zoneId": "zone.all",
    "zoneResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Location.All"
                }
            }
        ]
    }
}
```
</p>  
</details>

A set of default zones, with the predefined zoneIds defined as [constants](./src/main/java/com/amazon/aace/carControl/CarControlConfiguration.java), is included in the `com.amazon.aace.carControl` package. This set of zones covers a range of use cases required to describe the controllable zones of a vehicle.  
<br>
<details>
<summary>Predefined zoneIds (click to expand or collapse)</summary>
<p>

```
    "zone.all"
    "zone.driver"
    "zone.driver.row"
    "zone.first.row"
    "zone.fourth.row"
    "zone.front"
    "zone.passenger"
    "zone.passenger.row"
    "zone.rear"
    "zone.rear.driver"
    "zone.rear.passenger"
    "zone.second.row"
    "zone.third.row"
```

</p>
</details>
<br>
For hybrid implementations using the optional LVC extension, you can add complementary zones alongside the defaults if a zone in your vehicle cannot be described using one of the predefined zoneIds. Your custom zone should be included in the configuration.  
<br>
<details>
<summary>Example complementary zone definition (click to expand or collapse)</summary>
<p>

```json
{
    "zoneId": "custom.zone",
    "zoneResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "My.custom.zone"
                }
            }
        ]
    }
}
```

</p>
</details>

### Assets <a id="assets"></a>

Both [`endpoint`](#endpoint) and [`zone`](#zone) objects in the configuration include `assets`. An asset defines a mapping between an identifier and the one or more ways to name a specific object that uses the identifier. Assets allow you to customize voice interactions so the user can use more than one name for an object. They also prevent the configuration from duplicating values. Endpoints and zones use `friendlyNames` (arrays of synonymous label objects) to identify the assets they use. 

An asset description includes:

* A unique identifier `assetId` that names the asset. 
* A `defaultValue` that specifies the default name for an object identified by the asset.
* A `synonyms` list that provides possible alternatives to identify the `defaultValue`.
* A value object in the `values` array that allows you to specify the locales to which this asset belongs. You can specify different values for different locales. However, only US English (`en-US`) is currently supported.

<details><summary>Example fan asset definition (click to expand or collapse)</summary>

```json
{
    "version": 1,
    "assets": [
        {
            "assetId": "Alexa.DeviceName.Fan",
            "values": [
                {
                    "locales": [
                        "en-US"
                    ],
                    "defaultValue": "fan",
                    "synonyms": [
                        "blower",
                        "air flow"
                    ]
                }
            ]
        }
    ]
}
```

</p>
</details>  

A set of default assets is defined in the Auto SDK Engine. This set of assets covers a wide range of use cases required to describe the controllable endpoints of a vehicle. The names of the predefined assetIds are defined as [constants](./src/main/java/com/amazon/aace/carControl/CarControlAssets.java) in the com.amazon.aace.carControl package for easy use. Should you need to change the values of the default assets, you can provide a path to a file that defines them in the optional `defaultAssetsPath` field in the `assets` node of the configuration. The [file](./assets/assets-1P.json) is provided for you to copy, modify, and install in your chosen location if needed.  

<details><summary>Predefined assetIds (click to expand or collapse)</summary>
<p>

```
    "Alexa.DeviceName.AirConditioner"
    "Alexa.DeviceName.AmbientLight"
    "Alexa.DeviceName.CabinLight"
    "Alexa.DeviceName.Car"
    "Alexa.DeviceName.ClimateControl"
    "Alexa.DeviceName.Cooler"
    "Alexa.DeviceName.DomeLight"
    "Alexa.DeviceName.Fan"
    "Alexa.DeviceName.Heater"
    "Alexa.DeviceName.Light"
    "Alexa.DeviceName.ReadingLight"
    "Alexa.DeviceName.TrunkLight"
    "Alexa.DeviceName.Vent"
    "Alexa.DeviceName.Window"
    "Alexa.DeviceName.Windshield"
    "Alexa.Location.All"
    "Alexa.Location.Driver"
    "Alexa.Location.DriverRow"
    "Alexa.Location.FirstRow"
    "Alexa.Location.FourthRow"
    "Alexa.Location.Front"
    "Alexa.Location.Passenger"
    "Alexa.Location.PassengerRow"
    "Alexa.Location.Rear"
    "Alexa.Location.RearDriver"
    "Alexa.Location.RearPassenger"
    "Alexa.Location.SecondRow"
    "Alexa.Location.ThirdRow"
    "Alexa.Setting.AirRecirculation"
    "Alexa.Setting.Auto"
    "Alexa.Setting.BodyVents"
    "Alexa.Setting.ClimateSync"
    "Alexa.Setting.Color"
    "Alexa.Setting.Defog"
    "Alexa.Setting.Defrost"
    "Alexa.Setting.Economy"
    "Alexa.Setting.FanSpeed"
    "Alexa.Setting.FloorVents"
    "Alexa.Setting.Heat"
    "Alexa.Setting.Intensity"
    "Alexa.Setting.Manual"
    "Alexa.Setting.MixVents"
    "Alexa.Setting.Mode"
    "Alexa.Setting.Position"
    "Alexa.Setting.Temperature"
    "Alexa.Setting.WindshieldVents"
    "Alexa.Unit.Temperature.Celsius"
    "Alexa.Unit.Temperature.Fahrenheit"
    "Alexa.Value.Blue"
    "Alexa.Value.Green"
    "Alexa.Value.Indigo"
    "Alexa.Value.Orange"
    "Alexa.Value.Red"
    "Alexa.Value.Violet"
    "Alexa.Value.White"
    "Alexa.Value.Yellow"
    "Alexa.Value.High"
    "Alexa.Value.Low"
    "Alexa.Value.Maximum"
    "Alexa.Value.Medium"
    "Alexa.Value.Minimum"
```

</p>
</details>
<br>
You can add complementary assets alongside the defaults if an endpoint in your vehicle cannot be described using one of the predefined assetIds. To do this, define your custom assets in a single JSON file and specify the path to that file in the optional `customAssetsPath` field in the `assets` element of the configuration.

<details>
<summary>Example complementary asset definition (click to expand or collapse)</summary>
<p>

```json
{
    "version": 1,
    "assets": [
        {
            "assetId": "My.Camera",
            "values": [
                {
                    "locales": [
                        "en-US"
                    ],
                    "defaultValue": "camera",
                    "synonyms": []
                }
            ]
        }
    ]
}
```

</p>
</details>

>**Note:** If you need not specify either a default or custom assets path in your configuration, exclude the "assets" node entirely.
 
### Capabilities <a id="capabilities"></a>

`Capabilities` add controls to endpoints and describe how to access them with voice interaction.

#### PowerController

The `Alexa.PowerController` capability interface describes the messages used to control and report the overall power state of a device.

For more details, please refer to the online [PowerController documentation](https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html).

<details><summary>Example PowerController capability definition (click to expand or collapse)</summary>
<p>
 
```json
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

</p>
</details>

#### ToggleController

The `Alexa.ToggleController` capability interface turns components of endpoints on or off. You can include multiple instances of a ToggleController on a single endpoint, as long as they have unique values in the `instance` and `friendlyNames` fields. To control the overall power state of a device, use the `PowerController` capability instead.

* Since there can be several ToggleController capabilities in the same endpoint, the `instance` property uniquely identifies which ToggleController is being targeted in the endpoint.

* The `friendlyNames` element specifies how the ToggleController is named for voice interaction.

For more details, please refer to the online [ToggleController documentation](https://developer.amazon.com/docs/device-apis/alexa-togglecontroller.html).

<details><summary>Example ToggleController capability definition (click to expand or collapse)</summary>
<p>

```json
{
    "type": "AlexaInterface",
    "interface": "Alexa.ToggleController",
    "version": "3",
    "instance": "climate.sync",
    "capabilityResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Setting.ClimateSync"
                }
            }
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
    }
}
```

</p>
</details>

#### RangeController

The `Alexa.RangeController` capability interface models settings of an endpoint that are represented by numbers within a minimum and maximum range. You can include multiple instances of a RangeController on a single endpoint as long as they have unique values in the `instance` and `friendlyNames` fields. The RangeController interface is highly configurable and enables you to model many different kinds of settings for many different kinds of components. 

* Since there can be several RangeControllers in the same endpoint, the `instance` property uniquely identifies which RangeController is being targeted in the endpoint.

* The `friendlyNames` element specifies how the RangeController is named for voice interaction.

* The `configuration` element allows you to specify the range and presets supported by the controller.

* The `supportedRange` element allows you to define the `minimumValue` and `maximumValue` as well  the `precision`. The precision specifies the step between successive allowed values, starting from the `minimumValue`.

* The `presets` element allows you to define the `rangeValue` presets that users can access by voice by using the configured `friendlyNames`.

For more details, please refer to the online [RangeController documentation](https://developer.amazon.com/docs/device-apis/alexa-rangecontroller.html).

<details><summary>Example RangeController capability definition (click to expand or collapse)</summary>
<p>
 
```json
{
    "type": "AlexaInterface",
    "interface": "Alexa.RangeController",
    "version": "3",
    "instance": "speed",
    "capabilityResources": {
    "friendlyNames": [
        {
            "@type": "asset",
            "value": {
                "assetId": "Alexa.Setting.FanSpeed"
            }
        }
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
            "minimumValue": 1,
            "maximumValue": 10,
            "precision": 1
        },
        "presets": [
            {
                "rangeValue": 1,
                "presetResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Value.Low"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Value.Minimum"
                            }
                        }
                    ]
                }
            },
            {
                "rangeValue": 5,
                "presetResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Value.Medium"
                            }
                        }
                    ]
                }
            },
            {
                "rangeValue": 10,
                "presetResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Value.High"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Value.Maximum"
                            }
                        }
                    ]
                }
            }
        ],
        "relationships": {
            "isMemberOf": {
                "zoneId": "zone.all"
            }
        }
    }
}
```

</p>
</details>

#### ModeController

The `Alexa.ModeController` capability interface describes messages used to control the mode settings of an endpoint. You can use the `ModeController` interface to model properties of an endpoint that can be set to one of a list of values, such as the color of a light. The list of values can be ordered, but that's not required. The `Alexa.ModeController` interface is highly configurable and enables you to model many different kinds of settings for many different kinds of devices.

* Since there can be several ModeControllers in the same endpoint, the `instance` property uniquely identifies which ModeController is being targeted in the endpoint.

* The `friendlyNames` element specifies how the ModeController is named for voice interaction.

* The `configuration` element specifies the range and presets supported by the controller.

* Each mode has a `value` identifying it as well as its corresponding `friendlyNames` for voice access.

For more details, please refer to the online [ModeController documentation](https://developer.amazon.com/docs/device-apis/alexa-modecontroller.html).

<details><summary>Example ModeController capability definition (click to expand or collapse)</summary>
<p>

```json
{
    "type": "AlexaInterface",
    "interface": "Alexa.ModeController",
    "version": "3",
    "instance": "mode",
    "capabilityResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Setting.Mode"
                }
            }
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
        "ordered": false,
        "supportedModes": [
            {
                "value": "ECONOMY",
                "modeResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Setting.Economy"
                            }
                        }
                    ]
                }
            },
            {
                "value": "AUTOMATIC",
                "modeResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Setting.Auto"
                            }
                        }
                    ]
                }
            },
            {
                "value": "MANUAL",
                "modeResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Setting.Manual"
                            }
                        }
                    ]
                }
            }
        ]
    }
}
```

</p>
</details>

>**Note:** Set `"proactivelyReported"` and `"retrievable"` to `"false"` in all controller configurations. Neither feature is currently supported in Auto SDK.

### Programmatic Configuration <a id="programmatic-configuration"></a>

You can generate the `EngineConfiguration` programmatically using `aace::carControl::config::CarControlConfiguration`:

```java
// Auto SDK Engine configuration
ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
    // ... other engine configurations ...
));

...

CarControlConfiguration config = CarControlConfiguration.create();
config.addZone("zone.all")
	.addAssetId("Alexa.Location.All")
.createControl("heater", "zone.all")
    .addAssetId("Alexa.DeviceName.Heater")
    .addAssetId("Alexa.DeviceName.Cooler")
    .addPowerController(true)
    .addRangeController("temperature", true, 60, 90, 1, "Alexa.Unit.Temperature.Fahrenheit")
        .addAssetId(CarControlAssets.Setting.TEMPERATURE)
        .addAssetId(CarControlAssets.Setting.HEAT)
        .addPreset(60)
            .addAssetId("Alexa.Value.Low")
            .addAssetId("Alexa.Value.Minimum")
        .addPreset(75)
            .addAssetId("Alexa.Value.Medium")
        .addPreset(90)
            .addAssetId("Alexa.Value.High")
            .addAssetId("Alexa.Value.Maximum");

configuration.add(config)
```

<details><summary>Corresponding configuration generated (click to expand or collapse)</summary>
<p>


```json
{
    "aace.carControl": {
        "assets": {
            "defaultAssetsPath": "/path/to/default/assets.json",
            "customAssetsPath": "/path/to/custom/assets.json"
        },

        "endpoints": [
            {
                "endpointId": "all.heater",
                "endpointResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.DeviceName.Heater"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.DeviceName.Cooler"
                            }
                        }
                    ]
                },
                "capabilities": [
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
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.RangeController",
                        "version": "3",
                        "instance": "temperature",
                        "capabilityResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.Temperature"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.Heat"
                                    }
                                }
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
                            "unitOfMeasure": "Alexa.Unit.Temperature.Fahrenheit",
                            "supportedRange": {
                                "minimumValue": 60,
                                "maximumValue": 90,
                                "precision": 1
                            },
                            "presets": [
                                {
                                    "rangeValue": 60,
                                    "presetResources": {
                                        "friendlyNames": [
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Value.Low"
                                                }
                                            },
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Value.Minimum"
                                                }
                                            }
                                        ]
                                    }
                                },
                                {
                                    "rangeValue": 75,
                                    "presetResources": {
                                        "friendlyNames": [
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Value.Medium"
                                                }
                                            }
                                        ]
                                    }
                                },
                                {
                                    "rangeValue": 90,
                                    "presetResources": {
                                        "friendlyNames": [
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Value.High"
                                                }
                                            },
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Value.Maximum"
                                                }
                                            }
                                        ]
                                    }
                                }
                            ]
                        }
                    }
                ],
                "relationships": {
                    "isMemberOf": {
                        "zoneId": "zone.all"
                    }
                }
            }
        ],
        "zones": [
            {
                "zoneId": "zone.all",
                "zoneResources": {
                    "friendlyNames": [
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Location.All"
                            }
                        }
                    ]
                }
            }
        ]
    }
}
```

</p>
</details>

See more configuration examples in the [Configuration Examples section](#configuration-examples).


## Handling Car Control Functionality <a id="handling-car-control-functionality"></a>

The Auto SDK manages internal state information for Car Control and provides an interface for you to handle the car control commands from the Auto SDK. To implement a custom handler for Car Control, extend the `com.amazon.aace.carControl.CarControl` class.

The methods in the `CarControl` platform interface include an `endpointId` to easily identify the targeted endpoint and, when applicable, the controller with its `controllerId`. These identifiers are the same as you used in the module configuration described above.

The class contains methods to override for the four supported capability interfaces (PowerController, ToggleController, RangeController, and ModeController).

```java
import com.amazon.aace.carControl.CarControl;

public class CarControlHandler extends CarControl {

    // PowerController
    @Override
    public void turnPowerControllerOn(String endpointId) {
        // Turn on the PowerController of the endpoint associated with endpointId
    }
    @Override
    public void turnPowerControllerOff(String endpointId) {
        // Turn off the PowerController of the endpoint associated with endpointId
    }
    @Override
    public boolean isPowerControllerOn(String endpointId) throws Exception {
        // Return the state of the PowerController of the endpoint associated with endpointId
    }

    // ToggleController
    @Override
    public void turnToggleControllerOn(String endpointId, String controllerId) {
        // Turn on the ToggleController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public void turnToggleControllerOff(String endpointId, String controllerId) {
        // Turn off the ToggleController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public boolean isToggleControllerOn(String endpointId, String controllerId) throws Exception {
        // Return the state of the ToggleController associated with controllerId of the endpoint associated with endpointId
    }

    // RangeController
    @Override
    public void setRangeControllerValue(String endpointId, String controllerId, double value) {
        // Set the value of the RangeController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public void adjustRangeControllerValue(String endpointId, String controllerId, double delta) {
        // Adjust the value of the RangeController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public double getRangeControllerValue(String endpointId, String controllerId) throws Exception {
        // Return the value of the RangeController associated with controllerId of the endpoint associated with endpointId
    }

     @Override
    public void setModeControllerValue(String endpointId, String controllerId, String value) {
        // Set the mode of the ModeController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public void adjustModeControllerValue(String endpointId, String controllerId, int delta) {
        // Adjust the mode of the ModeController associated with controllerId of the endpoint associated with endpointId
    }
    @Override
    public String getModeControllerValue(String endpointId, String controllerId) throws Exception {
        // Return the mode of the ModeController associated with controllerId of the endpoint associated with endpointId
    }
};

...

// Register the platform interface implementation with the Engine
mEngine.registerPlatformInterface(new CarControlHandler());
```
## Configuration Examples <a id ="configuration-examples"></a>

This section provides sample configuration snippets (both JSON and the programmatic generation of the JSON) for commonly used endpoints: fan, heater, vent position, and lights.

### Sample Fan Configuration

<details><summary>JSON (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "all.fan",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.Fan"
                }
            }
        ]
    },
    "capabilities": [
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
        },
        {
            "type": "AlexaInterface",
            "interface": "Alexa.RangeController",
            "version": "3",
            "instance": "speed",
            "capabilityResources": {
                "friendlyNames": [
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Setting.FanSpeed"
                        }
                    }
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
                    "minimumValue": 1,
                    "maximumValue": 10,
                    "precision": 1
                },
                "presets": [
                    {
                        "rangeValue": 1,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Low"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Minimum"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "rangeValue": 5,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Medium"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "rangeValue": 10,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.High"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Maximum"
                                    }
                                }
                            ]
                        }
                    }
                ]
            }
        }
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

<details><summary>Programmatic (click to expand or collapse)</summary>
<p>

```java
config->createControl("front.fan", "zone.all")
    .addAssetId("Alexa.DeviceName.Fan")
    .addPowerController(true)
    .addRangeController("speed", true, 0, 10, 1)
        .addAssetId("Alexa.Setting.FanSpeed")
        .addPreset(0)
            .addAssetId("Alexa.Value.Low")
            .addAssetId("Alexa.Value.Minimum")
        .addPreset(5)
            .addAssetId("Alexa.Value.Medium")
        .addPreset(10)
            .addAssetId("Alexa.Value.High")
            .addAssetId("Alexa.Value.Maximum")
```

</p>
</details>

### Sample Heater Configuration

<details><summary>JSON (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "all.heater",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.Heater"
                }
            },
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.Cooler"
                }
            }
        ]
    },
    "capabilities": [
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
        },
        {
            "type": "AlexaInterface",
            "interface": "Alexa.RangeController",
            "version": "3",
            "instance": "temperature",
            "capabilityResources": {
                "friendlyNames": [
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Setting.Temperature"
                        }
                    },
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Setting.Heat"
                        }
                    }
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
                "unitOfMeasure": "Alexa.Unit.Temperature.Fahrenheit",
                "supportedRange": {
                    "minimumValue": 60,
                    "maximumValue": 90,
                    "precision": 2
                },
                "presets": [
                    {
                        "rangeValue": 60,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Low"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Minimum"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "rangeValue": 76,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Medium"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "rangeValue": 90,
                        "presetResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.High"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Maximum"
                                    }
                                }
                            ]
                        }
                    }
                ]
            }
        }
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

<details><summary>Programmatic (click to expand or collapse)</summary>
<p>

```java
config.createControl("all.heater", "zone.all")
    .addAssetId("Alexa.DeviceName.Heater")
    .addAssetId("Alexa.DeviceName.Cooler")
    .addPowerController(true)
    .addRangeController("temperature", true, 60, 90, 1, "Alexa.Unit.Temperature.Fahrenheit")
        .addAssetId("Alexa.Setting.Temperature")
        .addAssetId("Alexa.Setting.Heat")
        .addPreset(60)
            .addAssetId("Alexa.Value.Low")
            .addAssetId("Alexa.Value.Minimum")
        .addPreset(75)
            .addAssetId("Alexa.Value.Medium")
        .addPreset(90)
            .addAssetId("Alexa.Value.High")
            .addAssetId("Alexa.Value.Maximum")
```

</p>
</details>

### Sample Vent Configuration

<details><summary>JSON (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "vent",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.Vent"
                }
            }
        ]
    },
    "capabilities": [
        {
            "type": "AlexaInterface",
            "interface": "Alexa.PowerController",
            "version": "3",
            "properties": {
                "proactivelyReported": false,
                "retrievable": false,
                "supported": [
                    {
                        "name": "powerState"
                    }
                ]
            }
        },
        {
            "type": "AlexaInterface",
            "interface": "Alexa.ModeController",
            "version": "3",
            "instance": "position",
            "capabilityResources": {
                "friendlyNames": [
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Setting.Position"
                        }
                    }
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
                "ordered": false,
                "supportedModes": [
                    {
                        "value": "BODY",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.BodyVents"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "FLOOR",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.FloorVents"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "WINDSHIELD",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.WindshieldVents"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "MIX",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Setting.MixVents"
                                    }
                                }
                            ]
                        }
                    }
                ]
            }
        }
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

<details><summary>Programmatic (click to expand or collapse)</summary>
<p>

```java
config.createControl("vent", "zone.all")
    .addAssetId("Alexa.DeviceName.Vent")
    .addPowerController(false)
    .addModeController("position", false, false)
        .addAssetId("Alexa.Setting.Position")
        .addValue("BODY")
            .addAssetId("Alexa.Setting.BodyVents")
        .addValue("FLOOR")
            .addAssetId("Alexa.Setting.FloorVents")
        .addValue("WINDSHIELD")
            .addAssetId("Alexa.Setting.WindshieldVents")
        .addValue("MIX")
            .addAssetId("Alexa.Setting.MixVents")
```

</p>
</details>

### Sample Simple Light Configuration

<details><summary>JSON (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "dome.light",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.DomeLight"
                }
            },
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.CabinLight"
                }
            }
        ]
    },
    "capabilities": [
        {
            "type": "AlexaInterface",
            "interface": "Alexa.PowerController",
            "version": "3",
            "properties": {
                "proactivelyReported": false,
                "retrievable": false,
                "supported": [
                    {
                        "name": "powerState"
                    }
                ]
            }
        }
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

<details><summary>Programmatic (click to expand or collapse)</summary>
<p>

```java
config.createControl("dome.light", "zone.all")
    .addAssetId("Alexa.DeviceName.DomeLight")
    .addAssetId("Alexa.DeviceName.CabinLight")
    .addPowerController(false)
```

</p>
</details>

### Sample Colored Ambient Light Configuration

<details><summary>JSON (click to expand or collapse)</summary>
<p>

```json
{
    "endpointId": "ambient.light",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.DeviceName.AmbientLight"
                }
            }
        ]
    },
    "capabilities": [
        {
            "type": "AlexaInterface",
            "interface": "Alexa.PowerController",
            "version": "3",
            "properties": {
                "proactivelyReported": false,
                "retrievable": false,
                "supported": [
                    {
                        "name": "powerState"
                    }
                ]
            }
        },
        {
            "type": "AlexaInterface",
            "interface": "Alexa.ModeController",
            "version": "3",
            "instance": "color",
            "capabilityResources": {
                "friendlyNames": [
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Setting.Color"
                        }
                    }
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
                "supportedModes": [
                    {
                        "value": "RED",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Red"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "BLUE",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Blue"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "GREEN",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Green"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "WHITE",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.White"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "ORANGE",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Orange"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "YELLOW",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Yellow"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "INDIGO",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Indigo"
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "value": "VIOLET",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Value.Violet"
                                    }
                                }
                            ]
                        }
                    }
                ]
            }
        }
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}
```

</p>
</details>

<details><summary>Programmatic (click to expand or collapse)</summary>
<p>

```java
config.createControl("ambient.light", "zone.all")
    .addAssetId("Alexa.DeviceName.AmbientLight")
    .addPowerController(true)
    .addModeController("color", false, true)
        .addAssetId("Alexa.Setting.Color")
        .addValue("RED")
            .addAssetId("Alexa.Value.Red")
        .addValue("BLUE")
            .addAssetId("Alexa.Value.Blue")
        .addValue("GREEN")
            .addAssetId("Alexa.Value.Green")
        .addValue("WHITE")
            .addAssetId("Alexa.Value.White")
        .addValue("ORANGE")
            .addAssetId("Alexa.Value.Orange")
        .addValue("YELLOW")
            .addAssetId("Alexa.Value.Yellow")
        .addValue("INDIGO")
            .addAssetId("Alexa.Value.Indigo")
        .addValue("VIOLET")
            .addAssetId("Alexa.Value.Violet")
```

</p>
</details>