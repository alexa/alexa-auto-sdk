# Alexa Auto APL Renderer

The Alexa Auto APL Renderer library enables the Alexa Auto App to render Amazon Presentation Language (APL) documents on the user's device. The library consists of the following components:

* `APLReceiver`: This class receives the APL intents. After receiving an APL `RenderDocument` intent, it starts adding `APLFragment` to [VoiceActivity](https://github.com/alexa/alexa-auto-sdk/blob/master/aacs/android/app-components/alexa-auto-voice-ui/src/main/java/com/amazon/alexa/auto/voice/ui/VoiceActivity.java) or sends the `APLDirective` event to `APLFragment`.

* `APLHandler`: This class initializes `APLPresenter`, set `APLLayout` in `APLPresenter` when the layout is inflated. This class also handles APL intents, such as those for rendering or clearing APL documents. It also executes APL commands and implements an event interface named `IAPLEventSender`, which reports events to Alexa or the capability agent.

* `APLFragment`: This class inflates the APL layout to render the APL document. The `APLLayout` object in [fragment_apl.xml](https://github.com/alexa/alexa-auto-sdk/blob/master/aacs/android/app-components/alexa-auto-apl-renderer/src/main/res/layout/fragment_apl.xml) defines the layout. This class also calls the `APLhandler` methods to handle APL intents.

## Prerequisites for Using the Alexa Auto APL Renderer Library
The APL Renderer library for the Alexa Auto App depends on the capabilities provided by an Auto SDK module called APL Render module. For example, the APL Render module provides the `APLPresenter` class implementation. The Alexa Auto APL Renderer library initializes this class to provide the orchestration logic in the APL rendering process.

For information about how to build the APL Render module, see the [APL Render documentation](./modules/apl-render/README.md).

## Using Alexa Auto APL Renderer Library with Alexa Auto App
To use the Alexa Auto APL Renderer Library with the Alexa Auto App, include the appropriate build dependency, fonts package and configure APL in the Alexa Auto App.

### Including Build Dependency (AAR)

The Alexa Auto APL Renderer library requires a prebuilt Android view host, which is available as an AAR on the developer portal. To download the AAR, contact your Solutions Architect (SA) or Partner Manager.

>**Note:** To include the build dependency, you must place the Android view host AAR in the [modules/apl-render/src/main/libs/](./modules/apl-render/src/main/libs) folder of the APL Render module.

```
apl-render
└── src
    └── main
        └── libs
            ├── aplRelease.aar
            └── ttsplayerRelease.aar
```

### Including Fonts Package

The Alexa Auto APL Renderer library requires a fonts package, which is available as a zip file on the developer portal. To download the fonts package, contact your Solutions Architect (SA) or Partner Manager.

>**Note:** The fonts package must be unzipped under the [modules/apl-render/src/main](./modules/apl-render/src/main) folder. For example, `unzip fonts.zip -d modules/apl-render/src/main` within the `alexa-auto-apl-render` app component.

```
apl-render
└── src
    └── main
        └── res
            └── font
                ├── bookerly.xml
                ├── bookerly_lcd_bd.ttf
                ├── bookerly_lcd_bd_it.ttf
                ├── bookerly_lcd_it.ttf
                ├── bookerly_lcd_lt.ttf
                ├── bookerly_lcd_lt_it.ttf
                └── bookerly_lcd_rg.ttf
```
### Configuring APL in Alexa Auto App

The Alexa Auto App uses `aacs_config.json` for configuring the Auto SDK. Follow these steps to enable APL and specify the display format:

1. Enable APL under the `aacs.apl` configuration node as follows:

    ```json
    "aacs.modules": {
        "aacs.apl": {
            "APL": {
                "enabled": true
            }
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
                            "uiMode": "AUTO",
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
                                      "width": 900,
                                      "height": 1200
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
                    }
                  ]
            }
        }
    }
    ```
    
For descriptions of the visual characteristic parameters, see the [Alexa Smart Screen SDK documentation](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/config/SmartScreenSDKConfig.md#visual-characteristics-parameters).

#### APL Viewport

The APL viewport refers to the UI screen area where APL will be rendered. The width and height pixel values defined in the `Alexa.Display.Window` configuration define the dimensions APL viewport. That configuration is also uploaded to the cloud so that Alexa skills have knowledge of the APL viewport dimensions. Make sure that the configuration is as accurate as possible in order to provide the best APL experience.

The table below lists the supported APL viewport sizes. Your configuation must define values that fall within the
range of values listed on the table.

|Profile         |type    |shape              |mode |minWidth (dp)|maxWidth (dp)|minHeight (dp)|maxHeight (dp)|
|----------------|--------|-------------------|-----|-------------|-------------|--------------|--------------|
|Auto Extra Small|standard| overlay/rectangle |auto |	400 |	599  |	480 |	1279|
|Auto Small      |standard| overlay/rectangle |auto |	600 |	959  |	480 |	1279|
|Auto Medium     |standard| overlay/rectangle |auto |	960 | 1279 |	480 |	1279|

>**Note:** APL viewports are defined using dependent pixels (`dp`) but the `Alexa.Display.Window` dimensions must be specified in pixels (`px`). To convert from `dp` to `px` use the formula `px = (dpi/160) * dp`. For example, to define an Auto Medium APL viewport with dimensions 1200 (width `dp`) x 960 (height `dp`) on a device with 240 screen dpi, the values would map to `width (px) = (240/160) * 1200 = 1800` and `height (px) = (240/160) * 960 = 1440`.

### Using Alexa Auto App

After the gradle build command finishes building the Alexa Auto App, you can test the sample app by asking, "Alexa, tell me a joke." An APL document is rendered on the device.

The Auto SDK 4.0 enables the ability to report the vehicle driving state to provide safer visual experiences while the vehicles is moving. To enable the driving state support, add `-PenabledUXRestrictions` to the build command. When you say "Alexa, show coffee shops near me" and view the details for a point of interest, the data displayed in the APL detail card will contain more information while the driving state is `parked`. Additionally, the Auto SDK 4.0 supports the ability to report the light conditions around the vehicle to support day/night mode and provide a custom theme id to alter the look and feel of the APL experience. 

>**Note:** The alexa-auto-ux-restrictions requires Android API level 29. Provide your own implementation for the `CarUxRestrictionsController` interface if your device uses API level less than 29. By default, the driving state will always be set to `moving` if the `CarUxRestrictionsController` is not implemented.

## Known Issues
* When interrupting music playback with APL utterance, APL card will be dismissed when music playback resumes, this issue could not be seen if music ducking support is enabled in Alexa Auto App. To do this, add audio ducking node in `aacs.alexa`, as shown in the following example:
   ```
   "aacs.alexa": {
       "audio": {
           "audioOutputType.music": {
               "ducking":{
                   "enabled": true
               }
           }
       }
   }
   ```
