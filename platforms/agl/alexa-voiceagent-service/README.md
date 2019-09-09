# Automotive Grade Linux Alexa Voice Agent

The AGL Alexa Voice Agent is Alexa client software that is plugged into the AGL speech framework to voice-enable applications, as described in the speech [architecture](https://confluence.automotivelinux.org/display/SPE/Speech+EG+Architecture).

**Table of Contents**

* [Overview](#overview)
* [Architecture](#architecture)
* [Setup](#setup)
* [Building the AGL Alexa Voice Agent](#building)
* [Installing and Running the AGL Alexa Voice Agent](#installandrun)
* [Authorization with AVS](#authorization)
* [Release Notes](#releasenotes)

## Overview <a id ="overview"></a>

The AGL Alexa Voice Agent is implemented as a standard AGL [binding](https://docs.automotivelinux.org/docs/en/master/apis_services/reference/af-main/1-afm-daemons.html) that exposes APIs for:

* Speech recognition start and cancel
* Subscription to events containing Alexa's dialog, authentication, and connection states
* Subscription to events containing Alexa's capability messages or directives
* User authentication and authorization using Amazon's [Login with Amazon (LWA) Code-Based Linking](https://developer.amazon.com/docs/login-with-amazon/minitoc-lwa-other-devices.html)

[Alexa Auto SDK](https://github.com/alexa/alexa-auto-sdk) is the underlying technology that powers the speech recognition capabilities of this binding.

>**License Information:** The [Google Test v1.8.0](https://github.com/google/googletest) dependencies are fetched and run by the build system when the AGL Alexa Voice Agent is compiled with the `ENABLE_AASB_UNIT_TESTS` option.


## Architecture<a id = "architecture"></a>
![architecture](./assets/architecture.png)

### Binding Controller
The [Binding controller](./src/plugins/AlexaVoiceagentApi.cpp) is the entry point of the binding. It does initialization and dependency injection and also handles the requests for methods/verbs exposed by the binding.

### Auto SDK Platform Handlers
The Auto SDK platform handlers host the C++ platform API implementation handlers of the Alexa Auto SDK. Audio related APIs such as SpeechRecognizer, MediaPlayer and Speaker are implemented by the [Alexa Auto GStreamer Extension](../../../extensions/experimental/gstreamer/README.md). For other platform APIs such as Navigation, PhoneControl etc., the implementation converts the C++ methods into JSON response messages with payloads that it passes back to the Controller.

### Reference Audio I/O Implementation
The Auto SDK uses a GStreamer extension, hosted in the `${AAC_SDK_HOME}/extensions/experimental` folder, as its reference I/O audio implementation. This extension is a GStreamer based audio reference implementation of the MediaPlayer and Speaker platform APIs. It also provides an interface for reading audio input from the microphone. Please refer to the [GStreamer Extension readme](../../../extensions/experimental/gstreamer/README.md) for supported platforms and known issues.

## Setup<a id ="setup"></a>
### Register a Product
After registering for an Amazon developer account, you'll need to create an Alexa device and security profile. Make note of the following parameters as you go through setup: Product ID, Client ID, and Client Secret.

1. Log in to the [Amazon Developer portal](https://developer.amazon.com) and navigate to the [My Products](https://developer.amazon.com/avs/home.html#/avs/home) view of the AVS Console.
2. Click **Create Product** and fill in the registration form for your application. (The device registration details can be useful in certain development workflows but will not actually be used for this example.)
3. Make note of the **Product ID**.
4. Click **Next**.
5. Click **Create New Profile**.
6. Enter a name and description for your security profile.
7. Click **Next**.
8. Click on the **Other devices and platforms** tab.
9. Enter the **Client Id Name** and click **GENERATE ID**.
8. Make note of the **Client ID**.
9. Agree to the license terms in the Developer Portal and click **Finished**.

### Alexa Auto SDK Build Dependencies
The Alexa Voice Agent depends on Auto SDK. You must install the AGL SDK toolchain prior to building the Alexa Auto SDK. See the [Alexa Auto SDK Builder](../../../builder/README.md) instructions to build the Alexa Auto SDK binaries for your [AGL target](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf). Currently, only the [ARM64](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf) target is supported, and the binding is tested on the [R-CAR M3 board](https://www.renesas.com/us/en/solutions/automotive/soc/r-car-m3.html).

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
In order to use the certain Alexa Auto SDK functionality, you must whitelist your product with Amazon. Copy the product's Amazon ID from the Developer Console and follow the whitelisting directions on the [Need Help?](../../../NEED_HELP.md) page.

## Configuration
The Alexa Voice Agent binding supports a JSON configuration file that you must populate before compiling the software.
Open `${AAC_SDK_HOME}/platforms/agl/alexa-voiceagent-service/src/plugins/data/config/AlexaAutoCoreConfig.json` and:

1. Populate the **deviceInfo** section. This information will be used for authorizing the device with AVS during the LWA Code-Based Linking process.
2. Update the **aace.vehicle** section with your vehicle information if necessary.
3. Update the **aace.audio.input** section:
  * **voice**: update to **Audio4a** input role. The hw:ep812ch role maps to Microchip's mic array.
  * Otherwise use the default values in the config file.
4. Update the **aace.audio.output** section:
  * **tts**: update to Audio4a TTS role
  * **music**: update to Audio4a Music role
  * **notification**: update to Audio4a Notification role
  * **alarm**: update to Audio4a Alarm role
  * **earcon**: update to Audio4a Earcon role
  * **communication**: update to Audio4a Communication role
  * **ringtone**: update to Audio4a Ringtone role
5. Update the aace.localvoicecontrol configuration if you are compiling the Auto SDK with the optional Local Voice Control (LVC) module

## Building the AGL Alexa Voice Agent<a id="building"></a>
>**Prerequisite**: Install the AGL SDK, preferably the latest stable release from [IOT.bzh AGL artifacts](https://iot.bzh/download/public/2019/AGL_Images/m3ulcb/latest/). The AGL SDK contains [application framework libraries](http://docs.automotivelinux.org/docs/en/master/apis_services/reference/af-binder/reference-v3/func-api.html) that are required to make inter-binding calls and publish AGL events.

Go to the `${AAC_SDK_HOME}/platform/agl/alexa-voiceagent-service` directory and enter the following commands to generate the **alexa-voiceagent-service-debug.wgt** in the build folder:

1. `mkdir build`
2. `pushd build`
3. `source /opt/agl-sdk/\<sdk-version>/environment-setup-aarch64-agl-linux`
4. `cmake .. -DAAC_HOME=${AAC_INSTALL_ROOT} -DCMAKE_BUILD_TYPE=Debug`
5. `make widget`

## Installing and Running the AGL Alexa Voice Agent<a id = "installandrun"></a>
1. Copy the **alexa-voiceagent-service-debug.wgt** into the target device.
2. In the shell of the target device, run the following command:

 `afm-util install alexa-voiceagent-service-debug.wgt`
3. Reboot the target device.

## Authorization with the Alexa Voice Service<a id = "authorization"></a>
To access the Alexa Voice Service (AVS) you must acquire [Login with Amazon (LWA)](https://developer.amazon.com/login-with-amazon) access tokens. The Alexa Voice Agent binding has an implentation of **Code-Based Linking (CBL)** that you can use to acquire such tokens and authorize with AVS.

**Code-Based Linking (CBL)**: The user is provided with a short alphanumeric code and a URL in which to enter the code on any web browser-enabled device. [Read more about CBL](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html), if desired.

1. On your desktop browser, load `${AAC_SDK_HOME}/platform/agl/alexa-voiceagent-service/htdocs/index.html`
2. In the input text field, enter the socket address of the Alexa Voice Agent binding running on the target device. For example: `\<IP Address>:\<Port>`
3. Click the **Subscribe to CBL Events** button to display a URL and code in the event box of the page.
4. Navigate to the URL, enter the code, and agree to authorize your product. This will move the Alexa Voice Agent binding to the CONNECTED and AUTHORIZED state, and it will be ready to accept user utterances.

## v2.0.0 Release Notes<a id="releasenotes"></a>

#### Supported Features

* TTS Responses
* Alexa cards
* Phone call control
* Navigation
* Online Music Playback
* Playback Controller

#### Resolved Issues
* Fixed an issue where the Alexa Voice Agent binding did not start on the latest AGL image.

#### Known Issues

* There is no way to log out after authorizing with AVS. Restart the Alexa Voice Agent to authorize again.
