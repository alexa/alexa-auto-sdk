# Alexa Presentation Language (APL) Module

The Alexa Auto SDK Alexa Presentation Language (APL) module provides the interfaces required by your platform implementation to interface with the APL capabilities of Alexa.

<!-- omit in toc -->
## Table of Contents

- [Overview](#overview)
- [Building the APL Module](#building-the-apl-module)
- [Configuring the Engine](#configuring-the-engine)
- [Registering an APL Handler](#registering-an-apl-handler)
- [Visual Characteristics](#visual-characteristics)
- [APL Rendering for Android](#apl-rendering-for-android)


## Overview 

To handle [Alexa Presentation Language directives](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html) from Alexa Voice Service (AVS), extend the APL module. These directives contain metadata needed to render Alexa's visual responses for devices with a graphical user interface (GUI). Additionally, the APL module provides interfaces for sending events, providing document and window state, and more. For more information about APL, see the [APL documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html).

>**Note:** The APL module doesn't render APL documents; it provides the conduit to render APL documents and process user events from the GUI or voice user interface (VUI). The functionality for rendering APL documents is provided by the [APL Render module](#apl-rendering-for-android).

## Building the APL Module

The APL module depends on the [Alexa Smart Screen SDK](https://github.com/alexa/alexa-smart-screen-sdk) which is available on GitHub as open source software from Amazon. The [Alexa Auto SDK Builder](../../../../builder/README.md) automatically builds the APL module for the Alexa Auto SDK, along with necessary components of the Alexa Smart Screen SDK (downloaded as part of the build process), for the specified target.

## Configuring the Engine
The APL module can be optionally configured with the following Engine setting:

```
{
  "alexaPresentationCapabilityAgent": {
     "displayDocumentInteractionIdleTimeout": <TIMEOUT_IN_MS>
  }
}
```
You can also generate the configuration programmatically by using the `com.amazon.aace.apl.confi.APLConfiguration.createAlexaPresentationTimeoutConfig()` factory method.

>**Note:** The default value for the configuration timeout is 30 seconds.

## Registering an APL Handler

To implement a custom handler for APL, extend the `aace::apl::APL` class.

```java
public class APLHandler extends APL {

    public APLHandler() {
        ...
    }

    @Override
    public void renderDocument(String jsonPayload, String token, String windowId) {
        ...
    }

    @Override
    public void clearDocument(String token) {
        ...
    }

    @Override
    public void executeCommands(String payload, String token) {
        ...
    }

    @Override
    public void interruptCommandSequence(String token) {
        ...
    }

    @Override
    void dataSourceUpdate(String sourceType, String jsonPayload, String token) {
        ...
    }
};
```

**renderDocument( jsonPayload, token, windowId )**

When `renderDocument` is called, the platform implementation renders the document based on the APL specification in the `jsonPayload` argument, then calls `renderDocumentResult` to indicate success or failure (along with an error message). The `token` and `windowId` arguments are used by the rendering engine to manage the APL presentation. The `windowId` may be empty or null, in which the platform implementation should render on the default window. See the [RenderDocument Directive](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#renderdocument-directive) for more information.

>**Important!:** The payload may contain customer sensitive information and should be used with utmost care. Failure to do so may result in exposing or mishandling of customer data.

**clearDocument( token )**

When `clearDocument` is called, the platform implementation clears the rendered document and calls `clearCard` to indicate completion.

**executeCommands( jsonPayload, token )**

When `executeCommands` is called, the platform implementation executes the commands based on the APL specification in the payload and calls `executeCommandsResult` to indicate success or failure (along with an error message). The `token` argument is used by the rendering engine to manage the APL presentation. See the [ExecuteCommands Directive](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#executecommands-directive) for more information.

**interruptCommandSequence( token )**

When `interruptCommandSequence` is called, the platform implementation should interrupt the execution sequence of the current command. This will be called if an `ActivityEvent::INTERRUPT` event is reported by the platform implementation.

**dataSourceUpdate( sourceType, jsonPayload, token )**

When `dataSourceUpdate` is called, the platform implementation provides the received data to the view host. This is typically a response from Alexa to a `sendDataSourceFetchRequestEvent` event. The `token` argument is used by the rendering engine to manage the APL presentation. See the [Send Index List Data](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#sendindexlistdata-directive), [Send Token List Data](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#sendtokenlistdata-directive), and [Update Index List Data](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-interface.html#updateindexlistdata-directive) directives for more information. For information on data sources, see the [APL data source documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html).

## Visual Characteristics

The APL module requires that the platform implementation define the visual characteristics of the device. Visual characteristics are passed directly to the Smart Screen SDK, and therefore have the format described in the [Alexa Smart Screen SDK documentation](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/config/SmartScreenSDKConfig.md#visual-characteristics-parameters).

Include the `visualCharacteristics` configuration in the JSON object `aace.alexa/avsDeviceSDK/gui` as shown in the following example. You can pass the configuration to the Engine using a `StreamConfiguration` or `ConfigurationFile` object.

```json
{
    "aace.alexa": {
        "avsDeviceSDK": {
            "gui": {
                "visualCharacteristics": [
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.InteractionMode",
                        "version": "1.1",
                        "configurations": {
                            "interactionModes": [
                                {
                                    "id": "apl-interaction-id",
                                    "uiMode": "HUB",
                                    "interactionDistance": {
                                        "unit": "INCHES",
                                        "value": 24
                                    },
                                    "touch": "SUPPORTED",
                                    "keyboard": "SUPPORTED",
                                    "video": "SUPPORTED",
                                    "dialog": "SUPPORTED"
                                }
                            ]
                        }
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.Presentation.APL.Video",
                        "version": "1.0",
                        "configurations": {
                            "video": {
                                "codecs": [
                                    "H_264_42",
                                    "H_264_41"
                                ]
                            }
                        }
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.Display.Window",
                        "version": "1.0",
                        "configurations": {
                            "templates": [
                                {
                                    "id": "apl-window-id",
                                    "type": "STANDARD",
                                    "configuration": {
                                        "sizes": [
                                            {
                                                "type": "DISCRETE",
                                                "id": "window-size-id",
                                                "value": {
                                                    "unit": "PIXEL",
                                                    "value": {
                                                        "width": 1280,
                                                        "height": 720
                                                    }
                                                }
                                            }
                                        ],
                                        "interactionModes": [
                                            "apl-interaction-id"
                                        ]
                                    }
                                }
                            ]
                        }
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.Display",
                        "version": "1.0",
                        "configurations": {
                            "display": {
                                "type": "PIXEL",
                                "touch": [
                                    "UNSUPPORTED"
                                ],
                                "shape": "RECTANGLE",
                                "dimensions": {
                                    "resolution": {
                                        "unit": "PIXEL",
                                        "value": {
                                            "width": 2048,
                                            "height": 1536
                                        }
                                    },
                                    "physicalSize": {
                                        "unit": "INCHES",
                                        "value": {
                                            "width": 8.9,
                                            "height": 6.05
                                        }
                                    },
                                    "pixelDensity": {
                                        "unit": "DPI",
                                        "value": 288
                                    },
                                    "densityIndependentResolution": {
                                        "unit": "DP",
                                        "value": {
                                            "width": 2048,
                                            "height": 1536
                                        }
                                    }
                                }
                            }
                        }
                    }
                ]
            }
        }
    }
}
```

## APL Rendering for Android

The Auto SDK provides the APL Render module, which is an Android library for rendering APL documents in an Android application. For information about integrating the APL Render module into your product, see the [APL Render module README](../apl-render/README.md).
