# ChangeLog

___

## v1.3.0 released on 2018-11-20:

### Enhancements

Alexa Auto SDK now includes the following features and enhancements:

* Android 8 and ARM v8a platform support.
* Making calls to contacts from a locally paired mobile phone as long as the Alexa Auto SDK has a valid auth token. Read more about [Contact Uploader API](./modules/contact-uploader/README.md).
* Redial, answer, terminate and decline calls using voice. End users can also send dual-tone multi-frequency (DTMF) via voice to interact with Interactive Voice Responders (IVRs). Read more here [Phone Call Controller](./modules/phone-control/README.md).
* Switching to local media sources, generic controls and deep linking into 3rd party media applications compatible with the Amazon Media App Command and Control (MACC) specification using the External Media Player Interface 1.1. This allows customers to switch between  a CD player, AM/ FM player  and auxiliary input that is MACC compliant. Read more here [Handling External Media Adapter with MACCAndroidClient](./platforms/android/ALEXA.md#handlingexternalmediaadapterwithmaccandroidclient).  
* Enhancement for 3rd party wake word engine to enable cloud based verification.
* Provides a way to override Template Runtime display card timeout values for RenderTemplate and RenderPlayerInfo by updating the [templateRuntimeCapabilityAgent Engine configuration](https://alexa.github.io/aac-sdk/modules/core/#configuring-the-engine) values.

### Resolved Issues

No resolved issues.

### Known Issues

* The Alexa Auto SDK does not re-discover Media Apps Command and Control (MACC) compliant apps if they are unresponsive after being idle for a long period  (around 30 minutes).
* The Alexa Auto SDK Engine becomes unresponsive if it receives PLAY directive during shutdown. However since shutdown is triggered when car ignition is turned off, there is not direct customer impact expected.
* When a timer sounds during an Alexa to Alexa call, uttering "stop" ends the call not the timer.
* Multiple automotive devices using the same account at the same time, can access contacts from phones paired across those devices.

## v1.2.0 released on 2018-10-15:

### Enhancements

* Additional information related to the presentation of alerts is now available. The extended interface now includes Alert token, type, rendering time, and label if applicable when an alert is set and notification when an alert is deleted.
* In the Navigation platform interface, ```SetDestination``` now provides business hours and contact information for a returned location when available.

### Resolved Issues

* If a location is not available, the location state is set to ```unavailable```. Previously it was treated as ```(0,0)``` which was a valid value for longitude and latitude.
* Fixed an issue related to stopping an alert where there could be up to a 10 second delay before the alert completely stopped.
* Fixed issue where the ```TemplateRuntime``` platform interface could not be registered before ```AudioPlayer```.

### Known Issues

There are no known issues in this release.

## v1.1.1 released on 2018-09-10:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues

Issues fixed in this release:

* Update a dependency build recipe to skip the checksum verification to allow for document changes in the current tag.

### Known Issues

There are no known issues in this release.

## v1.1.0 released on 2018-08-31:

### Enhancements

* Added support for choosing one of multiple network adaptors before starting the engine.

* Added support for latest Amazon Wakeword engine.

* Added custom volume control support for infotainment system's native input volume range. The range that comes down to the device will be 0 to 100.

* Added support for encoding the utterance in OPUS format with the Amazon Wakeword Engine as well as PTT. Our builder pulls the libopus source code as a part of build process.

* Added Locale API to return the list of Alexa supported locales.

* Updated Vehicle Information API to capture the microphone details.

* Added support for routines, music alarms, timers and alarms volume management and deleting all timers and alarms.

* Added support for TemplateRuntime Interface 1.1 which provides visual playback control for Alexa enabled products with TemplateRuntime Interface support. This includes upgrades to PlaybackController Interface 1.1 and TemplateRuntime Interface 1.1.
    * > **Note**: The older button-press APIs (`playButtonPressed()` or `nextButtonPressed()`) have been deprecated in favor of the new generic `buttonPressed(PlaybackButtonType)`.

* Updated the builder script to confirm compiance with open source component licenses.

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

Issues fixed in this release:

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

## **v1.0.0**

* v1.0.0 released on 2018-06-29
* Support for phone control APIs
* Support for CancelNavigation() in the Navigation API
* Support for Amazon Wake Word Engine (WWE)

### **Enhancements**

The following enhancements were added to the Alexa Auto SDK since the v1.0.0 Beta release.

* Alexa Auto SDK now supports 2 Navigation directives.
    * **`SetDestination`**
    * **`CancelNavigation`**
* For Phone Control we now support the Dial Directive with three events.
    * **`CallActivated`**
    * **`CallTerminated`**
    * **`CallFailed`**

### **Known Issues**

The following are know issues in this release.

* The Engine doesn't immediately reconnect to AVS when the **`NetworkInfoProvider`** updates network status.
* Some shared memory objects are not freed when Engine object is disposed.

Sample App issues are documented in the [Sample App README](./samples/android/README.md).

## **v1.0.0 Beta**

* v1.0.0 Beta released on 2018-04-29

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
* Several minor documentation issues which will be address in the GA release
