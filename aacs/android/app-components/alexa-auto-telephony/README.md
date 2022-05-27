# AACS Telephony 

The AACS Telephony library is an Android library for you to pre-integrate Alexa Phone Call Controller functionalities with Android Telephony. The library handles phone-related directives and events from and to the Alexa Auto SDK Engine, via the AACS Core Service. The library also works with the Dialer app on the car head unit. By including this optional library in AACS or your own application, you can easily integrate phone capabilities (e.g., dialing, redialing, and answering calls) with Alexa.  

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Building the Library](#building-the-library)
- [Setup for AACS Telephony Library](#setup-for-aacs-telephony-library)
- [Sequence Diagram](#sequence-diagram)
- [Phone Call Controlling](#phone-call-controlling)
      - [Answer](#answer)
      - [Dial](#dial)
      - [Redial](#redial)
      - [SendDTMF](#senddtmf)
      - [Stop](#stop)
      - [Update Device Configuration](#update-device-configuration)
- [(Optional) Receiving Device Connection Changes](#optional-receiving-device-connection-changes)
- [Known Issue](#known-issue)

## Overview

AACS Telephony Library is responsible for communicating with the AACS Core Service and initiating the corresponding actions based on the incoming directives from the Engine. The following list describes its major components that carry out these responsibilities: 

* The AACS Telephony Service is responsible for:
    * Receiving and processing `PhoneCallController` AASB message intents from the AACS Core Service.
    * Receiving defined intents from your application for specific actions, such as providing the proper Phone Account Handler to be used to place a call.

* The AACS Telephony platform implementation (`PhoneCallController`) with the Android telephony framework is responsible for:
    * Fulfilling the phone-call-related directives, which are received as intents by the AACS Telephony Service 
    * Capturing and reporting call state changes to the AACS Core Service
    * Capturing and reporting Bluetooth connection state changes to the AACS Core Service, and broadcasting the changes to any client listeners

The AACS Telephony Library is an optional module, which you can use as is or as a reference when you integrate the Phone Call Controller module with AACS. You can enable it in the AACS Sample App or in your application APK.

## Prerequisites
* Your Android system needs to support the Android Telephony framework.
* Your Android system needs to have a default dialer app that provides dialer and in-call UI. 
* Your Android system needs to support Bluetooth Hands-Free Profile (HFP) and Phone Book Access Profile (PBAP). Specifically, Bluetooth profiles with ID `HEADSET_CLIENT` and `PBAP_CLIENT` are required. For example, a device running Android Automotive OS supports these profiles.

## Building the Library
You can build the library locally using the following steps:

  1) Enter the following command to change the directory:
~~~
    cd ${AAC_SDK_HOME}/aacs/android/sample-app
~~~

  2) Enter the following command to build the Telephony library:
~~~
    ./gradlew :alexa-auto-telephony:assembleRelease
~~~

Replace `assembleRelease` with `assembleDebug` if you want to build the debug version of the library. The generated AAR is available at `alexa-auto-sdk/aacs/android/app-components/alexa-auto-telephony/aacstelephony/build/outputs/aar`.
You must include the `AACSIPC`, `AACSConstants`, `AACSCommonUtils`, `AACS` and `Auto SDK` AARs in your application to use with the AACS Telephony AAR.


To enable telephony support in the AACS Sample App, follow these steps:

1) Enter the following command to change the directory:
~~~
    cd ${AAC_SDK_HOME}/aacs/android/sample-app
~~~   
2) Enter the following command to start the local build with telephony enabled.
~~~
    ./gradlew assembleLocalRelease -PenabledTelephony
~~~
For more build options, see the [AACS Sample App README](../../sample-app/README.md#optional-arguments).

## Setup for AACS Telephony Library
Before using the AACS Telephony Library, follow these major steps:

1. Make the application containing the library into a system-privileged application.
2. Provide the library with appropriate system permissions.
3. Provide permission in your application's Android manifest so that the AACS Telephony Service can be started by your application.
4. Specify targets for intents from the AACS Core Service.

### Making Application into a System-Privileged App

The AACS Telephony Library needs to control the phone calls, and monitor the Bluetooth connection states and phone call states. Therefore, the application containing the library must be a system-privileged application. 

If the library is included in AACS, as recommended, you must run AACS as a system-privileged application. Similarly, if you put the library in your application, your application must be system-privileged. An application acquires system privilege when you install it in `/system/priv-app/`. 

### Providing System Permissions

The AACS Telephony Library requires three system-level permissions:

```
android.permission.CONTROL_INCALL_EXPERIENCE
android.permission.INTERACT_ACROSS_USERS
android.permission.CAPTURE_AUDIO_OUTPUT
```

Grant the permissions to the package containing the AACS Telephony Library. For instructions about granting permissions, see the [Android documentation](https://source.android.com/devices/tech/config/perms-allowlist). To grant permissions to AACS, if it contains the library, use `com.amazon.alexaautoclientservice` as the package name in the XML file referenced in the Android documentation.

### Providing Permission in Android Manifest

For security reasons, for your application to send intents to or receive intents from the AACS Telephony Service, specify the `com.amazon.aacstelephony` permission in your application's Android manifest as follows: 

```
<uses-permission android:name="com.amazon.aacstelephony" />
```

### Intent Targets

The AACS Telephony Service listens to intents from the AACS Core Service with these topics: `AASB` and `PhoneCallController`. To specify AACS Telephony Service as the intent target, follow one of these steps: 

* Manually specify the messages in the AACS configuration file. The targets in the AACS configuration file override the ones specified by intent filters. The following example shows how to specify AACS Telephony Service as an intent target for both the `AASB` and `PhoneCallController` topics. In this example, the AACS Telephony Library AAR is part of the AACS sample app APK. For more information about specifying intent targets, see the [AACS README](../../README.md#specifying-the-intent-targets-for-handling-messages).

```
    "AASB" : {
    "type": ["<target_1_type>","SERVICE", ...],
        "package": ["<target_1_package_name>", "com.amazon.alexa.auto.app", ...], 
        "class": ["<target_1_type>", "com.amazon.aacstelephony.AACSTelephonyService", ...]
    },
    //... other modules
    "PhoneCallController" : {
        "type": ["<target_1_type>", "SERVICE", ...],
        "package": ["<target_1_type>", "com.amazon.alexa.auto.app", ...],   
        "class": ["<target_1_type>", "com.amazon.aacstelephony.AACSTelephonyService", ...]
    }
    //... other modules
```

* Omit ANY targets for the `AASB` and `PhoneCallController` topics in the AACS configuration file. As a result, the intent filter defined in the AACS Telephony Library takes effect, enabling the AACS Telephony Service to receive the intents. 

## Sequence Diagram
The following sequence diagram illustrates the flow when the driver initiates a call with Alexa if the AACS Telephony Library is used.

![AACS Initiate Call](./docs/diagrams/AACSTelephony_initiateCall.png)

## Phone Call Controlling

This section describes the phone call controlling actions and work flows, whether the calls are initiated by Alexa or by the user from the head unit or mobile phone. 

### Answer

When a user asks Alexa to answer a call, the AACS Telephony Library answers the call that has the matching `callId` in the `PhoneCallController.Answer` payload of the AASB message. 

### Dial

When a user asks Alexa to dial a number or call an uploaded contact, the AACS Telephony Service calls the Android API [getDefaultOutgoingPhoneAccount](https://developer.android.com/reference/android/telecom/TelecomManager#getDefaultOutgoingPhoneAccount(java.lang.String)) to determine the proper [PhoneAccountHandle](https://developer.android.com/reference/android/telecom/PhoneAccountHandle) to use for initiating the call. 

The specific account returned by `getDefaultOutgoingPhoneAccount` depends on the following priorities:

* If the user-selected default PhoneAccount supports the specified scheme, it will be returned.
* If there exists only one PhoneAccount that supports the specified scheme, it will be returned.

In the Android Automotive OS the last connected device with HFP enabled is considered as the user-selected phone account. The OEMs can override this default behavior by implementing their own phone selection UI and calling the system API `setUserSelectedOutgoingPhoneAccount` to set the user selected account whenever the user makes a selection.

>**Note:** The AACS Telephony Service does not cache the phone account handle. Each time it receives the `Dial` message from AACS, it calls the `getDefaultOutgoingPhoneAccount` to determine which phone account handle to use.

### Redial

Similar to the Dial action, each Redial action requires the AACS Telephony Service to call the `getDefaultOutgoingPhoneAccount` to get the proper phone account handle. The AACS Telephony phone call controller queries the Android system on the head unit by using the `CallLog.Calls.getLastOutgoingCall` method, to get the last dialed number. 

### SendDTMF

When AACS Telephony Service gets a `sendDTMF` message, it applies the specified Dual Tone Multiple-Frequency (DTMF) tones to the call that has the matching `callId` in the `PhoneCallController.SendDTMF` payload of the AASB message. 

### Stop

When AACS Telephony Service gets a `PhoneCallController.Stop` AASB message, it stops the current call that has the matching `callId` in the message payload. 

### Update Device Configuration

This section describes how to update the device configuration.

>**Note:** the Auto SDK only supports updates to `DTMF_SUPPORTED` to enable or disable `SendDTMF`.

Use one of the following methods to update the device configuration:

* Use intent. Your application can send messages with a particular intent to the AACS Telephony Service to update the device configuration. Specify the attributes of the intent as follows:

    * Action is `com.amazon.aacstelephony.updateDeviceConfiguration`.
    * Category is `com.amazon.aacstelephony`.
    * Extras is:
```
        {
            "deviceProperty": "<Property name>",
            "enable": <Boolean value to enable or disable specified property>
        }
```

* Use a direct API call. This method is applicable only if you put the AACS Telephony Library in your application. The following code shows how to use the `PhoneCallController` API:

```java
    // Instantiate PhoneCallController
    PhoneCallController phoneCallController = new PhoneCallController(context, aacsMessageSender);
    phoneCallController.updateDeviceConfiguration(deviceProperty, enable);
```

>**Tip:** All the key constants, intent actions, and categories are defined in the `TelephonyConstants` class in the AACS Constants Library.

## (Optional) Receiving Device Connection Changes

The AACS Telephony Library not only detects and reports the Bluetooth connection state changes to the Engine (and subsequently Alexa), but can also broadcast the changes to your application if you find the broadcasts useful. Whenever `BluetoothStateListener` in the AACS Telephony Service detects a connection change with the Phone Book Access Profile (PBAP), it sends an intent with the device name and address to the listeners. The attributes of the intent with the connection information are as follows:

* Action is `com.amazon.aacstelephony.bluetooth.connected` for connected events and 
`com.amazon.aacstelephony.bluetooth.disconnected` for disconnected events.
* Category is `com.amazon.aacstelephony`.
* Extras is
```  
    {
        "deviceName": "<Device name>",
        "deviceAddress": "<Device Bluetooth MAC address>"
    }
```
It also detects when a phone is being paired and sends an intent with the device name, address, and bond state to the listener. There are  3 different bond states while a phone is pairing: [BOND_BONDED](https://developer.android.com/reference/android/bluetooth/BluetoothDevice#BOND_BONDED), [BOND_BONDING](https://developer.android.com/reference/android/bluetooth/BluetoothDevice#BOND_BONDING), and [BOND_NONE](https://developer.android.com/reference/android/bluetooth/BluetoothDevice#BOND_NONE).
* Action is `com.amazon.aacstelephony.bluetooth.bondStateChanged` 
* Category is `com.amazon.aacstelephony`
* Extras is
```  
    {
        "deviceName": "<Device name>",
        "deviceAddress": "<Device Bluetooth MAC address>"
        "bondState":<{BOND_BONDED, BOND_BONDING, or BOND_NONE}>
    }
```
The action "com.amazon.aacstelephony.bluetooth.pairedDevice" is also broadcasted during the initial connection check to add devices to notify the client application of devices paired before signing into Alexa.
* Action is `com.amazon.aacstelephony.bluetooth.pairedDevice` 
* Category is `com.amazon.aacstelephony`
* Extras is
```  
    {
        "deviceName": "<Device name>",
        "deviceAddress": "<Device Bluetooth MAC address>"
    }
```
## Known Issue

* When there is an active phone call, and if the application (either AACS or your application) containing AACS Telephony Library crashes, when it comes back, the InCallService defined in AACS Telephony Library would not automatically rebound, and therefore you cannot control the active call with Alexa. This is due to the InCallService in this library is not with the default dialer. New calls after the crash would trigger the InCallService to rebind to the system and phone call controlling would work as usual. Besides, reconnecting Bluetooth also triggers a rebinding in this case. 