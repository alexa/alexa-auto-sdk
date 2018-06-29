## ChangeLog

___

### **v1.0.0**

* v1.0.0 released on 2018-06-29
* Support for phone control APIs
* Support for CancelNavigation() in the Navigation API
* Support for Amazon Wake Word Engine (WWE)

### **Enhancements**

The following enhancements were added to the AAC SDK since the v1.0.0 Beta release.
* AAC SDK now supports 2 Navigation directives.
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

### **v1.0.0 Beta**

* v1.0.0 Beta released on 2018-04-29

### **Enhancements**

The following enhancements were added to the AAC SDK since the last Alpha release (binary).

 * `SetDestination()` API added to the Navigation module (see the [Modules](./README.md) sections in the "Alexa Automotive Core SDK" readme.)
 * Android Sample Application updated with a number of features such as rendering of Display Cards (Shopping List, Coffee Shops Nearby, etc), handling of the `SetDestination()` API, Notifications, LWA (Login with Amazon)

### **Known Issues**

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
