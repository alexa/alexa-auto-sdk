# ChangeLog

___

## v1.6.0 released on 2019-05-16:

### Enhancements

* General availability for Linux target platforms, including: Linux x86-64, Linux ARM 64 (armv8a), and Linux ARM 32 (armv7a).
* Alexa Auto SDK v1.6.0 enhances the C++ Sample App by improving the reference implementation for Linux platforms. Read more about the C++ Sample App [here](./samples/cpp/README.md).

### Resolved Issues

* Fixed an issue where Alexa Auto SDK Engine becomes unresponsive if it receives a `Play` directive during shutdown.
* Made changes to External Media Player events to send the service id and agent details, which are now mandated by the Alexa Music service. If you are using previous versions with Local Media Source switching or third-party app with MACC, you should upgrade to Alexa Auto SDK v1.6.0 to continue using the corresponding functionality.
  
### Known Issues

* If the local timezone of the device differs from the timezone that was configured through the Alexa companion app, you may experience unexpected behavior. For example, if your device shows 12pm PST, but the device on the Alexa companion app is configured with an EST timezone, then asking "Alexa set an alarm for 1pm today," will return, "Sorry I can't set alarms in the past.”
* If you play your notifications while music is playing, you will hear the music for a split second between the end of one notification and the start of the next.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.5.0 released on 2019-03-06:

### Enhancements

* Added a C++ sample application to demonstrate use cases that the Alexa Auto SDK supports. Read more about the C++ Sample App [here](./samples/cpp/README.md).
* Released the code for the AGL Alexa Voice Agent, a binding for Automotive Grade Linux powered by Alexa Auto SDK v1.5. The software is shipped as a standard AGL binding that exposes an API for speech recognition using the Alexa Voice Service. Please refer to the [AGL Alexa Voice Agent documentation](./platforms/agl/alexa-voiceagent-service/README.md) for instructions to build, install, and test the binding on an R-Car M3 board.
* Added support for runtime selection of the AmazonLite wake word locale. The AmazonLite locale will automatically switch when the AVS locale is switched.
* Added support for optionally logging and uploading Alexa Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and UPL between the request and Alexa’s response. Please contact your SA or Partner Manager for details or to request this package for Android.
* Added support for an optional platform interface `EqualizerController`. The Equalizer Controller enables Alexa voice control of device audio equalizer settings by making gain adjustments to three frequency bands (“BASS”, “MIDRANGE”, and/or “TREBLE”).
* Added an optional Code-Based Linking (CBL) authorization implementation in the Engine. With the new `cbl` module, the Engine handles acquiring access tokens. A `CBL` platform implementation should be registered with the Engine in place of an `AuthProvider` implementation to use this method for authorization.
* Improved the usage and deployment of the Local Voice Control extension on Android. Please contact your SA or Partner Manager for more information.
* Updated the vehicle information configuration API to include a vehicle identifier. An `aace.vehicle.info.vehicleIdentifier` property of vehicle configuration is now available through the existing `VehicleConfiguration`.

### Resolved Issues

* Fixed an issue where barging in while many unprocessed Speak directives are queued could cause SpeechSynthesizer to become unresponsive or crash
* Added an `EXPECTING` state to the `AlexaClient DialogState` to accommodate multi-turn for hold-to-talk interactions. When more user input is required during an interaction, tap-to-talk interactions will transition directly from `EXPECTING` to `LISTENING` whereas hold-to-talk will remain in the `EXPECTING` state until listening is manually triggered.
* Fixed an issue where the Android Sample App could get stuck in a loop of INVALID_REQUEST_EXCEPTION errors being thrown repeatedly after MACCAndroidClient reported an error. Note: To fix this, the C++ `ExternalMediaAdapter::getState` method signature changed to allow the implementation to say whether the state it provides is valid. This change is not backward compatible.
* Fixed an issue where the Android Sample App created a syslog sink and logged VERBOSE in release builds. Note: As part of the fix, the default Engine logger sink id changed from *console* to *default*. Existing calls to `LoggerConfiguration::createLoggerRuleConfig` with sink id `"console"` should be changed to sink id `"default"`.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a `Play` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.4.0 released on 2018-12-17:

