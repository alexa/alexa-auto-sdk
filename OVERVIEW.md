# Alexa Auto SDK Overview

The Alexa Auto SDK is designed to allow flexibility in the integration of Alexa as a part of the automotive in-cabin experience. The Alexa Auto SDK Engine provides runtime support for Alexa and other services and allows your application to integrate with platform-specific functionality. Platform-specific behavior in the Alexa Auto SDK is abstracted into interfaces called "platform interfaces." To implement the platform-specific functionality in your application, you extend the platform interfaces (by creating custom handlers) and register these platform interface handlers with the Engine.

**Table of Contents**

* [Alexa Auto SDK Architecture and Modules](#alexa-auto-sdk-architecture-and-modules)
* [Optional Extensions](#optional-extensions)

## Alexa Auto SDK Architecture and Modules <a id="alexa-auto-sdk-architecture-and-modules"></a>

> **Note**: This diagram shows only some of the available platform interfaces.

![architecture](./assets/aac_architecture.png)

### ![Alexa Auto SDK Engine](./assets/number-1.png) Alexa Auto SDK Engine

The runtime implementation of the Alexa Auto SDK is known as the Engine. Modules extend the Engine by providing services and defining the runtime behavior for platform interfaces that are registered by the application. The application software communicates with the Engine through the API defined by the platform interfaces.

### ![Alexa Auto SDK Engine](./assets/number-2.png) Core Module

The Core module (for [C++](./modules/core/README.md) or [Android](./platforms/android/modules/core/README.md)) includes platform interfaces and runtime Engine support for Core features in the Alexa Auto SDK, such as logging, location, and network information. The services are used by components in other modules and are required by the Alexa Auto SDK.


### ![Alexa Auto SDK Engine](./assets/number-3.png) Alexa Module

The Alexa module (for [C++](./modules/alexa/README.md) or [Android](./platforms/android/modules/alexa/README.md)) includes platform interfaces and runtime Engine support for Alexa features in the Alexa Auto SDK, including speech input and output, audio output, authorization, Alexa speaker, media playback, equalizer control, template and state rendering, local media sources, alerts, notifications, and Do Not Disturb (DND). 

### ![Alexa Auto SDK Engine](./assets/number-4.png) Navigation Module

The Navigation module (for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md)) includes platform interfaces and runtime Engine support for Alexa to interface with the onboard navigation system.


### ![Alexa Auto SDK Engine](./assets/number-5.png) Phone Call Controller Module

The Phone Control module (for [C++](./modules/phone-control/README.md) or [Android](./platforms/android/modules/phonecontrol/README.md)) includes platform interfaces and runtime Engine support for Alexa to interface with the onboard telephony system.

### Address Book Module

The Address Book module (for [C++](./modules/address-book/README.md) or [Android](./platforms/android/modules/addressbook/README.md)) includes platform interfaces and runtime Engine support to augment the communications and navigation capabilities of Alexa with user data such as phone contacts and/or navigation favorites ("home", "work", etc.)

### Code-Based Linking (CBL) Module

The CBL module (for [C++](./modules/cbl/README.md) or [Android](./platforms/android/modules/cbl/README.md)) includes platform interfaces and runtime Engine support to implement the Code-Based Linking (CBL) mechanism of acquiring Login with Amazon (LWA) access tokens.

### Alexa Presentation Language (APL) Module
The APL module (for [C++](./modules/apl/README.md) or [Android](./platforms/android/modules/apl/README.md)) includes platform interfaces and runtime Engine support to enable devices with graphical user interface (GUI) support to implement APL directives and events.

>**Note:** APL rendering on the Android Sample App requires a component that is available by [request from your Amazon Solutions Architect (SA) or Partner Manager](./NEED_HELP.md#requesting-additional-functionality-whitelisting).

### Messaging Module 

The Messaging module (for [C++](./modules/messaging/README.md) or [Android](./platforms/android/modules/messaging/README.md)) includes platform interfaces and runtime Engine support for Short Message Service (SMS) capabilities of Alexa such as sending and reading text messages.

## Optional Extensions <a id ="optional-extensions"> </a>
The following extensions to the Alexa Auto SDK are available by request. Please contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

>**Important!** The optional extensions are provided as archives. In order to install an optional extension, you must first download the archive. The version of the optional extension archive must match the version of the Auto SDK that you are using. For example, if you are using Auto SDK 2.0 and want to install the Local Voice Control extension, you must download version 2.0 of the Local Voice Control extension archive from the link provided below.

### AmazonLite Wake Word extension <a id="wake-word-extension"></a>
Wake Word enables hands-free, voice-initiated interactions with Alexa. The Wake Word extension enables AmazonLite Wake Word support in the Alexa Auto SDK.

To install the optional AmazonLite Wake Word extension, you must download the appropriate archive version from the [Auto SDK AmazonLite Extension resources directory](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Amazonlite%20Extension).

### Alexa Communications extension <a id="alexa-communication-extension"></a>
The Alexa Communications extension enables integration with Alexa-to-Alexa calling, Alexa-to-PSTN calling, and messaging capabilities.

To install the optional Alexa Communications extension, you must download the appropriate archive version from the [Auto SDK Alexa Comms Extension resources directory](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Alexa%20Comms%20Extension).

### Local Voice Control (LVC) extension <a id="local-voice-control-extension"></a>
The Local Voice Control (LVC) extension provides car control, communication, navigation, and entertainment functionality, with and without an internet connection. It includes components that run an Alexa endpoint inside the vehicle's head unit. Local Voice Control is currently supported on Linux x86 64-bit, Linux ARM 32/64-bit, Android x86 64-bit, and Android ARM 32/64-bit platforms.

To install the optional LVC extension, you must download the appropriate archive version from the [Auto SDK Local Voice Control Extension resources directory](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Local%20Voice%20Control%20Extension).

### Device Client Metrics (DCM) extension <a id="metrics-uploading-extension"></a>
The Device Client Metrics (DCM) extension enables logging and uploading Alexa Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and user perceived latency (UPL) between the request and Alexa’s response. Metrics may be tagged Beta, Gamma, or Prod depending on the vehicle lifecycle. The DCM extension is available for all Linux and Android platforms.

To install the optional DCM extension, you must download the appropriate archive version from the [Auto SDK Device Metric Upload Service Extension resources directory](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Metric%20Upload%20Service%20Extension).

### Voice Chrome for Android extension <a id="voice-chrome-extension"></a>
The Voice Chrome extension adds Voice Chrome support to the Auto SDK for Android x86 64-bit and Android ARM 32/64-bit platforms. Voice Chrome provides a consistent set of visual cues representing Alexa attention state across a range of Alexa-enabled devices. The Voice Chrome extension includes a prebuilt Android AAR library for easy integration with your applications, as well as a patch to the Android Sample App that adds the Voice Chrome functionality.

To install the optional Voice Chrome for Android extension, you must download the appropriate archive version from the [Auto SDK Voice Chrome Extension resources directory](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Voice%20Chrome%20Extension).