# Alexa Automotive Core SDK

**Alexa Automotive Core (AAC)** SDK contains essential client-side software required to integrate Alexa into the automobile. AAC provides a runtime engine for communicating with the Alexa service. It also provides interfaces that allow the developer to implement platform specific behavior such as audio input, media playback, template and state rendering, and phone control. It also includes a sample app that demonstrates how to use the AAC interfaces.

**Table of Contents**:
* [What is Included](#whatsincluded)
* [General Build Requirements](#generalbuildreqs)
* [Build the AAC SDK](#buildthesdk)
* [AAC SDK Architecture](#architecture)
* [Release Notes and Known Issues](#relnotesknownissues)

> **Tip**: Try looking at [Need Help?](./NEED_HELP.md) if you don't see the topic you are looking for.

## What is Included<a id="whatsincluded"></a>

The following components are included with the AAC SDK:

* [Builder](./builder/README.md): This directory contains the scripts that allow developers to build modules for a variety of hardware targets.

* [Modules](./modules): This directory contains the AAC interfaces and source code.

* [Android Sample App](./samples/android/README.md): This directory contains the Android sample application that uses the AAC SDK to demonstrate end-to-end functionality.

AAC API Reference documentation is available for Android and C++. Go to the appropriate directory and open the **`index.html`** file with a web browser.

Choose a directory:

* [.../aac/docs/android/](./docs/android/)
* [.../aac/docs/cpp/](./docs/cpp/)

## General Build Requirements<a id="generalbuildreqs"></a>

You can build on a Linux, Unix, or macOS host of your choice.

However, we recommend and support running a Docker environment with the following configuration.  

* macOS Sierra or Ubuntu 16.04 LTS
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use.



## Build the AAC SDK<a id="buildthesdk"></a>

To build the AAC SDK you should read and understand the instructions in the [Builder README](./builder/README.md).

The recommended and tested Linux hosts are **Ubuntu 16.04 LTS** or **macOS Sierra**.

### Supported Target Platforms

AAC SDK is supported on the following platforms:

* Android ARM 32-bit
* Android ARM 64-bit (using 32-bit build)
* Android x86
* QNX ARM 64-bit

> **Note**: For Android targets, AAC is available as a prebuilt library on GitHub. Read the instructions about accessing and using the library in the [samples/android/ directory](./samples/android/README.md). 

## AAC SDK Architecture<a id="architecture"></a> Overview

The AAC SDK is designed to allow flexibility in the integration of Alexa as a part of the automotive in-cabin experience. The AAC Engine provides runtime support for Alexa and other services and allows the application to integrate with platform specific functionality. Platform specific behavior in AAC is abstracted into interfaces called "Platform Interfaces." These interfaces should be extended in the application and registered with the Engine.

The Platform Interfaces are documented in their respective module's API documentation.

>**Note**: This diagram shows only some of the available platform interfaces.

![architecture](./assets/aac_architecture.png)

### ![AAC Engine](./assets/number-1.png) AAC Engine

The runtime implementation of the AAC SDK is known as the Engine. Modules extend the Engine by providing services and defining the runtime behavior for Platform Interfaces that are registered by application. The application software communicates with the Engine through the API defined by the Platform Interfaces.

Click [here](./builder/README.md) for more information.

###  ![AAC Engine](./assets/number-2.png) Core Module

The Core Module includes Platform Interfaces and runtime Engine support for Core features in the AAC SDK, such as logging, location, and network information. The services are used by components in other modules and are required by AAC.

Click [here](./modules/core/README.md) for more information.

### ![AAC Engine](./assets/number-3.png) Alexa Module

The Alexa Module includes Platform Interfaces and runtime Engine support for Alexa features in the AAC SDK. View some [sequence diagrams](./SEQUENCE_DIAGRAMS.md) for more details.

This module includes support for  audio input, media playback, template and state rendering, and much more.

Click [here](./modules/alexa/README.md) for more information.

### ![AAC Engine](./assets/number-4.png) Navigation Module

The Navigation Module includes Platform Interfaces and runtime Engine support for Alexa to interface with the onboard navigation system.

Click [here](./modules/navigation/README.md) for more information.

### ![AAC Engine](./assets/number-5.png) Phone Control Module

The Phone Control Module includes Platform Interfaces and runtime Engine support for Alexa to interface with the onboard telephony system.

Click [here](./modules/phone-control/README.md) for more information.

## Getting Started With the Android Sample App

The Android Sample App provides an example of how to use AAC. The [Android Sample App README](./samples/android/README.md) has detailed instructions about how to use the sample app.

## Release Notes and Known Issues<a id="relnotesknownissues"></a>

>Note: Feature enhancements, updates, and resolved issues from previous releases are available to view in [CHANGELOG.md](./CHANGELOG.md)

### v1.0.1 released on 2018-07-31:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues

Issues fixed in this release:

* The Engine now reconnects to AVS when the `NetworkInfoProvider` updates the network status.
* All shared memory objects are now freed when the Engine object is disposed.
* We fixed a media playback state issue in the Engine that caused an unexpected pause and resume for a media stream that was already stopped.
* We added AudioPlayer::playerActivityChanged to the Android APIs.
* Updated the `AuthError` enumeration with additional error types.
* Removed deprecated `createAuthConfig()` configuration method.
* Fixed issue in the JNI where trying to create a UTF-8 string with invalid characters caused a crash, seen when sensitive logging is enabled.
* Improved JNI thread handling.
* Enabled capability registration for phone call control.
* We fixed an issue where the Android platform build failed on the initial attempt when using clean code.

### Known issues

There are no known issues in this release.

Sample App issues are documented in the [Sample App README](./samples/android/README.md#androidsampleapprelnote).
