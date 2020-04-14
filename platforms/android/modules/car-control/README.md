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
                    "assetId": "Alexa.Automotive.DeviceName.Fan"
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
                    "assetId": "Alexa.Automotive.Location.All"
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
            "assetId": "Alexa.Automotive.DeviceName.Fan",
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

A set of default assets is defined in the Auto SDK Engine. This set of assets covers a wide range of use cases required to describe the controllable endpoints of a vehicle. The names of the predefined assetIds are defined as [constants](./src/main/java/com/amazon/aace/carControl/CarControlAssets.java) in the com.amazon.aace.carControl package for easy use. 

<details><summary>Predefined assetIds (click to expand or collapse)</summary>
<p>

```
    "Alexa.Automotive.DeviceName.AirConditioner"
    "Alexa.Automotive.DeviceName.AmbientLight"
    "Alexa.Automotive.DeviceName.CabinLight"
    "Alexa.Automotive.DeviceName.Car"
    "Alexa.Automotive.DeviceName.ClimateControl"
    "Alexa.Automotive.DeviceName.Cooler"
    "Alexa.Automotive.DeviceName.DomeLight"
    "Alexa.Automotive.DeviceName.Fan"
    "Alexa.Automotive.DeviceName.Heater"
    "Alexa.Automotive.DeviceName.Light"
    "Alexa.Automotive.DeviceName.ReadingLight"
    "Alexa.Automotive.DeviceName.TrunkLight"
    "Alexa.Automotive.DeviceName.Vent"
    "Alexa.Automotive.DeviceName.Window"
    "Alexa.Automotive.DeviceName.Windshield"
    "Alexa.Automotive.Location.All"
    "Alexa.Automotive.Location.Driver"
    "Alexa.Automotive.Location.DriverRow"
    "Alexa.Automotive.Location.FirstRow"
    "Alexa.Automotive.Location.FourthRow"
    "Alexa.Automotive.Location.Front"
    "Alexa.Automotive.Location.Passenger"
    "Alexa.Automotive.Location.PassengerRow"
    "Alexa.Automotive.Location.Rear"
    "Alexa.Automotive.Location.RearDriver"
    "Alexa.Automotive.Location.RearPassenger"
    "Alexa.Automotive.Location.SecondRow"
    "Alexa.Automotive.Location.ThirdRow"
    "Alexa.Automotive.Setting.AirRecirculation"
    "Alexa.Automotive.Setting.Auto"
    "Alexa.Automotive.Setting.BodyVents"
    "Alexa.Automotive.Setting.ClimateSync"
    "Alexa.Automotive.Setting.Color"
    "Alexa.Automotive.Setting.Defog"
    "Alexa.Automotive.Setting.Defrost"
    "Alexa.Automotive.Setting.Economy"
    "Alexa.Automotive.Setting.FanSpeed"
    "Alexa.Automotive.Setting.FloorVents"
    "Alexa.Automotive.Setting.Heat"
    "Alexa.Automotive.Setting.Intensity"
    "Alexa.Automotive.Setting.Manual"
    "Alexa.Automotive.Setting.MixVents"
    "Alexa.Automotive.Setting.Mode"
    "Alexa.Automotive.Setting.Position"
    "Alexa.Automotive.Setting.Temperature"
    "Alexa.Automotive.Setting.WindshieldVents"
    "Alexa.Automotive.Unit.Temperature.Celsius"
    "Alexa.Automotive.Unit.Temperature.Fahrenheit"
    "Alexa.Automotive.Value.Blue"
    "Alexa.Automotive.Value.Green"
    "Alexa.Automotive.Value.Indigo"
    "Alexa.Automotive.Value.Orange"
    "Alexa.Automotive.Value.Red"
    "Alexa.Automotive.Value.Violet"
    "Alexa.Automotive.Value.White"
    "Alexa.Automotive.Value.Yellow"
    "Alexa.Automotive.Value.High"
    "Alexa.Automotive.Value.Low"
    "Alexa.Automotive.Value.Maximum"
    "Alexa.Automotive.Value.Medium"
    "Alexa.Automotive.Value.Minimum"
```
</p>
</details>
<br>

