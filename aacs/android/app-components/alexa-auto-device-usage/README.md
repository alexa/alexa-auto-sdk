# Alexa Auto Device Usage
Alexa Auto Device Usage library provides an implementation that enables the Alexa Auto App to capture network usage data using Android [NetworkStatsManager](https://developer.android.com/reference/android/app/usage/NetworkStatsManager). The data is logged as a metric and is sent to Amazon endpoints.

## Overview
The library consists of the following components:

*  `AASBReceiver`. This Android BroadcastReceiver subscribes to [AASB StartService message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/aasb/AASB/index.html#startservice) and [AASB StopService message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/aasb/AASB/index.html#stopservice), based on which the `DeviceUsageHandler` can start or stop the network data recording.
*  `DeviceUsageHandler`. The handler is responsible for starting and stopping the `NetworkStatsManagerRunner`. It starts the `NetworkStatsManagerRunner` in an executor thread which queries the network usage every 5 minutes and publishes the data via [DeviceUsage `ReportNetworkDataUsageMessage` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/DeviceUsage/index.html#reportnetworkdatausage) to the Auto SDK engine.
*  `NetworkStatsManagerRunner`. This class captures the network usage data using `NetworkStatsManager` APIs and calls the `DeviceUsageHandler` to report the data.

## Providing Permissions for Alexa Auto Device Usage Library
Using the Alexa Auto Device Usage Library requires the Alexa Auto App to hold certain Android permissions.

*  If the Android version of your device is lower than 10 (Android Q): The Alexa application needs the `android.permission.READ_PHONE_STATE` permission to access the `subscriber id` (associated to the eSIM of a user), which is required to query the network consumption over the `MOBILE` interface. You can grant this permission to Alexa Auto App in "Settings" -> "Apps & notifications" -> "Alexa" -> "Permissions" -> "Phone".

*  If the Android version of your device is 10 (Android Q) or higher: The Alexa application must be installed as a privileged system application and it must have the `android.permission.READ_PRIVILEGED_PHONE_STATE` privileged permission in order to be able to get the `subscriber id`.

> **Note** The Alexa Auto Device Usage Library carries all the Android permissions (`READ_PHONE_STATE`, `READ_PRIVILEGED_PHONE_STATE` and `PACKAGE_USAGE_STATS`) your application needs in order to enable the network usage data recording. To use this library, make sure your Alexa Auto App is built with the `-PenabledDeviceUsage` option, as specified in the [Building Alexa Auto App with Alexa Auto Device Usage Library](./README.md#building-aacs-sample-app-with-alexa-auto-device-usage-library) section. If your device is on Android 10 or higher version, install Alexa Auto App as a system privileged app, and place the following `privapp-permissions-com.amazon.alexa.auto.app.xml` file under the `/etc/permissions` directory on your device:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <permissions>
	    <privapp-permissions package="com.amazon.alexa.auto.app">
        <permission name="android.permission.READ_PRIVILEGED_PHONE_STATE" />
        <permission name="android.permission.PACKAGE_USAGE_STATS" />
        </privapp-permissions>
    </permissions>
```
