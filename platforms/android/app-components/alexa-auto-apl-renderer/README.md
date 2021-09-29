# Alexa Auto APL Renderer

The Alexa Auto APL Renderer library enables the AACS Sample App to render Amazon Presentation Language (APL) documents on the user's device. The library consists of the following components:

* `APLReceiver`: This class receives the APL intents. After receiving an APL `RenderDocument` intent, it starts the `APLActivity` class or sends the `APLDirective` event to `APLActivity`.

* `APLHandler`: This class handles APL intents, such as those for rendering or clearing APL documents. It also executes APL commands and implements an event interface named `IAPLEventSender`, which reports events to Alexa or the capability agent.

* `APLActivity`: This class initializes the APL runtime, instantiates `APLPresenter`, and calls the `APLhandler` methods to handle APL intents. This class also inflates the APL layout to render the APL document. The `APLLayout` object in [./src/main/res/layout/apl_view.xml](./src/main/res/layout/apl_view.xml) defines the layout.

**Important!** The Alexa Auto APL Renderer library is for you to experiment with APL document rendering on an automotive device, it is not a preferred UX in automotive experience. Do not use the library to render APL documents in a production vehicle. Support for APL rendering in a production environment will be provided in a future Alexa Auto SDK version.

**Important!** Certain APL templates recommend scrolling text and these will be removed for production versions of APL in Auto SDK 4.0. 

## Prerequisites for Using the Alexa Auto APL Renderer Library
The APL Renderer library for the AACS Sample App depends on the capabilities provided by an Auto SDK module called APL Render module. For example, the APL Render module provides the `APLPresenter` class implementation. The Alexa Auto APL Renderer library initializes this class to provide the orchestration logic in the APL rendering process.

For information about how to build the APL Render module, see the [APL Render README](../../modules/apl-render/README.md). After building the APL Render module, you must place the APL Render AAR (Android Archive) in the [libs/](./libs) folder.

## Using Alexa Auto APL Renderer Library with AACS Sample App
To use the Alexa Auto APL Renderer Library with the AACS Sample App, include the appropriate build dependency and configure APL in the AACS Sample App.

### Including Build Dependency (AAR)

The Alexa Auto APL Renderer library requires a prebuilt Android view host, which is available as an AAR on the developer portal. To download the AAR, contact your Solutions Architect (SA) or Partner Manager.

>**Note:** To include the build dependency, you must place the Android view host AAR in the [libs/](./libs) folder.

### Configuring APL in AACS Sample App

The AACS Sample App passes `aacs_config.json` to AACS for configuring the Auto SDK. Follow these steps to enable APL and specify the display format:

1. Enable APL in `aacs_config.json`:

   ```json
   "aasb.apl": {
       "APL": {
         "enabled" : true
       }
     }
   ```

2. Add the `gui` configuration node in `aacs.alexa`, as shown in the following example:

   ```json
   {
       "aacs.alexa": {
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
   ```

   For descriptions of the visual characteristic parameters, see the [Alexa Smart Screen SDK documentation](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/config/SmartScreenSDKConfig.md#visual-characteristics-parameters).

### Building AACS Sample App with Alexa Auto APL Renderer Library

To build the AACS Sample App with Alexa Auto APL Renderer library, go to `${AAC_SDK_HOME}/samples/android-aacs-sample-app/` and enter the following command:

```shell
gradle assembleRelease -PenabledAPL
```

### Using AACS Sample App

After the gradle build command finishes building the AACS Sample App, you can test the sample app by asking, "Alexa, tell me a joke." An APL document is rendered on the device.