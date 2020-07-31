# Get Started with the Alexa Auto SDK

The [Alexa Auto SDK](./OVERVIEW.md) contains essential client-side software required to integrate Alexa into the automobile. It is modular and abstract, providing a runtime engine for communicating with the Alexa service as well as interfaces that allow you to implement platform-specific behavior such as audio input, media playback, template and state rendering, phone control, and navigation. The Auto SDK also includes two sample applications (one for [C++](./samples/cpp/README.md) and one for [Android](./samples/android/README.md)) that demonstrate how to use the Auto SDK interfaces and allow you to test interactions before integration.

**To get started with the Auto SDK, follow these basic steps:**

> **Important!** You need to register for an [Amazon Developer Account](https://developer.amazon.com/home.html) and [create an Alexa device and security profile](./NEED_HELP.md#registering-a-product-and-creating-a-security-profile) to use the Alexa Auto SDK.

1. Clone the `alexa-auto-sdk` repository into your project and [build the Auto SDK package](./builder/README.md#using-the-auto-sdk-builder).

2. Install the built package on your [target hardware](./builder/README.md#supported-target-platforms).
3. Create and configure an instance of the Engine. For details, see the [C++](./modules/core/README.md#creating-the-engine) or [Android](./platforms/android/modules/core/README.md#creating-the-engine) Core module documentation.
    
4. Extend the Auto SDK interfaces by creating a custom handler for each interface that you want to implement and registering the handler with the Engine. The Auto SDK modules provide platform interfaces and runtime Engine support for different capabilities:

     * **Core** (for [C++](./modules/core/README.md) or [Android](./platforms/android/modules/core/README.md)) - enables you to implement core features such as system audio input and output integration, logging, location, and network information. The services are used by components in other modules and are required by the Alexa Auto SDK.
     * **Alexa** (for [C++](./modules/alexa/README.md) or [Android](./platforms/android/modules/alexa/README.md)) - enables you to implement Alexa features, including speech input and output, Alexa audio playback management, authorization,  equalizer control, template and state rendering, local media sources, alerts, notifications, and Do Not Disturb (DND)). 
     * **Navigation** (for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md)) - enables Alexa to interface with the onboard navigation system.
     * **Phone Call Controller** (for [C++](./modules/phone-control/README.md) or [Android](./platforms/android/modules/phonecontrol/README.md)) - enables Alexa to interface with the onboard telephony system.
     * **Address Book** (for [C++](./modules/address-book/README.md) or [Android](./platforms/android/modules/addressbook/README.md)) - enables you to augment the communications and navigation capabilities of Alexa with user data such as phone contacts and/or navigation favorites.
     * **CBL** (for [C++](./modules/cbl/README.md) or [Android](./platforms/android/modules/cbl/README.md)) - enables you to implement the Code-Based Linking (CBL) mechanism of acquiring Login with Amazon (LWA) access tokens.
     *  **Alexa Presentation Language (APL)** (for [C++](./modules/apl/README.md) or [Android](./platforms/android/modules/apl/README.md)) - enables your application to support APL directives and events for devices with graphical user interface (GUI) support. 
     *  **Car Control** (for [C++](./modules/car-control/README.md) or [Android](./platforms/android/modules/car-control/README.md)) - enables your application to build a custom vehicle-control experience that allows the user to voice-control vehicle features using Alexa.
     *  **Messaging** (for [C++](./modules/car-control/README.md) or [Android](./platforms/android/modules/messaging/README.md)) - enables your application to interface with the Short Message Service (SMS) capabilities of a connected phone using Alexa.
5. Start the Engine using the `start()` command.
6. Use the Sample App ([C++](./samples/cpp/README.md) or [Android](./samples/android/README.md)) to see how the Auto SDK works and to test end-to-end functionality. 

[Additional functionality](./OVERVIEW.md#optional-extensions) (for example, Car Control, SMS, AmazonLite Wake Word, Alexa Communications, Local Voice Control (LVC), Device Client Metrics (DCM), and Voice Chrome for Android) is available with [help from your Amazon Solutions Architect (SA) or Partner Manager](./NEED_HELP.md#requesting-additional-functionality-whitelisting).

## Release Notes and Known Issues <a id="relnotesknownissues"></a>

For a summary of feature enhancements, updates, and resolved and known issues, see the the Auto SDK [Change Log](./CHANGELOG.md).

>**Note:** [Android Sample App](./samples/android/README.md#v210-release-notes), [C++ Sample App](./samples/cpp/README.md#v210-release-notes), and [Automotive Grade Linux (AGL) Alexa Voice Agent](./platforms/agl/alexa-voiceagent-service/README.md#v210-release-notes) release notes are provided in their respective READMEs.

## Migration Guide
For guidelines on migrating Auto SDK versions, see the [Migration Guide](./MIGRATION.md).

## Learn More

For Auto SDK API documentation, see the interface reference documentation:

* [Alexa Auto SDK for Android](https://alexa.github.io/alexa-auto-sdk/docs/android/)
* [Alexa Auto SDK for C++](https://alexa.github.io/alexa-auto-sdk/docs/cpp/)

For UX design requirements, recommendations, and best practices, see the [Alexa Automotive design documentation](https://developer.amazon.com/docs/alexa-auto/about-this-guide.html).

## Important Considerations <a id="important-considerations"></a>

### Security Best Practices <a id="security-best-practices"></a>

All Alexa products are required to follow the [Security Best Practices for Alexa](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html). When building an Alexa experience using the Alexa Auto SDK, additionally adhere to the following security principles:

* Protect configuration files for the Auto SDK Engine from tampering and inspection.
* Protect configuration parameters, such as those found in Auto SDK Engine configuration files, from tampering and inspection, including but not limited to the following: SQLite database files, Unix Domain Sockets, wake word models, and metrics sink files.
* Protect components used for the Local Voice Control (LVC) extension, including associated LVC language model packages (Linux) and APKs (Android), from tampering and inspection, including but not limited to the following: Unix Domain Sockets, model directories, skill and service executables, prompts and assets JSON files, and all files configuring these components. 
* Follow additional SDK security best practices outlined [here](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/overview.html#security-best-practices).