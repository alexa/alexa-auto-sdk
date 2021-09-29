# Alexa Auto Device Usage Library
Alexa Auto Device Usage Library provides an implementation that enables the AACS Sample App to capture network usage data of Alexa Auto Client Service (AACS) using Android [NetworkStatsManager](https://developer.android.com/reference/android/app/usage/NetworkStatsManager). The data is logged as a metric and is sent to Amazon endpoints if AACS is built with the `Device Client Metrics (DCM)` extension. To obtain the pre-built AACS APK or AAR with DCM extension, contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

## Overview
The library consists of the following components:
*  `AASBReceiver`. This Android BroadcastReceiver subsribes to [AASB StartService message](../../../../extensions/aasb/docs/AASB/StartServiceMessage.html) and [AASB StopService message](../../../../extensions/aasb/docs/AASB/StopServiceMessage.html) to get the current AACS running status, based on which the `DeviceUsageHandler` can start or stop the network data recording.
*  `DeviceUsageHandler`. The handler is responsible for starting and stopping the `NetworkStatsManagerRunner` according to the AACS running status. It starts the `NetworkStatsManagerRunner` in an executor thread which queries the network usage every 5 minutes and publishes the data via [AASB DeviceUsage message](../../../../extensions/aasb/docs/DeviceUsage/ReportNetworkDataUsageMessage.html) to the Auto SDK engine.
*  `NetworkStatsManagerRunner`. This class captures the network usage data using `NetworkStatsManager` APIs and calls the `DeviceUsageHandler` to report the data.

## Providing Permissions for Alexa Auto Device Usage Library
Using the Alexa Auto Device Usage Library requires the AACS sample app to hold certain Android permissions.

*  If the Android version of your device is lower than 10 (Android Q): The Alexa application needs the `android.permission.READ_PHONE_STATE` permission to access the `subscriber id` (associated to the eSIM of a user), which is required to query the network consumption over the `MOBILE` interface. You can grant this permission to AACS sample app in "Settings" -> "Apps & notifications" -> "Alexa" -> "Permissions" -> "Phone".

*  If the Android version of your device is 10 (Android Q) or higher: The Alexa application must be installed as a privileged system application and it must have the `android.permission.READ_PRIVILEGED_PHONE_STATE` privileged permission in order to be able to get the `subscriber id`.

*  If you install AACS as a separate APK: The Alexa application must be installed as a privileged system application and it must have the `android.permission.PACKAGE_USAGE_STATS` which is required to access stats for apps other than the calling app.

> **Note** The Alexa Auto Device Usage Library carries all the Android permissions (`READ_PHONE_STATE`, `READ_PRIVILEGED_PHONE_STATE` and `PACKAGE_USAGE_STATS`) your application needs in order to enable the network usage data recording. To use this library, make sure your AACS sample app is built with the `-PenabledDeviceUsage` option, as specified in the [Building AACS Sample App with Alexa Auto Device Usage Library](./README.md#building-aacs-sample-app-with-alexa-auto-device-usage-library) section. If your device is on Android 10 or higher version, or you use AACS as a separate APK, install AACS sample app as a system privileged app, and place the following `privapp-permissions-com.amazon.alexa.auto.app.xml` file under the `/etc/permissions` directory on your device:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <permissions>
	    <privapp-permissions package="com.amazon.alexa.auto.app">
        <permission name="android.permission.READ_PRIVILEGED_PHONE_STATE" />
        <permission name="android.permission.PACKAGE_USAGE_STATS" />
        </privapp-permissions>
    </permissions>
```

## Building AACS Sample App with Alexa Auto Device Usage Library
To build the AACS Sample App with Alexa Auto Device Usage library, go to `${AAC_SDK_HOME}/samples/android-aacs-sample-app/` and enter the following command:

```shell
// release build
gradle assembleRelease -PenabledDeviceUsage

// debug build
gradle assembleDebug -PenabledDeviceUsage
```