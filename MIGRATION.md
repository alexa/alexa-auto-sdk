# Alexa Auto SDK Migration Guide

This guide outlines the changes you need to make to migrate from Auto SDK v2.0 to later versions of the Auto SDK.

>**Note:** If you migrate from a version earlier than v2.3, be sure to read the relevant sections of this guide to understand all changes introduced between your current version and v3.1. The information helps you decide what changes you must include. For example, if you migrate from v2.0, include the changes described in [Migrating from Auto SDK v2.0 to v2.1](#migrating-from-auto-sdk-v20-to-v21), the changes described in [Migrating from Auto SDK v2.1 to v2.2](#migrating-from-auto-sdk-v21-to-v22), and so on, taking into consideration the deprecated or removed features in each version.

<!-- omit in toc -->
## Table of Contents

- [Migrating from Auto SDK v3.0.0 to v3.1.0](#migrating-from-auto-sdk-v300-to-v310)
  - [Migrating to the Authorization Platform Interface](#migrating-to-the-authorization-platform-interface)
- [Deprecated Features Removed in Auto SDK v3.0.0](#deprecated-features-removed-in-auto-sdk-v300)
  - [Using the Address Book Module](#using-the-address-book-module)
- [Migrating from Auto SDK v2.2.1 to v2.3.0](#migrating-from-auto-sdk-v221-to-v230)
  - [Car Control Enhancements and Breaking Changes](#car-control-enhancements-and-breaking-changes)
  - [Language Model Packaging](#language-model-packaging)
  - [Android](#android)
  - [Clang Formatting](#clang-formatting)
- [Migrating from Auto SDK v2.2 to v2.2.1](#migrating-from-auto-sdk-v22-to-v221)
  - [TemplateRuntime Enhancements](#templateruntime-enhancements)
- [Migrating from Auto SDK v2.1 to v2.2](#migrating-from-auto-sdk-v21-to-v22)
  - [Implementing the Property Manager Interface](#implementing-the-property-manager-interface)
  - [Car Control Changes](#car-control-changes)
- [Migrating from Auto SDK v2.0 to v2.1](#migrating-from-auto-sdk-v20-to-v21)
  - [Build Changes](#build-changes)
  - [Engine Configuration File Updates](#engine-configuration-file-updates)
  - [Navigation Enhancements](#navigation-enhancements)
  - [Car Control Source File Relocation](#car-control-source-file-relocation)
  - [Code-Based-Linking (CBL) Handler in the Sample Apps](#code-based-linking-cbl-handler-in-the-sample-apps)

## Migrating from Auto SDK v3.0.0 to v3.1.0
This section provides the information you need to migrate from Auto SDK v3.0.0 to Auto SDK v3.1.0.

### Migrating to the Authorization Platform Interface

Auto SDK v3.1.0 introduces the Authorization module that provides a single platform interface to manage different types of authorizations supported by the Engine. This single platform interface works with the Engine services that carry out the actual authorization process or flow. For more information about how authorization works, see the [Core module README](./modules/core/README.md). This section provides the information you need for migrating to the Authorization platform interface from the CBL or AuthProvider platform interface, which are deprecated in v3.1.0

**Migrating from the CBL Platform Interface**

To migrate from the CBL platform interface to the Authorization platform interface, follow the instructions in the [CBL README](./modules/cbl/README.md), which describes the Authorization APIs for CBL authorization. 

The Engine notifies the application of any errors during the authorization process via the `authorizationError` API. The errors reported when you use the Authorization platform interface are different from the ones reported with the CBL platform interface, as shown in the following table:

| CBL                 | Authorization            | Description                                      |
|-----------------------|----------------------------|--------------------------------------------------|
| ERROR                 | UNKNOWN_ERROR              |Unknown error occurs during the authorization flow.     |
| TIMEOUT               | TIMEOUT                    |Request for the the CBL code from LWA times out.      |
| CODE_PAIR_EXPIRED     | CODE_PAIR_EXPIRED          |The code pair obtained has expired.               |
| AUTHORIZATION_EXPIRED | AUTHORIZATION_EXPIRED      |Refresh token is expired or revoked.              |
|             | START_AUTHORIZATION_FAILED |Authorization fails to start.                     |
|             | LOGOUT_FAILED              |Logout fails.                                     |

**Migrating from the AuthProvider Platform Interface**

To migrate from the AuthProvider platform interface to the Authorization platform interface, follow the instructions in the [Alexa README](./modules/alexa/README.md), which describes the Authorization APIs for Auth Provider authorization. 

The Engine notifies the application of any errors during the authorization process via the `authorizationError` API. The errors reported when you use the Authorization platform interface are different from the ones reported with the AuthProvider platform interface, as shown in the following table:

| AuthProvider        | Authorization              | Description                                      |
|---------------------|----------------------------|--------------------------------------------------|
| authFailure         | AUTH_FAILURE               |Invalid or expired access token was provided.     |
| NOT PRESENT         | UNKNOWN_ERROR              |Unknown error occurs during the authorization flow.     |
| NOT PRESENT         | START_AUTHORIZATION_FAILED |Authorization fails to start.  |
| NOT PRESENT         | LOGOUT_FAILED              |Logout fails.            |

## Deprecated Features Removed in Auto SDK v3.0.0
* The following asset IDs for Car Control have been removed: "Alexa.Automotive.DeviceName.DriverSeat", "Alexa.Automotive.DeviceName.LeftSeat", "Alexa.Automotive.DeviceName.PassengerSeat", "Alexa.Automotive.DeviceName.RightSeat".
* The `createControl()` method has been removed. Use `createEndpoint()` instead. 
* Support for the "isMemberOf" relationship for endpoint definition has been removed. You must list member endpoints in a zone definition.
* Implicit zone definitions have been removed.
* The following `TemplateRuntime` methods have been removed:
  * The `renderTemplate(const std::string& payload)` method has been removed. Use renderTemplate(const std::string& payload, FocusState focusState) instead.
  * The `renderPlayerInfo(const std::string& payload)` method has been removed. Use `renderPlayerInfo(const std::string& payload, PlayerActivity audioPlayerState, std::chrono::milliseconds offset, FocusState focusState)` instead.
* In the Alexa module, `AlexaProperties::SUPPORTED_LOCALES` has been removed. For Alexa to recognize the locale setting, specify one of these values: de-DE, en-AU, en-CA, en-GB, en-IN, en-US, es-ES, es-MX, es-US, fr-CA, fr-FR, hi-IN, it-IT, ja-JP, pt-BR.
* `Engine::setProperty()` and `Engine::getProperty()` have been removed. Use `PropertyManager::setProperty()` and `PropertyManager::getProperty()` instead. For details about the Property Manager platform interface, see "Managing Runtime Properties with the Property Manager" ([for C++](./modules/core/README.md#managing-runtime-properties-with-the-property-manager) or [for Android](./platforms/android/modules/core/README.md#managing-runtime-properties-with-the-property-manager)).
* The `SpeechRecognizer::enableWakeWordDetection()`, `SpeechRecognizer::disableWakeWordDetection()`, and `SpeechRecognizer::isWakewordDetectionEnabled()` methods have been removed.
* The Contact Uploader module has been removed. Use the [Address Book module](#using-the-address-book-module) instead.

### Using the Address Book Module
Address Book module enables the user to upload contacts from the phone that is paired with the car or the navigation favorites from the car head unit to Alexa cloud. For more information about how this module works, see the [Address Book README](./modules/address-book/README.md). Both the Android and C++ sample apps demonstrate the use of the `AddressBook` platform interface. See the sample app source code for specific implementation details.

The following Address Book API descriptions help you transition from the Contact Uploader module to the Address Book module:
 
`addAddressBook`

```
    bool addAddressBook(const std::string& addressBookSourceId, const std::string& name, AddressBookType type);
```

Use `addAddressBook` instead of `ContactUploader::addContactsBegin`. In addition, `addAddressBook` requires you to specify the source id to identify the address book, the friendly name of the address book, and the type of address book.
 
`removeAddressBook`
```
    bool removeAddressBook(const std::string& addressBookSourceId);
```
    
Use `removeAddressBook` instead of `ContactUploader:: removeUploadedContacts`. You must specify the id of the address book to be removed.
 
`getEntries`
```
    bool getEntries(
            const std::string& addressBookSourceId,
            std::weak_ptr<IAddressBookEntriesFactory> factory)
```
    
When using the Address Book module, the Engine pulls the address book contents from the platform implementation. You must upload the address book contents through the factory class, `IAddressBookEntriesFactory`, for the specified address book source id.
  
## Migrating from Auto SDK v2.2.1 to v2.3.0

This section outlines the changes you will need to make to migrate from Auto SDK v2.2.1 to Auto SDK v2.3.

### Car Control Enhancements and Breaking Changes

Read the updated Car Control module README (for [C++ platforms](./modules/car-control/README.md) or [Android](./platforms/android/modules/car-control/README.md)) to get a complete understanding of all supported features and the current format of the "aace.carControl" configuration schema. Read the updated API documentation for the `CarControlConfiguration` builder class (for [C++ platforms](./modules/car-control/platform/include/AACE/CarControl/CarControlConfiguration.h) or [Android](./platforms/android/modules/car-control/src/main/java/com/amazon/aace/carControl/CarControlConfiguration.java)) if you construct your configuration programmatically. The changes to the "aace.carControl" configuration for v2.3 are backward-compatible, meaning your previous configuration (regardless of whether it was file-based or built programmatically with the `CarControlConfiguration` class) will still compile and produce a valid configuration to input to Auto SDK. However, several updates are recommended to ensure expected behavior, even if you do not want new features.

#### 1. Zones configuration schema update 

Prior to v2.3, to assign an endpoint to exactly one zone, you would specify an "isMemberOf" relationship in the definition of the endpoint and specify no information about endpoints in the zone definition.

```jsonc
{
    "endpointId": "all.fan",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.DeviceName.Fan"
                }
            }
        ]
    },
    "capabilities": [
        ...
    ],
    "relationships": {
        "isMemberOf": {
            "zoneId": "zone.all"
        }
    }
}

...

{
    "zoneId": "zone.all",
    "zoneResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.Location.All"
                }
            }
        ]
    }
}
```

In 2.3, the "isMemberOf" relationship is removed from endpoint definitions so that endpoints need not belong to zones and the zone definition can be the source of truth for all its member endpoints. The zone definition now includes a list of member endpoints:

```jsonc
{
    "endpointId": "all.fan",
    "endpointResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.DeviceName.Fan"
                }
            }
        ]
    },
    "capabilities": [
        ...
    ]
}

...

{
    "zoneId": "zone.all",
    "zoneResources": {
        "friendlyNames": [
            {
                "@type": "asset",
                "value": {
                    "assetId": "Alexa.Automotive.Location.All"
                }
            }
        ]
    },
    "members" : [
      {
        "endpointId": "all.fan"
      },
      ...
    ]
}
```
You should update your configuration accordingly. The Auto SDK Engine translates the old format to the new format internally, but this will be deprecated in later versions. When updating to the new format, you must *not* combine usage of the "isMemberOf" format with the "members" list format. Fully migrate all definitions in your configuration.

#### 2. Deprecated implicit creation of zone definitions

If you construct your configuration programmatically with the `CarControlConfiguration` builder class, your implementation prior to v2.3 might not have explicitly specified definitions for the set of zones considered "official", but you still used them in your endpoint configurations anyway. The builder class added these definitions to the "aace.carControl" configuration automatically without requiring you to call `CarControlConfiguration::createZone()`. In v2.3, `CarControlConfiguration` still includes this logic for the old "official" zones, but it does not implicitly create any new zones, and it is recommended to define every zone you use by calling `CarControlConfiguration::createZone()`. Implicit zone definitions will be removed in a later version.

#### 3. New default zone feature

Specifying a "default" zone ID is an optional new feature, but it is highly recommended that you use it. See the Car Control module README for details about why this feature is important.

#### 4. Deprecated "DriverSeat" and related assets in favor of zones

Prior to v2.3, the default automotive catalog of assets introduced several asset IDs so that online-only systems could mock zones support for heaters on seat endpoints. The asset IDs are the following: `Alexa.Automotive.DeviceName.DriverSeat`, `Alexa.Automotive.DeviceName.LeftSeat`, `Alexa.Automotive.DeviceName.PassengerSeat`, `Alexa.Automotive.DeviceName.RightSeat`.

 Now that the cloud supports zones, you must stop using these asset IDs and properly model the endpoints using zones so that Alexa resolves user utterance intents as expected. These assets will be removed in a later version of Auto SDK. See the Car Control module README for sample configuration.

#### 5. New default assets

The Car Control module is updated to include many new assets in the default automotive catalog to support a wider range of utterances. If you previously defined custom assets to support any of the features introduced to the v2.3 assets, it is recommended that you use the new default assets instead of your previous custom ones. See the Car Control module README for details about assets.

#### 6. Reset your account when changing from 2.2 to 2.3 configuration

It is a known issue that you cannot delete any previously configured endpoint IDs associated with your customer account in the cloud. When upgrading your configuration from v2.2 to v2.3, contact your SA or Partner Manager for help to reset your account's endpoint database in the cloud. This is especially important if you are updating to use new features. It is also recommended that your v2.3 configuration follows the configuration sample of supported features shown in the Car Control README. Refer to this document for reference.

### Language Model Packaging

Language models for the Local Voice Control extension are now decoupled from the LVC.sh (Linux) binaries. If you use the Local Voice Control extension, you must install the language models to successfully migrate to v2.3.0. Download the language model tar files. Installation instructions are provided in the Local Voice Control extension.

### Android

#### Gradle
The gradle plugin has been updated to v3.6.2. This requires gradle v5.6.4 or above in order to build the Auto SDK for Android targets.

#### Sample App
The Android sample app supports overriding the client configuration by pushing a file named app_config.json to the /sdcard folder on the device. If the /sdcard/app_config.json file existed on the device before you migrate to v2.3.0, the file overrides the client configuration included in the v2.3.0 Android sample app APK.

### Clang Formatting
Auto SDK code has been formatted with `clang-format` version 9.0.0. This may lead to merge conflicts if changes have been made to v2.2.1 source code files and you migrate to v2.3.

## Migrating from Auto SDK v2.2 to v2.2.1
This section outlines the changes you will need to make to migrate from Auto SDK v2.2 to Auto SDK v2.2.1.

### TemplateRuntime Enhancements
Auto SDK v2.2.1 introduces additional TemplateRuntime platform interface features that you can integrate in your application to enrich the user's experience with Now Playing cards for AudioPlayer and ExternalMediaPlayer implementations. [Now Playing cards](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html#renderplayerinfo) are a form of display cards — visual aids that complement the Alexa voice experience — that contain media metadata, player controls and album art delivered in the TemplateRuntime RenderPlayerInfo directive.

Migration is only required to support the new features, but is highly recommended for the following reasons:
1. AudioPlayer and TemplateRuntime are now decoupled.
2. The following TemplateRuntime methods are now deprecated:
   1. The `renderTemplate( const std::string& payload )` method is deprecated. Use `renderTemplate( const std::string& payload, FocusState focusState )` instead.
   2. The `renderPlayerInfo( const std::string& payload )` method is deprecated. Use `renderPlayerInfo( const std::string& payload, PlayerActivity audioPlayerState, std::chrono::milliseconds offset, FocusState focusState )` instead.

#### renderTemplate

**Method**
```
renderTemplate( const std::string& payload,
                FocusState focusState )
```
The new renderTemplate method provides visual metadata associated with a user request to Alexa. The platform implementation should parse the template metadata and render a display card for the user.

**Parameters**
- `payload` Renderable template metadata in structured JSON format
- `focusState` The `FocusState` of the channel used by TemplateRuntime interface
  - `FOREGROUND` Represents the highest focus a Channel can have
  - `BACKGROUND` Represents the intermediate level focus a Channel can have

#### renderPlayerInfo

**Method**
```
renderPlayerInfo( const std::string& payload,
                  PlayerActivity audioPlayerState,
                  std::chrono::milliseconds offset,
                  FocusState focusState )
```

The new renderPlayerInfo method provides visual metadata associated with a user request to Alexa for audio playback. The platform implementation should parse the player info metadata and render a display card for the user. The `audioPlayerState` and `offset` are useful for implementing the progress bar in the display card. It is assumed that the client is responsible for progressing the progress bar when the `AudioPlayer` is in `PLAYING` state.

**Parameters**
- `payload` Renderable player info metadata in structured JSON format
- `audioPlayerState` The state of the `AudioPlayer`
  - `IDLE` Audio playback has not yet begun
  - `PLAYING` Audio is currently playing
  - `STOPPED` Audio playback is stopped, either from a stop directive or playback error
  - `PAUSED` Audio playback is paused
  - `BUFFER_UNDERRUN` Audio playback is stalled because a buffer underrun has occurred
  - `FINISHED` Audio playback is finished
- `offset` The offset in millisecond of the media that `AudioPlayer` is handling
- `focusState` The `FocusState` of the channel used by TemplateRuntime interface
  - `FOREGROUND` Represents the highest focus a Channel can have
  - `BACKGROUND` Represents the intermediate level focus a Channel can have

#### Sample Apps

The Android Sample App demonstrates the new features in `TemplateRuntimeHandler.java` in GUI form. Refer to sample app source code and [Alexa Voice Service documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html#renderplayerinfo) for specific implementation details.

The C++ Sample App simply demonstrates the new features by printing `audioPlayerState`, `offset`, and `focusState` to the console in the `TemplateRuntimeHandler::renderPlayerInfo()` method of `TemplateRuntimeHandler.cpp`.

## Migrating from Auto SDK v2.1 to v2.2
This section outlines the changes you will need to make to migrate from Auto SDK v2.1 to Auto SDK v2.2.

### Implementing the Property Manager Interface
Auto SDK v2.2 introduces the Property Manager, a component that maintains runtime properties by storing property values and listeners and delegating the `setProperty()` and `getProperty()` calls from your application to the respective Engine services. The Engine invokes the PropertyManager platform interface method `propertyChanged()` to notify your application about property value changes originating internally. The property values may be set by Auto SDK modules that define constants (for example `FIRMWARE_VERSION` and `LOCALE`), or they may be initiated from the Alexa Voice Service (AVS), such as when the user changes the `TIMEZONE` setting in the Alexa Companion App.

`PropertyManager::setProperty()` and `PropertyManager::getProperty()` replace deprecated `Engine::setProperty()` and `Engine::getProperty()`. For details about the Property Manager platform interface, see "Managing Runtime Properties with the Property Manager" ([for C++](./modules/core/README.md#managing-runtime-properties-with-the-property-manager) or [for Android](./platforms/android/modules/core/README.md#managing-runtime-properties-with-the-property-manager)).

### Car Control Changes
This section documents the changes you will need to make to migrate your Car Control implementation to Auto SDK v2.2.

#### New Asset ID Prefix
The asset ID prefix for default assets has been changed from `"Alexa."` to `"Alexa.Automotive."`. This change requires a code or configuration change only if your existing car control implementation uses the `CarControlConfiguration` configuration builder with the literal strings of asset IDs. If your existing car control implementation uses the predefined constants in `CarControlAssets.h` or `CarControlAssets.java`, then no change is required.

#### Specifying the Path to Custom Car Control Assets
If your implementation using the Local Voice Control (LVC) extension uses custom assets for car control, you must specify the path to the custom assets in both the `aace.carControl` Auto SDK car control configuration and the LVC configuration, not just the LVC configuration as in Auto SDK v2.0.

* **For C++ implementations:** The default LVC configuration for Linux expects any custom assets to be defined in a file called `assets.json` located at `/opt/LVC/data/led-service/assets/assets.json`. Use this path when you configure the `assets.customAssetsPath` field in the Auto SDK car control configuration, or provide a path to an assets file with equivalent content.
* **For Android implementations:** The file at the path you provide in the `assets.customAssetsPath` field of the Auto SDK car control configuration must be the same as the custom assets file you configure for your `ILVCClient` using the LVC APK.

#### Car Control Config Builder Asset Methods

Two new `CarControlConfiguration` methods are now implemented in the Engine:

* `CarControlConfiguration::addCustomAssetsPath()`
* `CarControlConfiguration::addDefaultAssetsPath()`

> **Note:** These methods were also present in Auto SDK v2.1; however they didn't function as designed. They have been updated to function correctly in Auto SDK v2.2.

This implementation populates the `"aace.carControl"` configuration object with the
`"assets.customAssetsPath"` and `"assets.defaultAssetsPath"` nodes.


## Migrating from Auto SDK v2.0 to v2.1
This section outlines the changes you will need to make to migrate from Auto SDK v2.0 to Auto SDK v2.1.

### Build Changes
The following build changes have been introduced in Auto SDK v2.1:

* The builder script usage has changed for Linux targets. All Linux targets now use the same platform name (`linux`), and `-t <target>` is mandatory. For example, to build for a Linux native target, use:

    `builder/build.sh linux -t native`
    
    to build for Linux native, pokyarm, and pokyarm64 targets, use:
    
    `builder/build.sh linux -t native,pokyarm,pokyarm64`

    See the [Builder README](./builder/README.md#running-builder) for details about supported platforms and targets.

* For QNX targets, you must cross-compile with the QNX multimedia software for the system audio extension (which is built by default for QNX targets). This requires a QNX Multimedia Suite license. See the [System Audio extension README](./extensions/experimental/system-audio/README.md) for details.

### Engine Configuration File Updates

The AVS Device SDK portion of the Auto SDK Engine configuration (the `aace.alexa.avsDeviceSDK` node) has been updated. See the [`config.json.in`](./samples/cpp/assets/config.json.in) file for details.

  * The `"deviceInfo"` node includes two new elements: `"manufacturerName"` and `"description"`.
  * A path to the capabilities database is now required. Use the `"capabilitiesDelegate"` element to specify this path.
  * The `"settings"` element has changed to `"deviceSettings"`, and it includes these changes:
    * The default locale setting has been moved from `"defaultAVSClientSettings/locale"` to `"defaultLocale"`.
    * `"deviceSettings"` now requires a `"defaultTimezone"`.


### Navigation Enhancements

Auto SDK v2.1 introduces additional navigation features that you can integrate in your application to enrich the user's experience: add/cancel a waypoint, show/navigate to a previous destination, turn and lane guidance, and map display control. Implementing these enhancements required deprecating the `setDestination()` interface in favor of the `startNavigation()` interface and adding several additional interfaces.

To migrate from Auto SDK v2.0 to Auto SDK v2.1, you must update your platform implementation to use the `startNavigation()` method instead of the `setDestination()` method, modify the payload for the `getNavigationState()` method, and implement the new navigation methods. This guide takes you through these steps. Please see the Navigation module README for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md) for additional information and resources.

#### What's New

The following abstract methods have been added to the Navigation platform interface:

* `startNavigation()`
* `showPreviousWaypoints()`
* `navigateToPreviousWaypoint()`
* `showAlternativeRoutes()`
* `controlDisplay()`
* `announceManeuver()`
* `announceRoadRegulation()`

The following methods have been added as well:

* `navigationEvent()`
* `navigationError()`
* `showAlternativeRoutesSucceeded()`

The following method has been removed from the Navigation module:

* `setDestination()`

The following method now returns a different payload:

* `getNavigationState()`

#### Implementing the New Navigation Features

To implement the new navigation features, follow these steps:

**STEP 1:** Replace the `setDestination()` method with the `startNavigation()` method.

The payload meaning of `startNavigation()` is different than that of deprecated `setDestination()`. `setDestination()` corresponded to adding a destination to the navigation system context. `startNavigation()`, on the other hand, corresponds to using the route information provided in the payload to start navigation, with one or more waypoints. In response to `startNavigation()`, your implementation should also call either the `navigationEvent()` method or the `navigationError()` method.

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
// NavigationHandler.java
@Override
*public void *startNavigation( String payload ) {
    ...
    // success
    navigationEvent( EventName.NAVIGATION_STARTED );
    ...
    // failure
    navigationError( ErrorType.NAVIGATION_START_FAILED, ErrorCode.INTERNAL_SERVICE_ERROR, "" );
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
// NavigationHandler.cpp
void NavigationHandler::startNavigation(const std::string& payload ) {
    ...
    // success
        navigationEvent( aace::navigation::NavigationEngineInterface::EventName::NAVIGATION_STARTED );
    ...
    // failure
        navigationError( aace::navigation::NavigationEngineInterface::ErrorType::NAVIGATION_START_FAILED, aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR, "" );    
```
</p>
</details>
    
**STEP 2:** Modify the payload for the `getNavigationState()` method.

The functionality of the `getNavigationState()` and `cancelNavigationState()` methods is unchanged from Auto SDK v2.0, but the `getNavigationState()` payload has changed. The NavigationState context has been updated to contain more information than in Auto SDK v2.0.

The `address` field has been updated from a string to the following object:

```
...
"address": {
       "addressLine1": "{{STRING}}", //Address line 1
       "addressLine2": "{{STRING}}", //Address line 2
       "addressLine3": "{{STRING}}", //Address line 3
       "city": "{{STRING}}", //City
       "districtOrCounty": "{{STRING}}", //district or county
       "stateOrRegion": "{{STRING}}", //state or region
       "countryCode": "{{STRING}}", //3 letter country code
       "postalCode": "{{STRING}}", //postal code
},
...
```
The `name` field has been added to the waypoint payload:

```
"name": "{{STRING}}", // name of the waypoint such as home or Starbucks
```

<details><summary>Here is an <strong>example of a full NavigationState context payload</strong> - click to expand or collapse</summary>
<p>

```
...,
{
  "header": {
    "namespace": "Navigation",
    "name": "NavigationState"
  },
  "payload": {
    "state": "{{STRING}}", //NAVIGATING or NOT_NAVIGATING
    "waypoints": [
        {
            "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM  
            "estimatedTimeOfArrival": {
                "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
                "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
             },
            "address": {
                   ++ "addressLine1": "{{STRING}}", //Address line 1
                   ++ "addressLine2": "{{STRING}}", //Address line 2
                   ++ "addressLine3": "{{STRING}}", //Address line 3
                   ++ "city": "{{STRING}}", //City
                   ++ "districtOrCounty": "{{STRING}}", //district or county
                   ++ "stateOrRegion": "{{STRING}}", //state or region
                   ++ "countryCode": "{{STRING}}", //3 letter country code
                   ++ "postalCode": "{{STRING}}", //postal code
             },
            ++ "name": "{{STRING}}", // name of the waypoint such as home or Starbucks
             "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
        },
        {
            "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM
            "estimatedTimeOfArrival": {
                "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
                "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
             },
            "address": {
                  ++  "addressLine1": "{{STRING}}", //Address line 1
                  ++  "addressLine2": "{{STRING}}", //Address line 2
                  ++  "addressLine3": "{{STRING}}", //Address line 3
                  ++  "city": "{{STRING}}", //city
                  ++  "districtOrCounty": "{{STRING}}", //district or county
                  ++  "stateOrRegion": "{{STRING}}", // state or region
                  ++  "countryCode": "{{STRING}}", //3 letter country code
                  ++  "postalCode": "{{STRING}}", // postal code
             },
           ++ "name": "{{STRING}}", // name of the waypoint such as home or Starbucks
            "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
            "pointOfInterest": {
                "id": "{{STRING}}", //POI lookup Id vended from Alexa 
                "hoursOfOperation": [
                 {
                     "dayOfWeek": "{{STRING}}",
                     "hours": [
                      {
                         "open": "{{STRING}}", // ISO-8601 time with timezone format
                         "close": "{{STRING}}" // ISO-8601 time with timezone format
                      }
                    ],
                    "type": "{{STRING}}" // Can be: OPEN_DURING_HOURS, OPEN_24_HOURS, etc.
                }
                ],
                "phoneNumber": "{{STRING}}"
             }
             
        },
        ...
      ],
     "shapes": [
            [
              {{LATITUDE_DOUBLE}},
              {{LONGITUDE_DOUBLE}}
            ],
            [
              {{LATITUDE_DOUBLE}},
              {{LONGITUDE_DOUBLE}}
            ],
        ...      
     ]   
  }
}
...,
```
</p>
</details>

**STEP 3:** Implement the new navigation abstract methods.

The new navigation methods are all called in response to navigation-based user utterances such as “show my previous route” or “what’s the speed limit here?”. At a minimum, your implementation should report a `navigationError()` to inform the user when the navigation system does not support that information.

>**Note:** The `navigationEvent()`, `showAlternativeRoutesSucceeded()` and `navigationError()` methods have been implemented in the Auto SDK but are not yet implemented on the cloud side. Sending the events will not affect navigation functionality, but the Alexa cloud will return an `INVALID_REQUEST_EXCEPTION` or `INVALID_SERVICE_EXCEPTION` until these events are implemented on the cloud side.  

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
@Override
public void showPreviousWaypoints() {
    //handle showing information about previous waypoints...
    navigationError( ErrorType.SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode.NOT_SUPPORTED, *""** *);
}
...
    
@Override
public void navigateToPreviousWaypoint() {
    //handle navigation to previous waypoint
    navigationError( ErrorType.PREVIOUS_NAVIGATION_START_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void showAlternativeRoutes( AlternateRouteType alternateRouteType ) {
    //pass AlternateRouteType enum
    navigationError( ErrorType.DEFAULT_ALTERNATE_ROUTES_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void controlDisplay ( ControlDisplay controlDisplay ) {
    //pass ControlDisplay enum
    navigationError( ErrorType.ROUTE_OVERVIEW_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void announceManeuver( String payload  ) {
    //pass the JSON string payload from AnnounceManeuver directive
    navigationError( ErrorType.TURN_GUIDANCE_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void announceRoadRegulation( RoadRegulation roadRegulation ) {
    //pass RoadRegulation enum
    navigationError( ErrorType.SPEED_LIMIT_REGULATION_FAILED, ErrorCode.NOT_SUPPORTED, "" );
} 
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
void NavigationHandler::showPreviousWaypoints() {
        //handle showing information about previous waypoints...
        navigationError( aace::navigation::Navigation::ErrorType.SHOW_PREVIOUS_WAYPOINTS_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, ""* *);
   }
    ...
    
    void NavigationHandler::navigateToPreviousWaypoint() {
        //handle navigation to previous waypoint
        navigationError( aace::navigation::Navigation::ErrorType.PREVIOUS_NAVIGATION_START_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::showAlternativeRoutes( aace::navigation::Navigation::AlternateRouteType alternateRouteType ) {
        //pass AlternateRouteType enum
        navigationError( aace::navigation::Navigation::ErrorType.DEFAULT_ALTERNATE_ROUTES_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::controlDisplay ( aace::navigation::Navigation::ControlDisplay controlDisplay ) {
        //pass ControlDisplay enum
        navigationError( aace::navigation::Navigation::ErrorType.ROUTE_OVERVIEW_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::announceManeuver( String payload  ) {
        //pass the JSON string payload from AnnounceManeuver directive
        navigationError( aace::navigation::Navigation::ErrorType.TURN_GUIDANCE_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::announceRoadRegulation( aace::navigation::Navigation::RoadRegulation roadRegulation ) {
        //pass RoadRegulation enum
        navigationError( aace::navigation::Navigation::ErrorType.SPEED_LIMIT_REGULATION_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    } 
```
</p>
</details>

#### New TemplateRuntime Interface Version

The Auto SDK now implements version 1.2 of the TemplateRuntime interface to handle display card templates. If you support TemplateRuntime in your implementation, you must update your implementation to support the new card types.

The TemplateRuntime interface remains the same, but the `LocalSearchListTemplate1` template has been deprecated in favor of the new `LocalSearchListTemplate2` template. In addition, two new templates (`TrafficDetailsTemplate` and `LocalSearchDetailTemplate1`), are now supported. The `TrafficDetailsTemplate` includes commute information to favorite destinations such as home or work. The `LocalSearchDetailTemplate1` template includes information about specific locations or information in response to users asking for details about locations presented in the `LocalSearchListTemplate2` template. For details about the TemplateRuntime interface, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html). For details about implementing TemplateRuntime in your Auto SDK implementation see the Alexa module README for [C++](./modules/alexa/README.md#handling-display-card-templates) or [Android](./platforms/android/modules/alexa/README.md#handling-display-card-templates).


### Car Control Source File Relocation

The Car Control module platform interface files and documentation are now located in `aac-sdk/modules/car-control` for C++ and `aac-sdk/platforms/android/modules/car-control` for Android, rather than in the Local Voice Control (LVC) extension directory structure.

>**Note:** In addition, if you use custom assets for car control in an implementation with the optional Local Voice Control (LVC) extension, you must specify the path to the custom assets in both the Auto SDK car control configuration and the LVC configuration, not just the LVC configuration. For details, see [Path to Custom Car Control Assets for LVC Implementations](#path-to-custom-car-control-assets-for-lvc-implementations).

### Code-Based-Linking (CBL) Handler in the Sample Apps 
Both of the Auto SDK Sample Apps now include the Code-Based Linking (CBL) handler implementation (in favor of the `AuthProvider` handler implementation ) to handle obtaining access tokens from Login with Amazon (LWA). Changing from the `AuthProvider` handler to the CBL handler is *not a required change*, but we recommend that you use the Auto SDK CBL interface for ease of implementation. For details about the CBL handler, please see the CBL module README [for C++](./modules/cbl/README.md) or [for Android](./platforms/android/modules/cbl/README.md).

If you want to continue using the `AuthProvider` interface, we recommend that you implement the new `onAuthFailure()` method that exposes 403 "unauthorized request" exceptions from Alexa Voice Service (AVS). This method may be invoked, for example, when your product makes a request to AVS using an access token obtained for a device which has been deregistered from the Alexa companion app. In the Sample Apps, you can override the  interface and unset your login credentials as if the user had done so with your GUI interface: 

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
@Override
    public void authFailure( String token ) {
        // handle user de-authorize scenario 
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
void AuthProviderHandler::authFailure( const std::string& token ) {
        // handle user de-authorize scenario 
```
</p>
</details>
