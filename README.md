# Overview of the Alexa Auto SDK

The Alexa Auto SDK contains essential client-side software required to integrate Alexa into the automobile. The Auto SDK provides libraries that connect to Alexa and expose C++ and Java interfaces for your vehicle software to implement the platform-specific behavior for audio input, media streaming, calling through a connected phone, turn-by-turn navigation, controlling vehicle features such as heaters and lights, and more. You can use the included sample applications, one for C++ and one for Android, to learn about the Auto SDK interfaces and to test interactions before integration.

<!-- omit in toc -->
## Table of Contents
- [Auto SDK Architecture](#auto-sdk-architecture)
- [Auto SDK Modules and Extensions](#auto-sdk-modules-and-extensions)
- [Alexa Auto Client Service (AACS)](#alexa-auto-client-service-aacs)
- [Security Best Practices](#security-best-practices)
- [See Also](#see-also)

## Auto SDK Architecture

The following architecture diagram illustrates a common design used for integrating the Auto SDK into the vehicle software. 

<img src="./assets/aac_architecture.png" />
</p>
</details>
The following sections describe the relationships among components in the architecture.

### Auto SDK Engine
The Engine is a system of components that provide the runtime implementation of the Auto SDK. The main program of your application or background service creates an instance of the Engine and configures the instance, registers platform interface handlers, and manages its lifecycle. When started by the main program, the Engine maintains a connection to Alexa, manages runtime execution states, and provides the underlying implementation of the functionality of the platform interfaces.

### Platform Interfaces
Platform interfaces are abstract interfaces provided by the Auto SDK for you to implement the platform-specific functionality of the Auto SDK integration. “Platform-specific functionality” refers to components of the integration that interact with the hardware, operating system, underlying software frameworks, or external libraries. Each platform interface defines an API for the application to interact with the Engine for a particular component, such as audio input or location services. The Engine invokes a registered platform interface “handler” when it needs to query data or delegate handling, such as rendering visual elements or placing a phone call, to your custom implementation. The handler invokes the Engine to provide a callback to a request from the Engine or provide a proactive notification of a state change.

### Handlers
Bridging the Engine and other processes running in the head unit, a handler implements the functionality required by the platform interface it extends. The implementation of a handler may include using an event bus, platform-specific inter-process communication (IPC) mechanisms, direct implementations with system libraries, or deep integrations with existing applications.

## Auto SDK Modules and Extensions
The Auto SDK is organized into logically related groups of functionality called “modules,” which enable you to select only the features you want to include in your integration. Each module includes “Platform” and “Engine” libraries. The Platform library includes the platform interfaces and configuration options required for a feature, and the Engine library augments the base functionality of the Engine with the underlying implementation of the feature. 

>**Note:** The libraries of each module are written in C++, but building the Auto SDK for an Android target enables an Android version of the modules that provide Java wrappers on the C++ interfaces for easier use.

The following sections describe the modules included in the Auto SDK. Modules not downloadable with the Auto SDK from GitHub are available as extensions, which you can obtain with help from your Amazon Solutions Architect (SA) or Partner Manager.

### Core Module

The Core module (for [C++](./modules/core/README.md) or [Android](./platforms/android/modules/core/README.md)) provides the infrastructure for audio input and output, authorization, logging, location reporting, metrics, property management, network monitoring services, local storage, and vehicle information services. The infrastructure is necessary for any module that provides platform interfaces (for example, the Alexa module). 

### Alexa Module

The Alexa module (for [C++](./modules/alexa/README.md) or [Android](./platforms/android/modules/alexa/README.md)) supports Alexa features such as speech input and output, authorization, volume control, media playback, equalizer control, template and state rendering, local media sources, alerts, notifications, and do not disturb. 

### Navigation Module

The Navigation module (for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md)) provides support for Alexa to interface with the onboard navigation system.

### Phone Call Controller Module

The Phone Call Controller module (for [C++](./modules/phone-control/README.md) or [Android](./platforms/android/modules/phonecontrol/README.md)) provides support for Alexa to interface with the onboard telephony system.

### Address Book Module

The Address Book module (for [C++](./modules/address-book/README.md) or [Android](./platforms/android/modules/addressbook/README.md)) augments the communications and navigation capabilities of Alexa with user data such as phone contacts and navigation favorites ("home", "work", etc.).

### Code-Based Linking (CBL) Module

The CBL module (for [C++](./modules/cbl/README.md) or [Android](./platforms/android/modules/cbl/README.md)) implements the CBL mechanism of acquiring Login with Amazon (LWA) access tokens. For information about the CBL mechanism, see the [Code-Based Linking documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/code-based-linking-other-platforms.html).

### Alexa Presentation Language (APL) Module
The APL module (for [C++](./modules/apl/README.md) or [Android](./platforms/android/modules/apl/README.md)) enables devices to support a visual Alexa experience.

>**Note:** The [APL Render module](./platforms/android/modules/apl-render/README.md) is provided to enable APL rendering capabilities in an Android application. 

### Messaging Module 
The Messaging module (for [C++](./modules/messaging/README.md) or [Android](./platforms/android/modules/messaging/README.md)) provides support for Short Message Service (SMS) capabilities of Alexa such as sending and reading text messages.

### Car Control Module
The Car Control module (for [C++](./modules/car-control/README.md) or [Android](./platforms/android/modules/car-control/README.md)) enables your application to build a custom vehicle-control experience that allows the user to voice-control vehicle features using Alexa.

### Connectivity Module
The Connectivity module (for [C++](./modules/connectivity/README.md) or [Android](./platforms/android/modules/connectivity/README.md)) creates a lower data consumption mode for Alexa, allowing automakers to offer tiered functionality based on the status of their connectivity plans.

### Text To Speech (TTS) Module
The TTS module (for [C++](./modules/text-to-speech/README.md) or [Android](./platforms/android/modules/text-to-speech/README.md)) enables a platform implementation to request synthesis of Alexa speech on demand from a text or Speech Synthesis Markup Language (SSML) string.

### Text To Speech (TTS) Provider Module
The TTS provider module (for [C++](./modules/text-to-speech-provider/README.md) or [Android](./platforms/android/modules/text-to-speech-provider/README.md)) synthesizes Alexa speech on demand. This module requires Auto SDK to be built with the Local Voice Control extension.
  
### AmazonLite Wake Word Extension
Wake Word enables hands-free, voice-initiated interactions with Alexa. The Wake Word extension enables AmazonLite Wake Word support in the Auto SDK.

### Alexa Communications Extension
The Alexa Communications extension enables integration with Alexa-to-Alexa calling, Alexa-to-PSTN calling, and messaging capabilities.

### Alexa Custom Assistant Extension
The Alexa Custom Assistant extension provides the functionality for toggling the settings of Alexa and the automaker's voice assistant, and notifies the IVI system at runtime about updates to the acting assistant for a specific interaction.

### Bluetooth Extension
The Bluetooth extension allows the Auto SDK to connect to devices through the Bluetooth Classic or Bluetooth Low Energy (BLE) protocol. Using these protocols, the Auto SDK can offer Bluetooth-based features to users of Android or iOS smartphones.

### Device Client Metrics (DCM) Extension
The Device Client Metrics (DCM) extension enables logging and uploading Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and user perceived latency (UPL) between the request and Alexa’s response.

### Geolocation Extension
The Geolocation extension adds geolocation consent support to the Auto SDK, enabling the user to grant consent to location sharing with Alexa from your application.

### Local Voice Control (LVC) Extension
The LVC extension provides car control, communication, navigation, local search, and entertainment functionality, without an internet connection. It includes components that run an Alexa endpoint inside the vehicle's head unit.

#### Local Navigation Module
The Local Navigation module enables you to provide customers with Alexa local search and navigation without WiFi or data plan connectivity.

### Mobile Authorization Extension
The Mobile Authorization extension enables applications running on the vehicle's head unit to simplify the login experience. To log in to Alexa, the user uses the Alexa mobile app on a paired smartphone instead of opening a web browser and entering a code.

### Voice Chrome for Android Extension
The Voice Chrome extension adds Voice Chrome support to the Auto SDK for Android x86 64-bit and Android ARM 32/64-bit platforms. Voice Chrome provides a consistent set of visual cues representing Alexa attention state across a range of Alexa-enabled devices. The Voice Chrome extension includes a prebuilt Android AAR library for easy integration with your applications, as well as a patch to the Android Sample App that adds the Voice Chrome functionality.

## Alexa Auto Client Service (AACS)
AACS simplifies the process of integrating the Auto SDK in Android-based devices. After you install, configure, and initialize AACS, it communicates with the applications, providing an interface between the applications and various Alexa functions, such as navigation and car control. You can also include AACS as an Android archive (AAR) in the application if you don't want to run AACS as a separate app. For more information about AACS, see the AACS [README](./platforms/android/alexa-auto-client-service/README.md).

AACS requires the Alexa Auto Service Bridge (AASB) extension, which provides a message-based interface to the Auto SDK Engine.  For more information about AASB, see the [AASB README](./extensions/aasb/README.md).

## Security Best Practices

All Alexa products are required to follow the [Security Best Practices for Alexa](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html). When building an Alexa experience using the Auto SDK, additionally adhere to the following security principles:

* Protect configuration files for the Auto SDK Engine from tampering and inspection.
* Protect configuration parameters, such as those found in Auto SDK Engine configuration files, from tampering and inspection, including but not limited to the following: SQLite database files, Unix Domain Sockets, wake word models, and metrics sink files.
* Protect components used for the Local Voice Control (LVC) extension, including associated LVC language model packages (Linux) and APKs (Android), from tampering and inspection, including but not limited to the following: Unix Domain Sockets, model directories, skill and service executables, prompts and assets JSON files, and all files configuring these components. 
* Your C++ implementation of Auto SDK interfaces must not retain locks, crash, hang, or throw exceptions.
* Use exploit mitigation flags and memory randomization techniques when you compile your source code to prevent vulnerabilities from exploiting buffer overflows and memory corruptions. 

## See Also
The following documents or websites provide more information about the Auto SDK.

* [In-vehicle Alexa experience design guidelines](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/about-this-guide.html) include principles, voice, visual, user interface (UI) patterns, and multimodal best practices. 
* [Getting Started Guide](./GETSTARTED.md) provides steps for getting started with the Auto SDK and for downloading extensions.
* [Change Log](./CHANGELOG.md) provides a summary of feature enhancements, updates, and resolved and known issues. 
* [Android Sample App](./samples/android/README.md) and [C++ Sample App](./samples/cpp/README.md) READMEs provide release notes about the sample apps.
* For Auto SDK API documentation, see the interface reference documentation:
    * [Alexa Auto SDK for Android](https://alexa.github.io/alexa-auto-sdk/docs/android/)
    * [Alexa Auto SDK for C++](https://alexa.github.io/alexa-auto-sdk/docs/cpp/)
* [Migration Guide](./MIGRATION.md) describes how to migrate from one Auto SDK version to another.

