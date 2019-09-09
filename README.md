# Alexa Auto SDK

The Alexa Auto SDK contains essential client-side software required to integrate Alexa into the automobile. The Alexa Auto SDK provides a runtime engine for communicating with the Alexa service. It also provides interfaces that allow you to implement platform specific behavior such as audio input, media playback, template and state rendering, phone control, and car control. It also includes two sample applications that demonstrate how to use the Alexa Auto SDK interfaces.

**Table of Contents**:

* [What's Included](#whatsincluded)
* [General Build Requirements](#generalbuildreqs)
* [Building the Alexa Auto SDK](#buildthesdk)
* [Alexa Auto SDK Architecture](#architecture)
* [Vehicle Configuration Information](#vehicleconfigurationdatarequirements)
* [Release Notes and Known Issues](#relnotesknownissues)

> **Note:** For UX design requirements, recommendations, and best practices, please see the [Alexa Automotive design documentation](https://developer.amazon.com/docs/alexa-auto/about-this-guide.html). For information on topics not covered in the list above, please see [Need Help?](./NEED_HELP.md).

## What's Included<a id="whatsincluded"></a>

The following components are included with the Alexa Auto SDK:

* [Builder](./builder/README.md): This directory contains the scripts that allow you to build modules for a variety of hardware targets.

* Modules: This directory contains the Alexa Auto SDK interfaces and source code in the following modules:
    >**Note:** The Android interfaces and source code are located in the `/platforms/android/modules` directory.
     * **Alexa** (for [C++](./modules/alexa/README.md) or [Android](./platforms/android/modules/alexa/README.md))
     * **Core** (for [C++](./modules/core/README.md) or [Android](./platforms/android/modules/core/README.md))
     * **Navigation** (for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md))
     * **Phone Call Controller** (for [C++](./modules/phone-control/README.md) or [Android](./platforms/android/modules/phonecontrol/README.md))
     * **Address Book** for [C++](./modules/address-book/README.md) or [Android](./platforms/android/modules/addressbook/README.md))
     * **Contact Uploader** (for [C++](./modules/contact-uploader/README.md) or [Android](./platforms/android/modules/contactuploader/README.md))
     
        >**Note** The Contact Uploader module is on the deprecation path and supported for existing implementations only. It is superseded by the Address Book module.
     * **CBL** (for [C++](./modules/cbl/README.md) or [Android](./platforms/android/modules/cbl/README.md))


* [Android Sample App](./samples/android/README.md): This directory contains the Android sample application that uses the Alexa Auto SDK to demonstrate end-to-end functionality.

* [C++ Sample App](./samples/cpp/README.md): This directory contains the C++ sample application that uses the Alexa Auto SDK to demonstrate end-to-end functionality.

Additional functionality is available with help from your Amazon Solutions Architect (SA) or Partner Manager:

* [Wake Word support](#wake-word-extension)
* [Alexa Communications](#alexa-communication-extension)
* [Local Voice Control (LVC)](#local-voice-control-extension)
* [Device Client Metrics (DCM)](#metrics-uploading-extension)
* [Voice Chrome for Android](#voice-chrome-extension)

Alexa Auto SDK API Reference documentation is available for Android and C++. Access the Alexa Auto SDK API reference documentation from the links below.

* [Alexa Auto SDK for Android](https://alexa.github.io/alexa-auto-sdk/docs/android/)
* [Alexa Auto SDK for C++](https://alexa.github.io/alexa-auto-sdk/docs/cpp/)

> **Important!** You need an [Amazon Developer Account](https://developer.amazon.com/home.html) to use the Alexa Auto SDK. Read the [Android Sample App](./samples/android/README.md) documentation for more details.

## General Build Requirements<a id="generalbuildreqs"></a>

You can build on a Linux, Unix, or macOS host of your choice.

However, we recommend and support running a Docker environment with the following configuration.

* macOS Sierra or Ubuntu 16.04 LTS
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use.

### Build Dependencies and License Information

During the build time, the following dependencies are fetched and built for the target platform by the [Alexa Auto SDK Builder](./builder/README.md). Please refer to each of the individual entities for the particular licenses.

* [AVS Device SDK v1.13](https://github.com/alexa/avs-device-sdk/)
  * [cURL 7.65.3](https://curl.haxx.se/)
  * [ngHTTP2 1.39.1](https://github.com/nghttp2/nghttp2)
  * [SQLite3 3.20.0](https://www.sqlite.org/)
  * [OpenSSL 1.1.0g](https://www.openssl.org/)
* [Google Test v1.8.0](https://github.com/google/googletest)
* [libopus 1.3.1](https://opus-codec.org/)
* [OpenEmbedded-Core Thud](https://www.openembedded.org/wiki/OpenEmbedded-Core)
* [BitBake](https://www.yoctoproject.org/software-overview/)
* [Android Toolchain](https://www.android.com/)
  * [NDK r20](https://developer.android.com/ndk/)
  * [SDK Tools 26.0.1](https://developer.android.com/studio/releases/sdk-tools)

> **Note**: that *OpenEmbedded-Core* will fetch and build additional components for preparing the dedicated toolchain for your environment (Such as *GNU Binutils*). Please refer to the [Yocto project](https://www.yoctoproject.org/software-overview/) to understand how it works.

## Building the Alexa Auto SDK<a id="buildthesdk"></a>

To build the Alexa Auto SDK you should read and understand the instructions in the [Builder README](./builder/README.md).

The recommended and tested host operating systems are **Ubuntu 16.04 LTS** or **macOS Sierra**.

### Supported Target Platforms

The Alexa Auto SDK is supported on the following platforms:

* Android 5.1 Lollipop API Level 22 or higher.
    * ARM 32-bit
    * ARM 64-bit
    * x86 32-bit
    * x86 64-bit
* QNX 7.0
    * ARM 64-bit
    * x86 64-bit
* AGL
    * ARM 64-bit
* Generic Linux
    * x86 64-bit
* Poky Linux
    * ARMv7a (+NEON)
    * AArch64

> **Note**: For Android targets, pre-built platform AARs for the default Auto SDK modules are available in the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/). Read the instructions about downloading and using the AARs in the [Android Sample App README](./samples/android/README.md).

## Alexa Auto SDK Architecture<a id="architecture"></a> Overview

The Alexa Auto SDK is designed to allow flexibility in the integration of Alexa as a part of the automotive in-cabin experience. The Alexa Auto SDK Engine provides runtime support for Alexa and other services and allows the application to integrate with platform specific functionality. Platform-specific behavior in Alexa Auto is abstracted into interfaces called "Platform Interfaces." These interfaces should be extended in the application and registered with the Engine.

The Platform Interfaces are documented in their respective module's API documentation.

> **Note**: This diagram shows only some of the available platform interfaces.

![architecture](./assets/aac_architecture.png)

### ![Alexa Auto SDK Engine](./assets/number-1.png) Alexa Auto SDK Engine

The runtime implementation of the Alexa Auto SDK is known as the Engine. Modules extend the Engine by providing services and defining the runtime behavior for Platform Interfaces that are registered by the application. The application software communicates with the Engine through the API defined by the Platform Interfaces.

### ![Alexa Auto SDK Engine](./assets/number-2.png) Core Module

The Core Module includes Platform Interfaces and runtime Engine support for Core features in the Alexa Auto SDK, such as logging, location, and network information. The services are used by components in other modules and are required by the Alexa Auto SDK.

Click [here](./modules/core/README.md) for more information.

### ![Alexa Auto SDK Engine](./assets/number-3.png) Alexa Module

The Alexa Module includes Platform Interfaces and runtime Engine support for Alexa features in the Alexa Auto SDK. View some [sequence diagrams](./SEQUENCE_DIAGRAMS.md) for more details.

This module includes support for audio input, media playback, template and state rendering, and much more.

Click [here](./modules/alexa/README.md) for more information.

### ![Alexa Auto SDK Engine](./assets/number-4.png) Navigation Module

The Navigation Module includes Platform Interfaces and runtime Engine support for Alexa to interface with the onboard navigation system.

Click [here](./modules/navigation/README.md) for more information.

### ![Alexa Auto SDK Engine](./assets/number-5.png) Phone Control Module

The Phone Control Module includes Platform Interfaces and runtime Engine support for Alexa to interface with the onboard telephony system.

Click [here](./modules/phone-control/README.md) for more information.

## Additional Functionality
The following extensions to the Alexa Auto SDK are available by request. Please contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

### Wake Word extension<a id="wake-word-extension"></a>
Wake Word enables hands-free, voice-initiated interactions with Alexa. The Wake Word extension enables AmazonLite Wake Word support in the Alexa Auto SDK.

### Alexa Communication extension<a id="alexa-communication-extension"></a>
The Alexa Communication extension enables integration with Alexa-to-Alexa calling, Alexa-to-PSTN calling, and messaging capabilities.

### Local Voice Control (LVC) extension<a id="local-voice-control-extension"></a>
The Local Voice Control (LVC) extension provides car control, communication, navigation, and entertainment functionality, with and without an internet connection. It includes components that run an Alexa endpoint inside the vehicle's head unit. Local Voice Control is currently supported on Linux x86 64-bit, Linux ARM 32/64-bit, Android x86 32/64-bit, and Android ARM 32/64-bit platforms.

### Device Client Metrics (DCM) extension<a id="metrics-uploading-extension"></a>
The Device Client Metrics (DCM) extension enables logging and uploading Alexa Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and user perceived latency (UPL) between the request and Alexa’s response. Metrics may be tagged Beta, Gamma, or Prod depending on the vehicle lifecycle. The DCM extension is available for all Linux and Android platforms.

### Voice Chrome extension<a id="voice-chrome-extension"></a>
The Voice Chrome extension adds Voice Chrome support to the Auto SDK for Android x86 32/64-bit and Android ARM 32/64-bit platforms. Voice Chrome provides a consistent set of visual cues representing Alexa attention state across a range of Alexa-enabled devices. The Voice Chrome extension includes a prebuilt Android AAR library for easy integration with your applications, as well as a patch to the Android Sample App that adds the Voice Chrome functionality.


## Getting Started With a Sample App

The [Android Sample App](./samples/android/README.md) and [C++ Sample App](./samples/cpp/README.md) each provide an example of using the Alexa Auto SDK. Their respective READMEs have detailed instructions for setup and usage.

## Vehicle Configuration Information<a id="vehicleconfigurationdatarequirements"></a>

Vehicle information must be supplied to the Alexa Auto SDK in order to pass the certification process. See additional information about vehicle configuration in the [C++](./modules/core/README.md#vehicle-information-requirements) or [Android](./platforms/android/modules/core/readme.md#vehicle-information-requirements) Core module documentation.

## Release Notes and Known Issues<a id="relnotesknownissues"></a>

Feature enhancements, updates, and resolved issues are available to view in the [Alexa Auto SDK Change Log](./CHANGELOG.md)

[Android Sample App](./samples/android/README.md#releasenotes), [C++ Sample App](./samples/cpp/README.md#releasenotes), and [AGL](./platforms/agl/alexa-voiceagent-service/README.md) known issues are documented in their respective READMEs.
