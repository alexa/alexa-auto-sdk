# Alexa Auto SDK

Alexa Auto SDK contains essential client-side software required to integrate Alexa into the automobile. The Alexa Auto SDK provides a runtime engine for communicating with the Alexa service. It also provides interfaces that allow the developer to implement platform specific behavior such as audio input, media playback, template and state rendering, and phone control. It also includes a sample app that demonstrates how to use the Alexa Auto SDK interfaces.

**Table of Contents**:

* [What is Included](#whatsincluded)
* [General Build Requirements](#generalbuildreqs)
* [Vehicle Configuration Information](#vehicleconfigurationdatarequirements)
* [Build the Alexa Auto SDK](#buildthesdk)
* [Alexa Auto SDK Architecture](#architecture)
* [Release Notes and Known Issues](#relnotesknownissues)

> **Tip**: Try looking at [Need Help?](./NEED_HELP.md) if you don't see the topic you are looking for.

## What is Included<a id="whatsincluded"></a>

The following components are included with the Alexa Auto SDK:

* [Builder](./builder/README.md): This directory contains the scripts that allow developers to build modules for a variety of hardware targets.

* Modules: This directory contains the Alexa Auto SDK interfaces and source code in the following modules:
     * [Alexa](./modules/alexa/README.md)
     * [Core](./modules/core/README.md)
     * [Navigation](./modules/navigation/README.md)
     * [Phone Control](./modules/phone-control/README.md)
     * [Contact Uploader](./modules/contact-uploader/README.md)

* [Android Sample App](./samples/android/README.md): This directory contains the Android sample application that uses the Alexa Auto SDK to demonstrate end-to-end functionality.

Additional functionality is available with help from your Amazon Solutions Architect (SA) or Partner Manager:
* [Wake Word support](#wake-word-extension)
* [Alexa Communications](#alexa-communication-extension)
* [Local Voice Control](#local-voice-control-extension)

Alexa Auto SDK API Reference documentation is available for Android and C++. Access the Alexa Auto SDK API reference documentation from the links below.

* [Alexa Auto SDK for Android](https://alexa.github.io/aac-sdk/docs/android/)
* [Alexa Auto SDK for C++](https://alexa.github.io/aac-sdk/docs/cpp/)

> **Important!** You need an [Amazon Developer Account](https://developer.amazon.com/home.html) to use the Alexa Auto SDK. Read the [Sample App](./samples/android/README.md) documentation for more details.

## General Build Requirements<a id="generalbuildreqs"></a>

You can build on a Linux, Unix, or macOS host of your choice.

However, we recommend and support running a Docker environment with the following configuration.

* macOS Sierra or Ubuntu 16.04 LTS
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use.

### Build Dependencies and License Information

During the build time, the following dependencies are fetched and built for the target platform by [Alexa Auto SDK Builder](./builder/README.md). Please refer to each of the individual entities for the particular licenses.

* [AVS Device SDK v1.9](https://github.com/alexa/avs-device-sdk/)
  * [cURL 7.58.0](https://curl.haxx.se/)
  * [ngHTTP2 1.31.1](https://github.com/nghttp2/nghttp2)
  * [SQLite3 3.20.0](https://www.sqlite.org/)
  * *Android target only* [OpenSSL 1.1.0h](https://www.openssl.org/)
* [Google Test v1.8.0](https://github.com/google/googletest)
* [libopus 1.2.1](https://opus-codec.org/)
* [OpenEmbedded-Core Rocko](https://www.openembedded.org/wiki/OpenEmbedded-Core)
* [BitBake](https://www.yoctoproject.org/software-overview/)
* *Android target only* [Android Toolchain](https://www.android.com/)
  * [NDK r16b](https://developer.android.com/ndk/)
  * [SDK Tools 26.0.1](https://developer.android.com/studio/releases/sdk-tools)

> **Note**: that *OpenEmbedded-Core* will fetch and build additional components for preparing the dedicated toolchain for your environment (Such as *GNU Binutils*). Please refer to the [Yocto project](https://www.yoctoproject.org/software-overview/) to understand how it works.


## Vehicle Configuration Information<a id="vehicleconfigurationdatarequirements"></a>

The Vehicle Configuration class provides an interface for vehicle information to be supplied to the SDK. This information is required to pass the certification process.

Read detailed information about the vehicle configuration class in the API reference documentation.

* [Alexa Auto SDK for Android](https://alexa.github.io/aac-sdk/docs/android/)
* [Alexa Auto SDK for C++](https://alexa.github.io/aac-sdk/docs/cpp/)

## Build the Alexa Auto SDK<a id="buildthesdk"></a>

To build the Alexa Auto SDK you should read and understand the instructions in the [Builder README](./builder/README.md).

The recommended and tested host operating systems are **Ubuntu 16.04 LTS** or **macOS Sierra**.

### Supported Target Platforms

Alexa Auto SDK is supported on the following platforms:

* Android 5.1 Lollypop API Level 22 or higher.
    * ARM 32-bit
    * ARM 64-bit
    * x86
    * x86 64-bit
* QNX 7.0
    * ARM 64-bit
    * x86 64-bit

> **Note**: For Android targets, Alexa Auto SDK is available as a prebuilt library on GitHub. Read the instructions about accessing and using the library in the [samples/android/ directory](./samples/android/README.md).

## Alexa Auto SDK Architecture<a id="architecture"></a> Overview

The Alexa Auto SDK is designed to allow flexibility in the integration of Alexa as a part of the automotive in-cabin experience. The Alexa Auto SDK Engine provides runtime support for Alexa and other services and allows the application to integrate with platform specific functionality. Platform specific behavior in Alexa Auto is abstracted into interfaces called "Platform Interfaces." These interfaces should be extended in the application and registered with the Engine.

The Platform Interfaces are documented in their respective module's API documentation.

> **Note**: This diagram shows only some of the available platform interfaces.

![architecture](./assets/aac_architecture.png)

### ![Alexa Auto SDK Engine](./assets/number-1.png) Alexa Auto SDK Engine

The runtime implementation of the Alexa Auto SDK is known as the Engine. Modules extend the Engine by providing services and defining the runtime behavior for Platform Interfaces that are registered by application. The application software communicates with the Engine through the API defined by the Platform Interfaces.

Click [here](./builder/README.md) for more information.

### ![Alexa Auto SDK Engine](./assets/number-2.png) Core Module

The Core Module includes Platform Interfaces and runtime Engine support for Core features in the Alexa Auto SDK, such as logging, location, and network information. The services are used by components in other modules and are required by Alexa Auto SDK.

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

### Wake Word Extension
Wake Word enables hands-free, voice-initiated interactions with Alexa. The Wake Word extension enables AmazonLite Wake Word support in the Alexa Auto SDK.

### Alexa Communication Extension
The Alexa Communication extension enables integration with Alexa-to-Alexa calling, Alexa-to-PSTN calling, and messaging capabilities.

### Local Voice Control Extension
The Local Voice Control extension provides car control functionality like climate control with and without an internet connection. It includes components that run an Alexa endpoint inside the vehicle's head unit. Local Voice Control is currently supported on Android 6.0.

## Getting Started With the Android Sample App

The Android Sample App provides an example of how to use the Alexa Auto SDK. The [Android Sample App README](./samples/android/README.md) has detailed instructions about how to use the sample app.

## Release Notes and Known Issues<a id="relnotesknownissues"></a>

> **Note**: Feature enhancements, updates, and resolved issues from previous releases are available to view in [CHANGELOG.md](./CHANGELOG.md)

### v1.4.0 released on 2018-12-17:

#### Enhancements

Alexa Auto SDK now includes the following enhancement:

* The Alexa Auto SDK now supports the Local Voice Control extension. The Local Voice Control extension enhances the Alexa Auto experience by providing voice based car controls whether connected to the internet or not. In this release, the Local Voice Control extension will provision access only to the car’s climate control

> **Note**: This extension is available on request - Please contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

#### Resolved Issues

No resolved issues.

#### Known Issues

* The Alexa Auto SDK does not re-discover Media Apps Command and Control (MACC) compliant apps if they are unresponsive after being idle for a long period(around 30 minutes).
* The Alexa Auto SDK Engine becomes unresponsive if it receives PLAY directive during shutdown. However since shutdown is triggered when car ignition is turned off, there is not direct customer impact expected.
* When a timer sounds during an Alexa to Alexa call, uttering "stop" ends the call not the timer.
* Multiple automotive devices using the same account at the same time, can access contacts from phones paired across those devices.

Sample App issues are documented in the [Sample App README](./samples/android/README.md#androidsampleapprelnote).
