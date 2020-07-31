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
* [Known Issues](#ki)

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

To use the Alexa Voice Agent, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Registered Product and Security Profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required for your configuration file.
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required for your configuration file.

### Alexa Auto SDK Build Dependencies

The AGL Alexa Voice Agent depends on the Auto SDK. You must install the AGL SDK toolchain prior to building the Alexa Auto SDK. See the [Alexa Auto SDK Builder](../../../builder/README.md) instructions to build the Alexa Auto SDK binaries for your [AGL target](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf). Currently, only the [ARM64](../../../builder/meta-aac-builder/conf/machine/aglarm64.conf) target is supported, and the binding is tested on the [R-CAR M3 reference board](https://www.renesas.com/us/en/solutions/automotive/soc/r-car-m3.html) and the Raspberry Pi 4.

It is recommended to [build an AGL image](https://docs.automotivelinux.org/docs/en/master/getting_started/) for the target device that will be used. The AGL build will generate both an image and a toolchain for that target platform. AGL version Itchy Icefish supports building some versions of the Alexa Voice Agent and wakeword. So it is recommended to build with those components.

For example, you can specify the Alexa Voice Agent and wakeword as follows:

```
source meta-agl/scripts/aglsetup.sh -m m3ulcb -b build agl-demo agl-devel agl-voiceagent-alexa agl-voiceagent-alexa-wakeword
```

  >**Important!** To build the Alexa wakeword you will need to download the appropriate package from the [developer portal](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Amazonlite%20Extension). The version of the Alexa Auto SDK supported by AGL may not always be the latest publicly available version available on github.

For example, the generated toolchain will have a structure similar to this one:

```
https://download.automotivelinux.org/AGL/release/icefish/latest/raspberrypi4/deploy/sdk/poky-agl-glibc-x86_64-agl-demo-platform-crosssdk-aarch64-toolchain-9.0.0.sh
```

and will be installed by default under `/opt/agl-sdk/9.0.2-aarch64`, then you would need to provide the `AGL_SDK` path by passing: `-DAGL_SDK=/opt/agl-sdk/9.0.2-aarch64`

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
5. In the `"aace.audio.input"` node, set `"voice"` to the Advanced Linux Sound Architecture (ALSA) name for your audio input device. The `"Default"` value will use the default microphone device. You may pass a different device by using the format "hw:1,0")
6. In the `"aace.audio.output"` node, leave the defaults values as specified. However, please refer to the [AGL pipewire](https://docs.automotivelinux.org/docs/en/master/apis_services/reference/audio/audio/pipewire.html) documentation to understand additional available roles.
    * Set `"tts"` to `"Speech-Low"`.
    * Set `"music"` to `"Speech-High"`.
    * Set `"alarm"` to `"Custom-Low"`.
7. If Amazon wakeword is required, leave the `"aace.wakeword"` `"enabled"` parameter set to (`true`).
8. To configure support for Local Media Sources (LMS), make these updates in the `"aace.localmediasource"` node:
    * Set `"enabled"` to `"true"`.
    * For each media source that you want to enable, set the corresponding parameter in the `"sources"` node to `"true"`.
    * For each media source that is enabled, you need to make sure that the [voice high capabilities](https://gerrit.automotivelinux.org/gerrit/admin/repos/apps/agl-service-voice-high-capabilities) messages are implemented otherwise the Alexa Voice Agent will not listen to utterances.
9. If desired, [configure the AGL Alexa Voice Agent for Car Control](#configuring-the-agl-alexa-voice-agent-for-car-control).   
  
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

## Known Issues	<a id = "ki"></a>
* There is no way to log out after authorizing with AVS. Restart the Alexa Voice Agent to authorize again.