>**Note:** Should you need to change a default asset value, you can override the assets in the Engine by providing a path to a file that defines them. The [default assets file](./assets/assets-1P.json) is provided for you to copy, modify, and install in your chosen location. Provide the path to this file in the optional `defaultAssetsPath` field in the `assets` node of the car control configuration. Typically this configuration override is not necessary, but the Auto SDK provides the option, if needed. This override applies only to online-only systems without the optional LVC extension.

You can add custom complementary assets alongside the defaults if an endpoint in your vehicle cannot be described using one of the predefined assetIds. To do this, define your custom assets in a single JSON file, ensuring that every `assetId` uses the prefix `"My.”`. Specify the path to that JSON file in the optional `customAssetsPath` field in the `assets` node of the configuration. The format of this file is the same as the format for the [default assets](./assets/assets-1P.json).

>**Note for hybrid systems with LVC:** The file at the path you provide in the `assets.customAssetsPath` field of the Auto SDK car control configuration must be the same as the custom assets file you configure for your `ILVCClient` using the LVC APK.

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

>**Note:** If you don't need to specify either a default or a custom assets path in your configuration, exclude the "assets" node of configuration entirely.
 
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
                    "assetId": "Alexa.Automotive.Setting.ClimateSync"
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
                "assetId": "Alexa.Automotive.Setting.FanSpeed"
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
                                "assetId": "Alexa.Automotive.Value.Low"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Automotive.Value.Minimum"
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
                                "assetId": "Alexa.Automotive.Value.Medium"
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
                                "assetId": "Alexa.Automotive.Value.High"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Automotive.Value.Maximum"
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
                    "assetId": "Alexa.Automotive.Setting.Mode"
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
                                "assetId": "Alexa.Automotive.Setting.Economy"
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
                                "assetId": "Alexa.Automotive.Setting.Auto"
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
                                "assetId": "Alexa.Automotive.Setting.Manual"
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
// Point to your custom assets definition, if your config requires it, else exclude this
config.addCustomAssetsPath("/path/to/custom/assets.json")

    .createControl("heater", CarControlConfiguration.Zone.ALL)
        .addAssetId(CarControlAssets.Device.HEATER)
        .addAssetId(CarControlAssets.Device.COOLER)
        .addPowerController(true)
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

