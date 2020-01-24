# Automotive Grade Linux Alexa Voice Agent

The Automotive Grade Linux (AGL) Alexa Voice Agent is Alexa client software that is plugged into the AGL speech framework to voice-enable applications, as described in the speech [architecture](https://confluence.automotivelinux.org/display/SPE/Speech+EG+Architecture).

**Table of Contents**

* [Overview](#overview)
* [AGL Alexa Voice Agent Architecture](#agl-alexa-voice-agent-architecture)
* [Setting up the AGL Alexa Voice Agent](#setting-up-the-agl-alexa-voice-agent)
* [Configuring the AGL Alexa Voice Agent](#configuring-the-agl-alexa-voice-agent)
* [Building the AGL Voice Agent](#building-the-agl-alexa-voice-agent)
* [Installing and Running the AGL Alexa Voice Agent](#installing-and-running-the-agl-alexa-voice-agent)
* [Authorizing with the Alexa Voice Service (AVS)](#authorizing-with-the-alexa-voice-service-avs)
* [Release Notes](#v210-release-notes)

## Overview <a id ="overview"></a>

The AGL Alexa Voice Agent is implemented as a standard AGL [binding](https://docs.automotivelinux.org/docs/en/master/apis_services/reference/af-main/1-afm-daemons.html) that exposes APIs for:

* Speech recognition start and cancel
* Subscription to events containing Alexa's dialog, authentication, and connection states
* Subscription to events containing Alexa's capability messages or directives
* User authentication and authorization using Amazon's [Login with Amazon (LWA) Code-Based Linking](https://developer.amazon.com/docs/login-with-amazon/minitoc-lwa-other-devices.html)

The [Alexa Auto SDK](../../../OVERVIEW.md) is the underlying technology that powers the speech recognition capabilities of this binding.

>**License Information:** The [Google Test v1.8.0](https://github.com/google/googletest) dependencies are fetched and run by the build system when the AGL Alexa Voice Agent is compiled with the `ENABLE_AASB_UNIT_TESTS` option.

## AGL Alexa Voice Agent Architecture <a id = "agl-alexa-voice-agent-architecture"></a>

The following diagram illustrates the high-level architecture of the AGL Alexa Voice Agent. Key components include the [binding controller](#binding-controller), the [Auto SDK Platform Handlers](#auto-sdk-platform-handlers), and the [Reference Audio I/O Implementation](#reference-audio-io-implementation).

![architecture](./assets/architecture.png)

### Binding Controller <a id ="binding-controller"></a>
The [Binding controller](./src/plugins/AlexaVoiceagentApi.cpp) is the entry point of the binding. It does initialization and dependency injection and also handles the requests for methods/verbs exposed by the binding.

### Auto SDK Platform Handlers <a id = "auto-sdk-platform-handlers"></a>
The Auto SDK platform handlers host the C++ platform API implementation handlers of the Alexa Auto SDK. Audio-related APIs such as SpeechRecognizer, MediaPlayer, and Speaker are implemented by the [System Audio extension](../../../extensions/experimental/system-audio/README.md). For other platform APIs, such as Navigation and PhoneControl, the implementation converts the C++ methods into JSON response messages with payloads that it passes back to the Controller.

### Reference Audio I/O Implementation <a id = "reference-audio-io-implementation"></a>
The Alexa Auto SDK System Audio extension, which is included automatically by the Builder when you build the Auto SDK, provides the reference audio input/output implementation. It includes the platform implementations of the `AudioOutput` and `AudioOutputProvider` interfaces for audio playback and the `AudioInput` and `AudioInputProvider` interfaces for audio capturing capability. Please refer to the [System Audio Extension readme](../../../extensions/experimental/system-audio/README.md) for further details.

## Setting up the AGL Alexa Voice Agent <a id ="setting-up-the-agl-alexa-voice-agent"></a>

To use the Android Sample App, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Registered Product and Security Profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required for your configuration file.
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required for your configuration file.

### Alexa Auto SDK Build Dependencies

The AGL Alexa Voice Agent depends on the Auto SDK. You must install the AGL SDK toolchain prior to building the Alexa Auto SDK. See the [Alexa Auto SDK Builder](../../../builder/README.md) instructions to build the Alexa Auto SDK binaries for your [AGL target](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf). Currently, only the [ARM64](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf) target is supported, and the binding is tested on the [R-CAR M3 board](https://www.renesas.com/us/en/solutions/automotive/soc/r-car-m3.html).

For example, if you have installed the following toolchain:

```
https://iot.bzh/download/public/2019/AGL_Images/m3ulcb/latest/sdk/poky-agl-glibc-x86_64-agl-demo-platform-crosssdk-aarch64-toolchain-7.99.1.sh
```

which is installed by default under `/opt/agl-sdk/7.99.1-aarch64`, then you would need to provide the `AGL_SDK` path by passing: `-DAGL_SDK=/opt/agl-sdk/7.99.1-aarch64`

from the command line when you run the build command.

Then to build the Alexa Auto SDK modules and needed dependencies for an `AGL ARM64` target, run the following commands:

```
$ git clone https://github.com/alexa/aac-sdk.git
$ ${AAC_SDK_HOME}/builder/build.sh agl -t aglarm64
$ pushd ${AAC_SDK_HOME}/builder/deploy/aglarm64/
$ tar -xvf aac-sdk-build-aglarm64.tar.gz
$ export AAC_INSTALL_ROOT=${AAC_SDK_HOME}/builder/deploy/aglarm64/opt/AAC
$ popd
```
### Enabling Device Capabilities

In order to use certain Alexa Auto SDK functionality, you must whitelist your product with Amazon. Copy the product's Amazon ID from the Developer Console and follow the whitelisting directions on the [Need Help?](../../../NEED_HELP.md#requesting-additional-functionality-whitelisting) page.

## Configuring the AGL Alexa Voice Agent <a id ="configuring-the-agl-alexa-voice-agent"></a>
The Alexa Voice Agent binding supports a JSON configuration file (`AlexaAutoCoreEngineConfig`) that you must populate before compiling the software:

1. Open the [`AlexaAutoCoreEngineConfig.json`](./src/plugins/data/config/AlexaAutoCoreEngineConfig.json) file.

2. Leave the `"libcurlUtils"` node populated with its default values. 
3. Leave the `"deviceSettings"` node populated with its default value, or change the default `"locale"` and `"timezone"` to match your location values.
4. Update the `"aace.vehicle"` node with your vehicle information, if necessary.
5. In the `"aace.audio.input"` node, set `"voice"` to the Advanced  Linux Sound Architecture (ALSA) name for your audio subsystem. (The `"hw:ep812ch"` role maps to Microchip's mic array.)
6. In the `"aace.audio.output"` node:
    * Set `"tts"` to `"multimedia"`.
    * Set `"music"` to `"emergency"`.
    * Set `"notification"` to `"notification"`.
    * Set `"alarm"` to `"navigation"`.
    * Set `"earcon"` to `"earcon"`.
    * Set `"communication"` to `"communication"`.
    * Set `"ringtone"` to `"ringtone"`.
7. Leave the `"aace.wakeword"` and `"aace.cbl"` `"enabled"` parameters set to their default (`true`) settings.
8. To configure support for Local Media Sources (LMS), make these updates in the `"aace.localmediasource"` node:
    * Set `"enabled"` to `"true"`.
    * For each media source that you want to enable, set the corresponding parameter in the `"sources"` node to `"true"`.
9. If desired, [configure the AGL Alexa Voice Agent for Car Control](#configuring-the-agl-alexa-voice-agent-for-car-control).

    >**Note:** In order for your application to support car control, you must compile the Auto SDK with the optional Local Voice Control (LVC) extension before configuring the AGL Alexa Voice Agent for car control. After you compile the Auto SDK with the LVC extension, you can configure the AGL Alexa Voice Agent to support hybrid car control functionality. 
  
### Configuring the AGL Alexa Voice Agent for Car Control <a id = "configuring-the-agl-alexa-voice-agent-for-car-control-(lvc)"></a>

The AGL Alexa Voice agent supports hybrid car control functionality and requires the LVC extension. The LVC extension is available by request. Please [contact your Amazon Solutions Architect (SA)](../../../NEED_HELP.md#requesting-additional-functionality-whitelisting) for more information about how to obtain the LVC extension.

  >**Important!** For hybrid car control support, you must configure the system to automatically start LVC during system startup. The LVC binaries are installed under `/opt/LVC`, and a start script is provided under `/opt/LVC/start-lvc.sh`.

The AGL Alexa Voice agent supports a custom vehicle-control experience comprised of the following elements:

* **The concept of endpoint control:** Different components or "endpoints" in the vehicle can be controlled separately. Endpoints are identified by the names of the component they control (e.g. "fan", "vent", or "heater"), which allows the user to target the selected endpoint by voice.
* **The concept of zone:** Each endpoint belongs to a "zone" or to the entire vehicle ("zone.all"). A zone is identified by its name (e.g. "driver", "front" or "all"). The AGL Alexa Voice Agent supports the following predefined zones: `"zone.all"`, `"zone.driver"`, `"zone.passenger"`, `"zone.back.driver"`, `"zone.back.passenger"`, `"zone.first.row"`, `"zone.second.row"`, `"zone.third.row"`, `"zone.fourth.row"`, `"zone.front"`, `"zone.rear"`.
    
* **A simple set of control capabilities:** You make an endpoint controllable when you include a `"controlId"` (associated with an enabled zone) and define controllers and associated values for that control in the [`AlexaAutoCoreEngineConfig.json`](./src/plugins/data/config/AlexaAutoCoreEngineConfig.json) file.
    
By modifying the appropriate sections of the [`AlexaAutoCoreEngineConfig.json`](./src/plugins/data/config/AlexaAutoCoreEngineConfig.json) file, you can build a custom configuration that defines endpoints, with their associated zones and controllers, to match the specific vehicle you wish to control using the AGL Alexa Voice Agent:

1. Update the `"aace.localvoicecontrol"` node:
          *  Set `"enabled'` to `true`.
          *  Set `"socketRootDirectory"` to the same path where the LVC extension is installed (the default path is "`/opt/"`).
 
2. In the `"aace.localskills"` node, configure the `"aace.carcontrol"` settings:
      *  Set `"enabled"` to `true`.
      *  Under `endpoints`, configure the zones and associated controls for your vehicle.
           * For each zone that you want to include in the car control configuration:
               * Set `"enabled"` to `true`.
           * For each endpoint (component) that you want to make controllable for this zone:
               * Specify a `"controlId"` that maps the endpoint to the zone. For example, to include a heater control for the `"zone.driver"` zone, set the `"heater"` `"controlId"` to `"driver.heater"`.
               <br>**Each `"controlId"` must be unique across the car control configuration.**
       
               * Set the `"controllerId"`(s) and (if applicable) `"values"` for the controllers associated with the control.
               >**Note:** Each control inherits the functionality to be turned on or off ("Alexa turn on/off the heater/AC/light/fan/vent") in addition to the functionality provided by the controller.
               
               * For each zone that you want to exclude from the car control configuration, make sure that `"enabled"` is set to `false`.

               >**Note:** The `AlexaAutoCoreEngineConfig.json` file includes a default configuration for `"zone.all"` which you can modify as you like. To configure other zones, you must build out configurations for those zones in a similar fashion and make sure the `"enabled"` parameter is set to `true` for all zones you want to include in your AGL Alexa Voice Agent car control configuration.

<details><summary>Example passenger zone definition (click to expand or collapse)</summary>
<p>

```
{                  
    "zone" : "zone.passenger",
    "enabled": true
    "airconditioner" : {
        "controlId": "passenger.ac",
        "mode": {
           "controllerId": "mode",
           "values" : {
              "AUTO": true,
              "ECONOMY": true,
              "MANUAL": true
           }
        },
        "intensity" : { 
           "controllerId": "intensity",
           "values" : {
             "LOW": true,
             "MEDIUM": true,
             "HIGH": true
           }
        }
    },
    "heater" : {
        "controlId": "passenger.heater",
        "controllerId": "temperature",
        "minimum": 60,
        "maximum": 90,
        "precision": 1,
        "unit":  "FAHRENHEIT"
    },
    "fan" : {
        "controlId": "passenger.fan",
        "controllerId": "speed",
        "minimum": 1,
        "maximum": 10,
        "precision": 1
    },
    "vent": {
        "controlId": "all.vent",
        "positions" : {
          "controllerId": "position",
          "values" : {
            "BODY": true,
            "MIX":  false,
            "FLOOR": false,
            "WINDSHIELD":  false
          }
        }
    },
    "lights" : [ 
      {
        "controlId": "reading.light",
        "type": "READING_LIGHT"
      }
    ]

}   
        
```
</p>
</details>
<br>

The table below lists the endpoints and associated controllers and values (if any) that you can define in the [`AlexaAutoCoreEngineConfig.json`](./src/plugins/data/config/AlexaAutoCoreEngineConfig.json) file.

* If a controller does not have an associated list of values, simply specify a `"controllerId"` comprised of the endpoint and the controller. For example, to include a climate recirculation control, set the `"controllerId"` for `"recirculate"` to `"climate.recirculate"`.

* For the `"lights"` endpoint, specify a  `"controlId"` that maps the control to the type of light, and set the `"type"` value to that type. For example, to include a dome light control, set `"controlId"` to `"dome.light"` and `"type"` to `"DOME_LIGHT"`. Optionally, you can also specify a light color by setting the `"color"` controller. If you specify a color, the user can make requests such as "Alexa, turn the dome light to blue." If you don't specify a `"color"`, the user can still ask Alexa to turn the light on or off; for example: "Alexa, turn the dome light on".

<table style="width: 100%">
  <colgroup>
    <col width="20%">
    <col width="20%">
    <col width="20%">
    <col width="40%">
  </colgroup>
  <table body>
  <thead>
    <tr>
      <th>Control</th>
      <th>Controllers</th>
      <th>Values</th>
      <th>Example Utterance</th>
    </tr>
  </thead>
  <tr>
    <td markdown="span">"climate"</td>
    <td markdown="span">"sync"
    <br> "recirculate"</td>
    <td markdown="span">
    <td markdown="span">"Alexa, turn climate sync off"
    <br>
    <br>"Alexa, turn on recirculation"</td>
  </tr>
  <tr>
    <td markdown="span">"airconditioner"</td>
    <td markdown="span">"mode"
    <br>
    <br>
    <br>
    <br>"intensity"
    <br>
    <br>
    <br></td>
    <td markdown="span">"AUTO": true or false
    <br>"ECONOMY": true or false
    <br>"MANUAL": true or false
    <br>
    <br>"LOW": true or false
    <br>"MEDIUM": true or false
    <br>"HIGH": true or false</td>
    <td markdown="span">"Alexa, turn the AC on"
    <br>
    <br>"Alexa, set the AC to economy"
    <br>
    <br>"Alexa, set the AC to high"</td>
  </tr>
  <tr>
    <td markdown="span">"heater"</td>
    <td markdown="span">"temperature"
    <td markdown="span">"minimum": minimum temperature setting
    <br>"maximum": maximum temperature setting
    <br>"precison": number of degrees by which the temperature is adjusted when users ask Alexa to increase or decrease the temperature
    <br>"unit": "FAHRENHEIT" or "CELSIUS"
    <td markdown="span">"Alexa, turn the heater off"
    <br>
    <br>"Alexa, turn up the heater"</td>
  </tr>
  <tr>
    <td markdown="span">"fan"</td>
    <td markdown="span">"speed"
    <td markdown="span">"minimum": minimum value for fan speed
    <br>"maximum": maximum value for fan speed
    <br>"precision": the number by which users can adjust fan speed when asking Alexa to increase or decrease the fan
    <td markdown="span">"Alexa, turn the fan on"
    <br>
    <br>"Alexa, set the fan to maximum"
    <br>
    <br>"Alexa, increase the fan by 1"</td>
</tr>
  <tr>
    <td markdown="span">"vent"</td>
    <td markdown="span">"positions"
    <td markdown="span">"BODY": true or false
    <br>"MIX": true or false
    <br>"FLOOR": true or false
    <br>"WINDSHIELD": true or false
    <td markdown="span">"Alexa, set the vent to floor"
</td>
  </tr>
  <tr>
    <td markdown="span">"window"</td>
    <td markdown="span">"defroster"</td>
    <td>"defrost": true or false</td>
    <td markdown="span">"Alexa, turn the defroster off"</td>
   </tr
  <tr>
    <td markdown="span">"lights"
    <td markdown="type">"type"
    <br>
    <br>
    <br>
    <br>
    <br> "color"
    <br>
    <br>
    <br>
    <br>
    <br>
    <br>
    <br>
    <br>
    <td markdown="AMBIENT_LIGHT"
    <br> "DOME_LIGHT"
    <br> "LIGHT"
    <br> "READING_LIGHT"
    <br> "TRUNK_LIGHT"
    <br>
    <br> "WHITE"
    <br>"RED"
    <br>"ORANGE"
    <br>"YELLOW"
    <br>"GREEN"
    <br>"BLUE"
    <br>"INDIGO"
    <br>"VIOLET"</td>
    <td markdown="span">"Alexa, turn the reading light on"
    <br>
    <br>"Alexa, set the dome light to blue"</td>
  </tr>
  </table body>
  </table>

## Building the AGL Alexa Voice Agent <a id="building-the-agl-alexa-voice-agent"></a>

>**Prerequisite**: Install the AGL SDK, preferably the latest stable release from [IOT.bzh AGL artifacts](https://iot.bzh/download/public/2019/AGL_Images/m3ulcb/latest/). The AGL SDK contains [application framework libraries](http://docs.automotivelinux.org/docs/en/master/apis_services/reference/af-binder/reference-v3/func-api.html) that are required to make inter-binding calls and publish AGL events.

Go to the `${AAC_SDK_HOME}/platform/agl/alexa-voiceagent-service` directory and enter the following commands to generate the **alexa-voiceagent-service-debug.wgt** in the build folder:

1. `mkdir build`
2. `pushd build`
3. `source /opt/agl-sdk/\<sdk-version>/environment-setup-aarch64-agl-linux`
4. `cmake .. -DAAC_HOME=${AAC_INSTALL_ROOT} -DCMAKE_BUILD_TYPE=Debug`
5. `make widget`

## Installing and Running the AGL Alexa Voice Agent <a id = "installing-and-running-the-agl-alexa-voice-agent"></a>

1. Copy the **alexa-voiceagent-service-debug.wgt** into the target device.
2. In the shell of the target device, run the following command:

 `afm-util install alexa-voiceagent-service-debug.wgt`
3. Reboot the target device.

## Authorizing with the Alexa Voice Service (AVS) <a id = "authorizing-with-the-alexa-voice-service-avs"></a>

To access the Alexa Voice Service (AVS) your product must acquire [Login with Amazon (LWA)](https://developer.amazon.com/login-with-amazon) access tokens. The Alexa Voice Agent binding has an implentation of [Code-Based-Linking](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html) that you can use to acquire such tokens and authorize with AVS:

1. On your desktop browser, load `${AAC_SDK_HOME}/platform/agl/alexa-voiceagent-service/htdocs/index.html`
2. In the input text field, enter the socket address of the Alexa Voice Agent binding running on the target device. For example: `\<IP Address>:\<Port>`
3. Click the **Subscribe to CBL Events** button to display a URL and code in the event box of the page.
4. Navigate to the URL, enter the code, and agree to authorize your product. This will move the Alexa Voice Agent binding to the `CONNECTED and AUTHORIZED` state, and it will be ready to accept user utterances.

## v2.1.0 Release Notes <a id="v210-release-notes"></a>

#### Supported Features

* Car Control (requires LVC extension)
* Local Media Services
* TTS Responses
* Alexa cards
* Phone call control
* Navigation
* Online Music Playback
* Playback Controller

#### Known Issues
	
* The Alexa voice agent supports timers and alarms, however, in order to implement them you must assign them an audio role different than the ones you assign to TTS and music. Auto SDK v2.1.0 supports AGL v7.99, which provides only two functioning audio roles: “emergency” and “multimedia”. Since you must assign these roles to TTS and music in order to support those functionalities, no additional audio roles remain to be assigned to alarms and timers.
* After repeated barge-ins, the AGL Alexa voice agent may no longer play TTS responses. The dialog state will change from `THINKING` to `IDLE` unexpectedly. Rebooting the device or Alexa voice agent will fix the issue.
* Playback controls are not supported for local media sources.
* Adjusting volume by delta has no effect; however, you can adjust the volume by absolute values or up/down requests.
* There is no way to log out after authorizing with AVS. Restart the Alexa Voice Agent to authorize again.