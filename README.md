## Alexa Automotive Core SDK (Developer Preview)

**Alexa Automotive Core (AAC)** is a C++ SDK specifically targeted for the automotive domain. It contains essential client-side software (that uses [Alexa Voice Services Devices SDK](https://alexa.github.io/avs-device-sdk/index.html)) required to integrate with [Alexa Voice Services (AVS)](https://developer.amazon.com/alexa-voice-service) and other Amazon services into the automobile. In addition to the existing AVS APIs, AAC also exposes certain automotive-specific platform APIs that need to be implemented by the platform developers, documentation, build scripts, tools, and a sample application that allows developers to get started quickly with integrating AAC SDK into their respective platforms.

### What is included

In this version the following components are included:

* [Builder](./builder): `builder/`: This directory contains the scripts that allow developers to build modules for variety of hardware platform targets.

* [Modules](./modules): `modules/`: This directory contains the multiple libraries that allows the application to access various Amazon services within your application.

* [Android Sample App](./samples/android): `samples/android/`: This directory contains the Android sample application that uses the Android Platform Support and AAC SDK to demonstrate end to end functionality.

* [Android API Docs](./docs/): `docs/`: This contains a local copy of the AAC API class references in HTML format. Open  `docs/android.html` in your Browser to view the Android API documentation.


### Getting Started

In the Developer Preview version of the software we are only supporting Android target (API Level 22 or higher). All other targets are preliminary and will be fully supported in the future. To get started, you will need a host PC running **Ubuntu Linux** (16.04 LTS or later) or **macOS** (10.12 Sierra or later).

#### **Prerequisites**

For macOS hosts, you need the [Homebrew](https://brew.sh/) packages listed below.

```
$ brew install binutils coreutils findutils
```

#### **Android targets**

1. Follow the instructions on the [Builder](./builder) page to build modules for Android.
2. Use the [Android Sample App](./samples/android) to learn how to use the AAC SDK.

>**Note:** The Google Android Emulator does not support microphone access and hence cannot be used with Alexa. You can use an Android tablet, phone or other Android device that supports microphone input.

#### **Generic Linux targets (future)**

See [Alexa Auto Core Builder](./builder/README.md) to build modules for Generic Linux targets.

#### **QNX 7 targets (future)**

See [Alexa Auto Core Builder](./builder/README.md) to build modules for QNX7 targets.

#### **macOS targets (future)**

Follow the instructions on [Building Alexa Auto Core with CMake](./builder/ConfigureCMake.md) to build modules for macOS targets. Note that you must use a macOS host PC to proceed.

### Architecture <a id="architecture"></a>

![architecture](./assets/aac_architecture.png)

### Engine <a id="engine"></a>

The runtime implementation of the AAC SDK is known as the Engine, which can be found in the [Core Module](./platforms/android/CORE.md). The application software communicates with the Engine through the API defined by the Platform Interfaces.

Any application that wants to use the Engine needs to follow these steps:
* Create an instance of the Engine at startup.
* Configure the Engine instance by passing in a list of `EngineConfiguration` objects required to initialize the Engine Services.
* Platform Interfaces are then registered by calling the `registerPlatformInterface()` method for each interface that is used.
* Finally, the `start()` method is called - effectively starting the Engine.
* When the application terminates, the Engine's `stop()` method is called to shutdown all of the running services and release any resources that are being used by the Engine.

### Modules <a id="modules"></a>

The AAC SDK is divided into individual modules which represent separate groups of one or more services and libraries that handle a feature category. These build into independent libraries. In this verison of the SDK we have provided three modules. [Core](./platforms/android/CORE.md) and [Alexa](./platforms/android/ALEXA.md) are required modules in the current implementation of the AAC SDK, and [Navigation](./platforms/android/NAVIGATION.md) is an option if the platform supports it.

#### **Core Module**

The Core Module is always required, as it provides all the base classes and service managers for the Engine.
* [C++ API](./modules/core/README.md)
* [Android API](./platforms/android/CORE.md)

#### **Alexa Module**

The Alexa Module is also a required module as it provides access to the AVS Device SDK and the implementation of some extra steps to sequence events and directive handling. The  platform developer can focus on just using the provided API to interact with AVS. This is done by registering platform interfaces via the Engine object.
* [C++ API](./modules/alexa/README.md)
* [Android API](./platforms/android/ALEXA.md)

#### **Navigation Module**

The Navigation module provides the platform interfaces to handle Alexa navigation events and directives and is dependent on the Alexa Module.
* [C++ API](./modules/navigation/README.md)
* [Android API](./platforms/android/NAVIGATION.md)

### Platform Interfaces <a id="platforminterfaces"></a>
Platform interfaces are the responsibility of application developer (in this case the head unit software developer) as this is platform dependent and cannot be generically implemented in the SDK for all platforms. The way to implement this is by extending the platform interfaces that are made available in the various modules described above. By default, the Engine handles the AVS Device SDK directives and events. (Certain device-specific directives like Display Cards, and events generated by the platform such as when media playback is finished, are left to the platform implementation to handle directly.)

### Release Notes and Known Issues

#### **v1.0.0 Beta**

* v1.0.0 Beta released on 2018-04-29
* Supports [AVS Devices SDK v1.4.0](https://github.com/alexa/avs-device-sdk/releases/tag/v1.4)

#### **Enhancements**

The following enhancements were added to the AAC SDK since the last Alpha release (binary).

 * `SetDestination()` API added to the Navigation module (see the [Modules](#modules) section above)
 * Android Sample Application updated with a number of features such as rendering of Display Cards (Shopping List, Coffee Shops Nearby, etc), handling of the `SetDestination()` API, Notifications, LWA (Login with Amazon)

#### **Known Issues**

***SDK:***
* While the SDK does build against Android API22 and above and runs successfully on Android O devices, our current testing shows a native-code linking error when actually running on API22 devices.

***Android Sample App:***
* M3U and PLS based stream URLs are not parsed before sent to the Android Mediaplayer. Affects live streams typically coming from TuneIn and IHeartRadio services
* Media playback can take a long time to start sometimes for iHeartRadio and TuneIn
* The Android AAC Sample App was developed on an Android tablet with 2048 x 1536 resolution screen size. It can run on smaller devices, but some of the display cards may not display correctly
* During Playing Media in the Sample App we see the following messages (none of these will cause any issues):
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=streamFormat
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=progressReportDelayInMilliseconds
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=expectedPreviousToken
  * E/AAC:aace.alexa.AudioChannelEngineImpl:validateSource:reason=invalidSource
  * E/AAC:aace.alexa.AudioChannelEngineImpl:pause:reason=invalidSource,expectedState=X
* On App startup we see the following messages (none of these will cause any issues):
   * E/AVS:SQLiteAlertStorage:openFailed::File specified does not exist.:file path=/data/user/0/com.amazon.sampleapp/cache/appdata/alerts.sqlite
* Several minor documentation issues which will be address in the GA release