configuration.add(config)
```

<details><summary>Corresponding configuration generated (click to expand or collapse)</summary>
<p>


```json
{
    "aace.carControl": {
        "assets": {
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
                                "assetId": "Alexa.Automotive.DeviceName.Heater"
                            }
                        },
                        {
                            "@type": "asset",
                            "value": {
                                "assetId": "Alexa.Automotive.DeviceName.Cooler"
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
                                        "assetId": "Alexa.Automotive.Setting.Temperature"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Setting.Heat"
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
                                                    "assetId": "Alexa.Automotive.Value.Low"
                                                }
                                            },
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Automotive.Value.Minimum"
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
                                                    "assetId": "Alexa.Automotive.Value.Medium"
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
                                                    "assetId": "Alexa.Automotive.Value.High"
                                                }
                                            },
                                            {
                                                "@type": "asset",
                                                "value": {
                                                    "assetId": "Alexa.Automotive.Value.Maximum"
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
                                "assetId": "Alexa.Automotive.Location.All"
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
                    "assetId": "Alexa.Automotive.DeviceName.Fan"
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
                            "assetId": "Alexa.Automotive.Setting.FanSpeed"
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
                                        "assetId": "Alexa.Automotive.Value.Low"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Value.Minimum"
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
                                        "assetId": "Alexa.Automotive.Value.Medium"
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
                                        "assetId": "Alexa.Automotive.Value.High"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Value.Maximum"
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
config->createControl("all.fan", CarControlConfiguration.Zone.ALL)
    .addAssetId(CarControlAssets.Device.FAN)
    .addPowerController(true)
    .addRangeController("speed", false, 0, 10, 1)
        .addAssetId(CarControlAssets.Setting.FAN_SPEED)
        .addPreset(0)
            .addAssetId(CarControlAssets.Value.LOW)
            .addAssetId(CarControlAssets.Value.MINIMUM)
        .addPreset(5)
            .addAssetId(CarControlAssets.Value.MEDIUM)
        .addPreset(10)
            .addAssetId(CarControlAssets.Value.HIGH)
            .addAssetId(CarControlAssets.Value.MAXIMUM)
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
                    "assetId": "Alexa.Automotive.DeviceName.Heater"
                }
            },
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.DeviceName.Cooler"
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
                            "assetId": "Alexa.Automotive.Setting.Temperature"
                        }
                    },
                    {
                        "@type": "asset",
                        "value": {
                            "assetId": "Alexa.Automotive.Setting.Heat"
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
                                        "assetId": "Alexa.Automotive.Value.Low"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Value.Minimum"
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
                                        "assetId": "Alexa.Automotive.Value.Medium"
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
                                        "assetId": "Alexa.Automotive.Value.High"
                                    }
                                },
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Value.Maximum"
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
config.createControl("all.heater", CarControlConfiguration.Zone.ALL)
    .addAssetId(CarControlAssets.Device.HEATER)
    .addAssetId(CarControlAssets.Device.COOLER)
    .addPowerController(true)
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
            .addAssetId(CarControlAssets.Value.Maximum)
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
                    "assetId": "Alexa.Automotive.DeviceName.Vent"
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
                            "assetId": "Alexa.Automotive.Setting.Position"
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
                                        "assetId": "Alexa.Automotive.Setting.BodyVents"
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
                                        "assetId": "Alexa.Automotive.Setting.FloorVents"
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
                                        "assetId": "Alexa.Automotive.Setting.WindshieldVents"
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
                                        "assetId": "Alexa.Automotive.Setting.MixVents"
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
config.createControl("vent", CarControlConfiguration.Zone.ALL)
        .addAssetId(CarControlAssets.Device.VENT)
        .addPowerController(true)
        .addModeController("position", true, true)
            .addAssetId(CarControlAssets.Setting.POSITION)
            .addValue(VentPosition.BODY)
                .addAssetId(CarControlAssets.Setting.BODY_VENTS)
            .addValue(VentPosition.FLOOR)
                .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
            .addValue(VentPosition.WINDSHIELD)
                .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
            .addValue(VentPosition.MIX)
                .addAssetId(CarControlAssets.Setting.MIX_VENTS);
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
                    "assetId": "Alexa.Automotive.DeviceName.DomeLight"
                }
            },
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.DeviceName.CabinLight"
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
config.createControl("dome.light", CarControlConfiguration.Zone.ALL)
        .addAssetId(CarControlAssets.Device.DOME_LIGHT)
        .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
        .addPowerController(true);
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
                    "assetId": "Alexa.Automotive.DeviceName.AmbientLight"
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
                            "assetId": "Alexa.Automotive.Setting.Color"
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
                        "value": "RED",
                        "modeResources": {
                            "friendlyNames": [
                                {
                                    "@type": "asset",
                                    "value": {
                                        "assetId": "Alexa.Automotive.Value.Red"
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
                                        "assetId": "Alexa.Automotive.Value.Blue"
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
                                        "assetId": "Alexa.Automotive.Value.Green"
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
                                        "assetId": "Alexa.Automotive.Value.White"
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
                                        "assetId": "Alexa.Automotive.Value.Orange"
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
                                        "assetId": "Alexa.Automotive.Value.Yellow"
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
                                        "assetId": "Alexa.Automotive.Value.Indigo"
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
                                        "assetId": "Alexa.Automotive.Value.Violet"
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
config.createControl("ambient.light", CarControlConfiguration.Zone.ALL)
        .addAssetId(CarControlAssets.Device.AMBIENT_LIGHT)
        .addPowerController(true)
        .addModeController("color", false, false)
            .addAssetId(CarControlAssets.Setting.COLOR)
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
```

</p>
</details>