### Enhancements

* The Alexa Auto SDK now supports the Local Voice Control extension. The Local Voice Control extension enhances the Alexa Auto experience by providing voice-based car controls whether connected to the internet or not. In this release, the Local Voice Control extension will provision access only to the car’s climate control.

    > **Note**: This extension is available on request - Please contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

### Resolved Issues

No resolved issues.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a ```Play``` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.3.0 released on 2018-11-20:

### Enhancements

* Android 8 and ARM v8a platform support.
* Making calls to contacts from a locally-paired mobile phone as long as the Alexa Auto SDK has a valid auth token. Read more about [Contact Uploader API](./modules/contact-uploader/README.md).
* Redial, answer, terminate, and decline calls using voice. End users can also send dual-tone multi-frequency (DTMF) via voice to interact with Interactive Voice Responders (IVRs). Read more here [Phone Call Controller](./modules/phone-control/README.md).
* Switching to local media sources, generic controls and deep linking into 3rd party media applications compatible with the Amazon Media App Command and Control (MACC) specification using the External Media Player Interface 1.1. This allows customers to switch between a CD player, AM/FM player, and auxiliary input that is MACC-compliant. Read more here [Handling External Media Adapter with MACCAndroidClient](./platforms/android/ALEXA.md#handlingexternalmediaadapterwithmaccandroidclient).  
* Enhancement for 3rd party wake word engine to enable cloud based verification.
* Provides a way to override Template Runtime display card timeout values for RenderTemplate and RenderPlayerInfo by updating the [templateRuntimeCapabilityAgent Engine configuration](https://alexa.github.io/aac-sdk/modules/core/#configuring-the-engine) values.

### Resolved Issues

No resolved issues.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a ```Play``` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.2.0 released on 2018-10-15:

### Enhancements

* Additional information related to the presentation of alerts is now available. The extended interface now includes Alert token, type, rendering time, and label if applicable when an alert is set and notification when an alert is deleted.
* In the Navigation platform interface, ```SetDestination``` now provides business hours and contact information for a returned location when available.

### Resolved Issues

* If a location is not available, the location state is set to ```unavailable```. Previously it was treated as ```(0,0)```, which was a valid value for longitude and latitude.
* Fixed an issue related to stopping an alert where there could be up to a 10 second delay before the alert completely stopped.
* Fixed issue where the ```TemplateRuntime``` platform interface could not be registered before ```AudioPlayer```.

### Known Issues

There are no known issues in this release.

## v1.1.1 released on 2018-09-10:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues


* Updated a dependency build recipe to skip the checksum verification to allow for document changes in the current tag.

### Known Issues

There are no known issues in this release.

## v1.1.0 released on 2018-08-31:

### Enhancements

* Added support for choosing one of multiple network adaptors before starting the Engine.
* Added support for the latest Amazon Wakeword Engine.
* Added custom volume control support for infotainment system's native input volume range. The range that comes down to the device will be 0 to 100.
* Added support for encoding the utterance in OPUS format with the Amazon Wakeword Engine as well as PTT. Our builder pulls the libopus source code as a part of build process.
* Added Locale API to return the list of Alexa-supported locales.
* Updated Vehicle Information API to capture the microphone details.
* Added support for routines, music alarms, timers and alarms volume management, and deleting all timers and alarms.
* Added support for TemplateRuntime Interface 1.1, which provides visual playback control for Alexa-enabled products with TemplateRuntime Interface support. This includes upgrades to PlaybackController Interface 1.1 and TemplateRuntime Interface 1.1.
    * > **Note**: The older button-press APIs (`playButtonPressed()`, `nextButtonPressed()`, etc.) have been deprecated in favor of the new generic `buttonPressed(PlaybackButtonType)`.
* Updated the builder script to confirm compliance with open source component licenses.

### Resolved Issues

There are no resolved issues in this release.

### Known Issues

There are no known issues in this release.

## v1.0.2 released on 2018-08-08:

### Enhancements

This release is only for documentation updates. There are no new features or enhancements.

### Resolved Issues

Only name change updates were made to the documentation. There are no resolved issues in this release.

### Known Issues

There are no known issues in this release.

## v1.0.1 released on 2018-07-31:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues

* The Engine now reconnects to Alexa when the `NetworkInfoProvider` updates the network status.
* All shared memory objects are now freed when the Engine object is disposed.
* We fixed a media playback state issue in the Engine that caused an unexpected pause and resume for a media stream that was already stopped.
* We added AudioPlayer::playerActivityChanged to the Android APIs.
* Updated the `AuthError` enumeration with additional error types.
* Removed deprecated `createAuthConfig()` configuration method.
* Fixed issue in the JNI where trying to create a UTF-8 string with invalid characters caused a crash, seen when sensitive logging is enabled.
* Improved JNI thread handling.
* Enabled capability registration for phone call control.
* We fixed an issue where the Android platform build failed on the initial attempt when using clean code.

### Known Issues

There are no known issues in this release.

## v1.0.0 released on 2018-06-29:

### **Enhancements**

* Alexa Auto SDK now supports two ```Navigation``` directives.
    * **`SetDestination`**
    * **`CancelNavigation`**
* Added support for phone control APIs. The ```PhoneCallController``` platform interface supports the **`Dial`** directive with three events:
    * **`CallActivated`**
    * **`CallTerminated`**
    * **`CallFailed`**
* Support for Amazon Wake Word Engine (WWE)

### **Known Issues**

* The Engine doesn't immediately reconnect to AVS when the **`NetworkInfoProvider`** updates network status.
* Some shared memory objects are not freed when the Engine object is disposed.

Sample App issues are documented in the [Sample App README](./samples/android/README.md).

## v1.0.0 Beta released on 2018-04-29:

### **Enhancements**

The following enhancements were added to the Alexa Auto SDK since the last Alpha release (binary).

 * `SetDestination()` API added to the Navigation module (see the [Modules](./README.md) sections in the "Alexa Auto SDK" readme.)
 * Android Sample Application updated with a number of features such as rendering of Display Cards (Shopping List, Coffee Shops Nearby, etc), handling of the `SetDestination()` API, Notifications, LWA (Login with Amazon)

### **Known Issues**

***SDK:***

* While the SDK does build against Android API22 and above and runs successfully on Android O devices, our current testing shows a native-code linking error when actually running on API22 devices.

***Android Sample App:***

* M3U and PLS based stream URLs are not parsed before sent to the Android Mediaplayer. Affects live streams typically coming from TuneIn and IHeartRadio services
* Media playback can take a long time to start sometimes for iHeartRadio and TuneIn
* The Android Alexa Auto SDK Sample App was developed on an Android tablet with 2048 x 1536 resolution screen size. It can run on smaller devices, but some of the display cards may not display correctly
* During Playing Media in the Sample App we see the following messages (none of these will cause any issues):
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=streamFormat
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=progressReportDelayInMilliseconds
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=expectedPreviousToken
  * E/AAC:aace.alexa.AudioChannelEngineImpl:validateSource:reason=invalidSource
  * E/AAC:aace.alexa.AudioChannelEngineImpl:pause:reason=invalidSource,expectedState=X
* On App startup we see the following messages (none of these will cause any issues):
   * E/AVS:SQLiteAlertStorage:openFailed::File specified does not exist.:file path=/data/user/0/com.amazon.sampleapp/cache/appdata/alerts.sqlite
* Several minor documentation issues that will be addressed in the GA release
