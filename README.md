# Overview of the Alexa Auto SDK

The Alexa Auto SDK contains essential client-side software required to integrate Alexa into the automobile. The Auto SDK provides libraries that connect to Alexa and expose interfaces for your vehicle software to implement the platform-specific behavior for audio input, media streaming, calling through a connected phone, turn-by-turn navigation, controlling vehicle features such as heaters and lights, and more. You can use the included sample application to learn about the Auto SDK interfaces and to test interactions before integration.

The contents of this repository are distributed under several different license agreements. Please refer to the [LICENSE](./LICENSE) file for the license terms applicable to the materials that you are using.

<!-- omit in toc -->
## Table of Contents
- [Auto SDK Architecture](#auto-sdk-architecture)
- [Getting Started](#getting-started)
- [Auto SDK Integration](#auto-sdk-integration)
- [Security Best Practices](#security-best-practices)
- [See Also](#see-also)

## Auto SDK Architecture

The Auto SDK is modular, with a system of components that provide the runtime implementation of the Auto SDK. Each module exposes interfaces to handle specific functionality such as audio input and output, authorization, media streaming, navigation and controlling vehicle features. Most of the modules are included in the Auto SDK. Modules not downloadable with the Auto SDK from GitHub are available as extensions, which you can obtain with help from your Amazon Solutions Architect (SA) or Partner Manager. See [here](./SDK_MODULES.md) for more information on Auto SDK modules and extensions.

## Getting Started

### Prerequisites
Complete the following steps before you get started with the Auto SDK:

1. Register for an [Amazon Developer Account](https://developer.amazon.com/home.html) and [create an Alexa device and security profile](./NEED_HELP.md#registering-a-product-and-creating-a-security-profile) to use the Auto SDK.
2. Make sure that you meet the requirements for building the Auto SDK and understand the dependencies, as described in the SDK builder [README](./builder/README.md).  

### Build Auto SDK
Follow these steps to get started with the Auto SDK:

1. Clone the `alexa-auto-sdk` repository into your project.
2. If you want to use the optional Auto SDK modules, download the modules from the locations listed below.
	* [AmazonLite Wake Word extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Amazonlite%20Extension)

	* [Alexa Communications extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Alexa%20Comms%20Extension)

	* [Local Voice Control extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Local%20Voice%20Control%20Extension)

	* [Device Client Metrics (DCM) extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Metric%20Upload%20Service%20Extension)

	* [Voice Chrome for Android extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Voice%20Chrome%20Extension)

	>The version of the optional extension archive must match the version of the Auto SDK that you are using. For example, if you are using Auto SDK 3.0 and want to install the Local Voice Control extension, you must download version 3.0 of the Local Voice Control extension archive.

	>**Note:** The Alexa Presentation Language (APL) module is provided publicly, but requires [additional packages](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Alexa%20Auto%20SDK%20Alexa%20Presentation%20Language%20module) to be downloaded to successfully build.
	
3. Build the Auto SDK as described in the builder [README](./builder/README.md).
      
The following section provides the details of integrating with Android and Linux based platforms:

## Auto SDK Integration

### Integrating Auto SDK in Android-based platforms

The Alexa Auto Client Service (AACS) simplifies the process of integrating the Auto SDK in Android-based devices. AACS is an Alexa Auto SDK feature packaged in a stand alone Android application package (APK) or in an Android archive library (AAR). After you install, configure, and initialize AACS, it communicates with the applications, providing an interface between the applications and various Alexa functions, such as navigation and car control. You can also include AACS as an Android archive (AAR) in the application if you do not want to run AACS as a separate app. 

[Learn more >>](./aacs/android/README.md)

### Integrating Auto SDK in Linux-based platforms

The Alexa Auto Service Bridge (AASB) simplifies the process of integrating the Auto SDK in Linux-based devices. AASB framework provides a Message Broker API that transmits JSON messages between the OEM application and the Auto SDK. This API can be used publish and subscribe to AASB messages to implement the platform-specific functionality of the Auto SDK integration.

[Learn more >>](./LINUX_INTEGRATION.md)

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
* [Change Log](./CHANGELOG.md) provides a summary of feature enhancements, updates, and resolved and known issues. 
* AASB message definition provides the reference documentation for AASB messages. You can find AASB message definition for each module [here.](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/index.html) by navigating to "Modules" tab and select a module on the left menu. For example, you can find AASB message definition for Address Book module [here.](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/address-book/aasb-docs/AddressBook/index.html)
* [AACS Sample App](./aacs/android/README.md) and [C++ Sample App](./samples/cpp/README.md) READMEs provide information about the sample apps. This helps you test interactions before integration.
* [Migration Guide](./MIGRATION.md) describes how to migrate from one Auto SDK version to another.
