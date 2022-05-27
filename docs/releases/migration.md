# Auto SDK Migration Guide

## Overview

This guide highlights the changes in each Auto SDK version that require your application to update to maintain compatibility. The guide outlines the changes with step-by-step recommendations to help you stay up-to-date with the latest Auto SDK version. Each section describes an increment of one release, so if you skip intermediate versions when you upgrade, ensure you follow the steps in each section from your current version to the latest version. 

## Backward compatibility

Auto SDK remains backward compatible across minor version updates; however, to continually improve, Auto SDK sometimes deprecates APIs, configuration fields, and build options in a minor version. The changes this guide outlines in minor version upgrade sections are intended to highlight deprecations and help you stop using deprecated features as soon as possible to prepare for their removal in the next major version.

Although rare, if Auto SDK makes an exception to the backward compatibility tenants in a minor version, this guide explicitly calls out the change.

## Migrating from Auto SDK v4.0.0 to v4.1.0
This section provides the information you need to migrate from Auto SDK v4.0.0 to Auto SDK v4.1.0

### ShowAlternativeRoutesSucceeded savings amount must be float

The field `alternateRoute.savings.amount` in the `Navigation.ShowAlternativeRoutesSucceeded` AASB message (and corresponding deprecated platform interface API `Navigation::showAlternativeRoutesSucceeded`) is a float, but Auto SDK versions 4.0 and earlier incorrectly allowed passing a string for this value. Auto SDK 4.1 fixes the issue, so you must update your code to use a float if your code is also incorrectly using a string.

### LVC App Components replace LVC APK on Android Platform

AACS LVC App Components replace the LVC APK on Android. Auto SDK no longer releases the LVC APK, and the previous LVC APK does not work with 4.1 AACS. The LVC App Components are Android libraries (AARs) that run LVC in the same application as AACS, and the AACS sample app integrates them by default. 

* If your Alexa client application uses the Java platform interfaces (deprecated in 4.0), you are required to update your application to use AACS before integrating with LVC App Components. See [Migrate to the MessageBroker API](./migrate-to-messagebroker.md) and the [AACS documentation](https://alexa.github.io/alexa-auto-sdk/docs/android/) for information about migrating your application.

* If your Alexa client application is AACS-based already and has LVC functionality, the previous implementation based on AIDL interfaces and the LVC APK no longer applies. Specifically,

    * `ILVCClient` and `ILVCService` are removed. Remove your implementation of `ILVCClient` and the binding to the LVC service. Instead, by default, no additional implementation is needed with AACS in 4.1 because AACS starts and configures LVC.
    * You do not need to specify `android:sharedUserId` previously required for inter-application UDS IPC.

For more details about integrating the LVC App Components, see the documentation in the Local Voice Control extension on the Amazon developer portal.

## Migrating from Auto SDK v3.3.0 to v4.0.0
This section provides the information you need to migrate from Auto SDK v3.3.0 to Auto SDK v4.0.0

### Platform Interfaces are deprecated
The C++ and Java platform interfaces are deprecated in favor of Alexa Auto Services Bridge (AASB). Auto SDK 4.0 replaces the platform interfaces with a new `MessageBroker` API for subscribing to and publishing AASB messages. See [`Migrate to the MessageBroker API` ](./migrate-to-messagebroker.md) for information and instructions to migrate your application.

### AASB configuration for AACS is updated

In Auto SDK version 3.3, your application using AACS was required to configure the AASB version with the following `aacs.aasb` object in your AACS configuration file:

```
"aacs.aasb" : {           
    "version": "3.3"      
}
```

Remove this configuration from your 4.0 AACS configuration file.

Additionally, the optional `defaultMessageTimeout` and `autoEnableInterfaces` configuration fields are moved from the `aasb` object to the `messageBroker` object, so you must update your AACS configuration file if you use these fields. For example, if your AACS configuration file includes this block:

```
{
    "aacs.aasb": {
        "autoEnableInterfaces": false,
        "defaultMessageTimeout": 1000
    }
}

```

change it to this:

```
{
    "aacs.messageBroker": {
        "autoEnableInterfaces": false,
        "defaultMessageTimeout": 1000
    }
}
```

## Migrating from Auto SDK v3.2.1 to v3.3.0
This section provides the information you need to migrate from Auto SDK v3.2.1 to Auto SDK v3.3.0 

### Local Media Source and Global Preset Enhancements

#### GlobalPreset is deprecated
The `GlobalPreset` platform interface is deprecated because its feature set is supported by the new `DEFAULT` `LocalMediaSource` type. To preserve functionality for utterances targeting generic presets like "Alexa, play preset 1", implement and register a `LocalMediaSource` handler of `Source::DEFAULT` type. The user utterances that cause the Engine to invoke `GlobalPreset::setGlobalPreset()` will cause the Engine to invoke `LocalMediaSource::play()` with `ContentSelector::PRESET` instead. The `GlobalPreset` platform interface will be removed in a future version of Auto SDK.

#### Additional LocalMediaSource playerEvent calls are needed
Previous Auto SDK documentation stated that you must call `LocalMediaSource::playerEvent()` to report events "PlaybackStated" and "PlaybackStopped" only. Please update your implementation to call `playerEvent()` with states "PlaybackSessionStarted" and "PlaybackSessionEnded" as well. See the Alexa module documentation for information about when to report these events.

#### setFocus is deprecated
The API `LocalMediaSource::setFocus()` is deprecated because its functionality is equivalent to calling `LocalMediaSource::playerEvent()` with event name "PlaybackSessionStarted" when a player is brought into focus or "PlaybackSessionEnded" when a player is removed from focus. Please replace your calls to `setFocus(true)` and `setFocus(false)` with calls to `playerEvent("PlaybackSessionStarted")` and `playerEvent("PlaybackSessionEnded")`, respectively. `setFocus` will be removed in a future version of Auto SDK.

#### Reporting playback session ID is needed
The Alexa cloud requires `ExternalMediaPlayer` events and context for a particular player to include the playback session ID of a player's active session. To support this, the `LocalMediaSource::play()` function signature is updated to include a parameter for session ID for an Alexa-initiated session, which you must use when reporting player events for the player. The `playerEvent` and `playerError` signatures are also updated to include session ID. You must generate your own session ID when the playback is initiated by the user without Alexa. See the Alexa module documentation for more details about the `sessionId`. The versions of APIs without the session ID will be removed in a future version of Auto SDK.

### Migrating the Local Navigation Module APIs
The local search features of the Local Voice Control Extension's Local Navigation module are extended to support offline navigation to addresses, cities, and neighborhoods. To support the new feature set, the existing APIs are updated to a more general name. The changes are backward compatible, but the old APIs are deprecated and will be removed in a future version. Use the following steps to assist the migration to the new APIs:

#### LocalSearchProvider Platform Interface Changes
We have deprecated the functions `poiSearchRequest`, `poiLookupRequest`, `poiSearchResponse`, and `poiLookupResponse` in favor of `searchRequest`, `lookupRequest`, `searchResponse`, and `lookupResponse`, respectively.

* Override `LocalSearchProvider::searchRequest()` instead of `LocalSearchProvider::poiSearchRequest()`.
* Override `LocalSearchProvider::lookupRequest()` instead of `LocalSearchProvider::poiLookupRequest()`.
* Call `LocalSearchProvider::searchResponse()` instead of `LocalSearchProvider::poiSearchResponse()`.
* Call `LocalSearchProvider::lookupResponse()` instead of `LocalSearchProvider::poiLookupResponse()`.

We have also deprecated the AASB messages `PoiSearchRequestMessage`, `PoiLookupRequestMessage`, `PoiSearchResponseMessage`, and `PoiLookupResponseMessage` in favor of `SearchRequestMessage`, `LookupRequestMessage`, `SearchResponseMessage`, and `LookupResponseMessage`, respectively.

* Subscribe to `SearchRequestMessage` instead of `PoiSearchRequestMessage`.
* Subscribe to `LookupRequestMessage` instead of `PoiLookupRequestMessage`.
* Publish to `SearchResponseMessage` instead of `PoiSearchResponseMessage`.
* Publish to `LookupResponseMessage` instead of `PoiLookupResponseMessage`.

The JSON schemas of search and response are still the same.
Note: Do not use/implement a mix of the old APIs and the new APIs

#### Local Navigation Module Engine Configuration Changes
The `aace.localNavigation.localSearch` configuration keys `navigationPOISocketPath` and `poiEERSocketPath` are renamed to `navigationLocalSearchSocketPath` and `localSearchEERSocketPath`, respectively. 
For example, if your configuration was this

```jsonc
{
   "aace.localNavigation": {
        "localSearch": {
            "navigationPOISocketPath": "/opt/LVC/data/poi-er-service/poi_navigation.socket",
            "poiEERSocketPath": "/opt/LVC/data/poi-er-service/poi_eer.socket"
        }
    }
}
```
change it to this

```jsonc
{
   "aace.localNavigation": {
        "localSearch": {
            "navigationLocalSearchSocketPath": "/opt/LVC/data/local-search-er-service/local_search_navigation.socket",
            "localSearchEERSocketPath": "/opt/LVC/data/local-search-er-service/local_search_eer.socket"
        }
    }
}
```

If you are using the `LocalNavigationConfiguration::createLocalSearchConfig()` factory function to generate the configuration, your usage does not have to change because the signature is the same and implementation of this function generates the new JSON.

Note: the socket paths in the Linux default sample configuration file are updated, so if you use different values, ensure you update your LVC app configuration accordingly

#### LVC APK Configuration Changes
If you use LVC on Android, update the configuration returned by your implementation of the interface `ILVCClient.getConfiguration()`.

The paths `NavigationPOISocketDir` and `POIEERSocketDir` have been deprecated in favor of `NavigationLocalSearchSocketDir` and `LocalSearchEERSocketDir`, respectively.
The socket names `NavigationPOISocketName` and `POIEERSocketName` have been deprecated in favor of `NavigationLocalSearchSocketName` and `LocalSearchEERSocketName`, respectively. 

#### LVC Linux App Configuration Changes
The LVC configuration file `lvc-config.json` installed at `/opt/LVC/config` by the installation script `LVC.sh` has no changes to its JSON configuration schema since Auto SDK 3.2. However, the socket directories and names used by default in this file are updated to use more general names.

## Migrating from Auto SDK v3.1.0 to v3.2.0
This section provides the information you need to migrate from Auto SDK v3.1.0 to Auto SDK 3.2.0. All information about 3.2.0 is also applicable to 3.2.1.

### Using the Alexa Communication Extension
The Alexa Comms library in Auto SDK v3.2.0 uses Device Client Metrics (DCM) instead of AWS IoT for uploading metrics. Therefore, remove the `iotCertificateDirPath`, `iotHostAddress`, and `deviceTypeId` fields from the communication configuration file. For information about the configuration file format, see the Alexa Communication extension documentation.

If you build the Alexa Comms module configuration using the programmatic factory function `AlexaCommsConfiguration::createCommsConfig()` (C++) or `AlexaCommsConfiguration.createCommsConfig()` (Java), remove the parameters that are no longer present in the signature.

### Using the Device Client Metrics (DCM) Extension
The Device Client Metrics extension in Auto SDK v3.2.0 requires a field called `metricsTag` to be defined in the DCM configuration. The value of `metricsTag` is used for generating a unique identifier for anonymous registration metrics. 

>**Note:** You must not use the vehicle identification number (VIN) or device serial number (DSN) as `metricsTag`. For information about how to use this field, see the Device Client Metric extension documentation.

If you build the DCM module configuration using the programmatic factory function `DCMConfiguration::createDCMConfig()` (C++) or `DCMConfiguration.createDCMConfig()` (Java), add the `metricsTag` parameter as instructed in the API documentation.

## Migrating from Auto SDK v3.0.0 to v3.1.0
This section provides the information you need to migrate from Auto SDK v3.0.0 to Auto SDK v3.1.0.

### Migrating to the Authorization Platform Interface

Auto SDK v3.1.0 introduces the Authorization module that provides a single platform interface to manage different types of authorizations supported by the Engine. This single platform interface works with the Engine services that carry out the actual authorization process or flow. For more information about how authorization works, see the Core module documentation. This section provides the information you need for migrating to the Authorization platform interface from the CBL or AuthProvider platform interface, which are deprecated in v3.1.0

**Migrating from the CBL Platform Interface**

To migrate from the CBL platform interface to the Authorization platform interface, follow the instructions in the CBL module documentation, which describes the Authorization APIs for CBL authorization. 

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

To migrate from the AuthProvider platform interface to the Authorization platform interface, follow the instructions in the Alexa module documentation, which describes the Authorization APIs for Auth Provider authorization. 

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
* `Engine::setProperty()` and `Engine::getProperty()` have been removed. Use `PropertyManager::setProperty()` and `PropertyManager::getProperty()` instead. For details about the Property Manager platform interface, see the Core module documentation.
* The `SpeechRecognizer::enableWakeWordDetection()`, `SpeechRecognizer::disableWakeWordDetection()`, and `SpeechRecognizer::isWakewordDetectionEnabled()` methods have been removed.
* The Contact Uploader module has been removed. Use the Address Book module instead.

### Using the Address Book Module
Address Book module enables the user to upload contacts from the phone that is paired with the car or the navigation favorites from the car head unit to Alexa cloud. For more information about how this module works, see the Address Book module documentation. Both the Android and C++ sample apps demonstrate the use of the `AddressBook` platform interface. See the sample app source code for specific implementation details.

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

Read the updated Car Control module documentation to get a complete understanding of all supported features and the current format of the "aace.carControl" configuration schema. Read the updated API documentation for the `CarControlConfiguration` builder class if you construct your configuration programmatically. The changes to the "aace.carControl" configuration for v2.3 are backward-compatible, meaning your previous configuration (regardless of whether it was file-based or built programmatically with the `CarControlConfiguration` class) will still compile and produce a valid configuration to input to Auto SDK. However, several updates are recommended to ensure expected behavior, even if you do not want new features.

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

Specifying a "default" zone ID is an optional new feature, but it is highly recommended that you use it. See the Car Control module documentation for details about why this feature is important.

#### 4. Deprecated "DriverSeat" and related assets in favor of zones

Prior to v2.3, the default automotive catalog of assets introduced several asset IDs so that online-only systems could mock zones support for heaters on seat endpoints. The asset IDs are the following: `Alexa.Automotive.DeviceName.DriverSeat`, `Alexa.Automotive.DeviceName.LeftSeat`, `Alexa.Automotive.DeviceName.PassengerSeat`, `Alexa.Automotive.DeviceName.RightSeat`.

 Now that the cloud supports zones, you must stop using these asset IDs and properly model the endpoints using zones so that Alexa resolves user utterance intents as expected. These assets will be removed in a later version of Auto SDK. See the Car Control module documentation for sample configuration.

#### 5. New default assets

The Car Control module is updated to include many new assets in the default automotive catalog to support a wider range of utterances. If you previously defined custom assets to support any of the features introduced to the v2.3 assets, it is recommended that you use the new default assets instead of your previous custom ones. See the Car Control module documentation for details about assets.

#### 6. Reset your account when changing from 2.2 to 2.3 configuration

It is a known issue that you cannot delete any previously configured endpoint IDs associated with your customer account in the cloud. When upgrading your configuration from v2.2 to v2.3, contact your SA or Partner Manager for help to reset your account's endpoint database in the cloud. This is especially important if you are updating to use new features. It is also recommended that your v2.3 configuration follows the configuration sample of supported features shown in the Car Control module documentation. Refer to this document for reference.

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

`PropertyManager::setProperty()` and `PropertyManager::getProperty()` replace deprecated `Engine::setProperty()` and `Engine::getProperty()`. For details about the Property Manager platform interface, see Core module documentation.

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

    See the Builder documentation for details about supported platforms and targets.

* For QNX targets, you must cross-compile with the QNX multimedia software for the system audio extension (which is built by default for QNX targets). This requires a QNX Multimedia Suite license. See the System Audio extension documentation for details.

### Engine Configuration File Updates

The AVS Device SDK portion of the Auto SDK Engine configuration (the `aace.alexa.avsDeviceSDK` node) has been updated. See the [`config.json.in`](https://github.com/alexa/alexa-auto-sdk/blob/2.1/samples/cpp/assets/config.json.in) file for details.

  * The `"deviceInfo"` node includes two new elements: `"manufacturerName"` and `"description"`.
  * A path to the capabilities database is now required. Use the `"capabilitiesDelegate"` element to specify this path.
  * The `"settings"` element has changed to `"deviceSettings"`, and it includes these changes:
    * The default locale setting has been moved from `"defaultAVSClientSettings/locale"` to `"defaultLocale"`.
    * `"deviceSettings"` now requires a `"defaultTimezone"`.


### Navigation Enhancements

Auto SDK v2.1 introduces additional navigation features that you can integrate in your application to enrich the user's experience: add/cancel a waypoint, show/navigate to a previous destination, turn and lane guidance, and map display control. Implementing these enhancements required deprecating the `setDestination()` interface in favor of the `startNavigation()` interface and adding several additional interfaces.

To migrate from Auto SDK v2.0 to Auto SDK v2.1, you must update your platform implementation to use the `startNavigation()` method instead of the `setDestination()` method, modify the payload for the `getNavigationState()` method, and implement the new navigation methods. This guide takes you through these steps. See the Navigation module documentation for additional information and resources.

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

The TemplateRuntime interface remains the same, but the `LocalSearchListTemplate1` template has been deprecated in favor of the new `LocalSearchListTemplate2` template. In addition, two new templates (`TrafficDetailsTemplate` and `LocalSearchDetailTemplate1`), are now supported. The `TrafficDetailsTemplate` includes commute information to favorite destinations such as home or work. The `LocalSearchDetailTemplate1` template includes information about specific locations or information in response to users asking for details about locations presented in the `LocalSearchListTemplate2` template. For details about the TemplateRuntime interface, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html). For details about implementing TemplateRuntime in your Auto SDK implementation, see the Alexa module documentation.


### Car Control Source File Relocation

The Car Control module platform interface files and documentation are now located in `aac-sdk/modules/car-control` for C++ and `aac-sdk/platforms/android/modules/car-control` for Android, rather than in the Local Voice Control (LVC) extension directory structure.

>**Note:** In addition, if you use custom assets for car control in an implementation with the optional Local Voice Control (LVC) extension, you must specify the path to the custom assets in both the Auto SDK car control configuration and the LVC configuration, not just the LVC configuration. For details, see Car Control module documentation.

### Code-Based-Linking (CBL) Handler in the Sample Apps 
Both of the Auto SDK Sample Apps now include the Code-Based Linking (CBL) handler implementation (in favor of the `AuthProvider` handler implementation ) to handle obtaining access tokens from Login with Amazon (LWA). Changing from the `AuthProvider` handler to the CBL handler is not a required change, but we recommend that you use the Auto SDK CBL interface for ease of implementation. For details about the CBL handler, please see the CBL module documentation.

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
