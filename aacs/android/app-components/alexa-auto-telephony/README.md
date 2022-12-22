# Alexa Auto Telephony Library 

The Alexa Auto Telephony Library is an Android library for you to pre-integrate Alexa Phone Call Controller and Messaging functionalities with Android Telephony. The library handles phone-related directives and events from and to the Alexa Auto SDK Engine, via the AACS Core Service. The library also works with the Dialer app on the car head unit. You can easily integrate phone capabilities (e.g., dialing, redialing, answering calls, sending and reading text messages) with Alexa.

<!-- omit in toc -->
## Table of Contents
- [Alexa Auto Telephony Library](#alexa-auto-telephony-library)
  - [Overview](#overview)
  - [Prerequisites](#prerequisites)
  - [Setup for Alexa Auto Telephony Library](#setup-for-alexa-auto-telephony-library)
    - [Making Application into a System-Privileged App](#making-application-into-a-system-privileged-app)
    - [Providing System Permissions](#providing-system-permissions)
    - [Providing Permission in Android Manifest](#providing-permission-in-android-manifest)
    - [Intent Targets](#intent-targets)
  - [Phone Call Controlling](#phone-call-controlling)
    - [Sequence Diagram](#sequence-diagram)
    - [Answer](#answer)
    - [Dial](#dial)
    - [Redial](#redial)
    - [SendDTMF](#senddtmf)
    - [Stop](#stop)
    - [Update Device Configuration](#update-device-configuration)
  - [(Optional) Receiving Device Connection Changes](#optional-receiving-device-connection-changes)
  - [Messaging](#messaging)
    - [Sequence Diagram](#sequence-diagram-1)
    - [Send Message](#send-message)
    - [Reading Messages](#reading-messages)
  - [Known Issues](#known-issues)

## Overview

Alexa Auto Telephony Library is responsible for communicating with the AACS Core Service and initiating the corresponding actions based on the incoming directives from the Engine. The following list describes its major components that carry out these responsibilities: 

* The AACS Telephony Service is responsible for:
    * Receiving and processing `PhoneCallController` and `Messaging` AASB message intents from the AACS Core Service.
    * Receiving defined intents from your application for specific actions, such as providing the proper Phone Account Handler to be used to place a call.
    * Receiving defined intents from the Android platform such as unread messages from a bluetooth connected mobile device.

* The AACS Telephony platform implementation (`PhoneCallController`) with the Android telephony framework is responsible for:
    * Fulfilling the phone-call-related directives, which are received as intents by the AACS Telephony Service.
    * Capturing and reporting call state changes to the AACS Core Service.
    * Capturing and reporting Bluetooth connection state changes to the AACS Core Service, and broadcasting the changes to any client listeners.

* The AACS Telephony platform implementation (`Messaging`) with the Android Bluetooth Message Access Profile (`MAP`) framework is responsible for:
    * Fulfilling the messaging-related directives, which are received as intents by the AACS Telephony Service.
    * Capturing and reporting user consent to send and read text messages using the primary connected bluetooth device.
    * Capturing and reporting Bluetooth connection state changes to the AACS Core Service.

## Prerequisites
* Your Android system needs to support the Android Telephony framework.
* Your Android system needs to have a default dialer app that provides dialer and in-call UI. 
* Your Android system needs to support Bluetooth Hands-Free Profile (HFP) and Phone Book Access Profile (PBAP). Specifically, Bluetooth profiles with ID `HEADSET_CLIENT` and `PBAP_CLIENT` are required. For example, a device running Android Automotive OS supports these profiles.
* Your Android system needs to support Bluetooth Message Access Profile (MAP).
* The Android framework publishes unread text messages via the [Bluetooth.ACTION_MESSAGE_RECEIVED](https://android.googlesource.com/platform/frameworks/base/+/android-9.0.0_r3/core/java/android/bluetooth/BluetoothMapClient.java#45) intent. In order for the Alexa Auto Telephony Library to receive those intents, which allow Alexa to read text messages, there must be no default messaging application installed in the system or the AAOS system image must be modified to not [target those intents](https://android.googlesource.com/platform/packages/apps/Bluetooth/+/master/src/com/android/bluetooth/mapclient/MceStateMachine.java#862) to the default messaging application.
* An iOS mobile device must enable "Show Notifications" under the bluetooth connection settings once it is paired with the AAOS device in order to allow text messages to be sent.

## Setup for Alexa Auto Telephony Library
Before using the Alexa Auto Telephony Library, follow these major steps:

1. Make the application containing the library into a system-privileged application.
2. Provide the library with appropriate system permissions.
3. Provide permission in your application's Android manifest so that the Aelxa Auto App Telephony Service can be started by your application.
4. Specify targets for intents from the AACS Core Service.

### Making Application into a System-Privileged App

The Alexa Auto Telephony Library needs to control the phone calls, send text messages, initiate unread text message retrieval, and monitor the Bluetooth connection states and phone call states. Therefore, the Alexa Auto App containing the library must be a system-privileged application. 

### Providing System Permissions

The Alexa Auto Telephony Library requires the following system-level permissions:

```
android.permission.CONTROL_INCALL_EXPERIENCE
android.permission.INTERACT_ACROSS_USERS
android.permission.CAPTURE_AUDIO_OUTPUT
android.permission.BLUETOOTH_PRIVILEGED
```

Grant the permissions to the package containing the Alexa Auto Telephony Library. For instructions about granting permissions, see the [Android documentation](https://source.android.com/devices/tech/config/perms-allowlist).

### Providing Permission in Android Manifest

For security reasons, for your application to send intents to or receive intents from the AACS Telephony Service, specify the `com.amazon.aacstelephony` permission in your application's Android manifest as follows: 

```
<uses-permission android:name="com.amazon.aacstelephony" />
```

### Intent Targets

The AACS Telephony Service listens to intents from the AACS Core Service with these topics: `AASB`, `Messaging`, and `PhoneCallController`. To specify AACS Telephony Service as the intent target, follow one of these steps: 

* Manually specify the messages in the Alexa Auto App configuration file. The targets in the Alexa Auto App configuration file override the ones specified by intent filters. The following example shows how to specify AACS Telephony Service as an intent target for both the `AASB`, `Messaging`, and `PhoneCallController` topics. In this example, the Alexa Auto Telephony Library AAR is part of the Alexa Auto App APK. For more information about specifying intent targets, see the [configuration README](../../README.md#specifying-the-intent-targets-for-handling-messages).

```
    "AASB" : {
    "type": ["<target_1_type>","SERVICE", ...],
        "package": ["<target_1_package_name>", "com.amazon.alexa.auto.app", ...], 
        "class": ["<target_1_type>", "com.amazon.aacstelephony.AACSTelephonyService", ...]
    },
    //... other modules
    "Messaging" : {
        "type": ["<target_1_type>", "SERVICE", ...],
        "package": ["<target_1_type>", "com.amazon.alexa.auto.app", ...],
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

* Omit ANY targets for the `AASB`, `Messaging`, and `PhoneCallController` topics in the Alexa Auto App configuration file. As a result, the intent filter defined in the Alexa Auto Telephony Library takes effect, enabling the AACS Telephony Service to receive the intents. 



## Phone Call Controlling

This section describes the phone call controlling actions and work flows, whether the calls are initiated by Alexa or by the user from the head unit or mobile phone.

### Sequence Diagram
The following sequence diagram illustrates the flow when the driver initiates a call with Alexa if the Alexa Auto Telephony Library is used to initiate a call.

![AACS Initiate Call](./docs/diagrams/AACSTelephony_initiateCall.png)
### Answer

When a user asks Alexa to answer a call, the Alexa Auto Telephony Library answers the call that has the matching `callId` in the `PhoneCallController.Answer` payload of the AASB message. 

### Dial

When a user asks Alexa to dial a number or call an uploaded contact, the AACS Telephony Service calls the Android API [getDefaultOutgoingPhoneAccount](https://developer.android.com/reference/android/telecom/TelecomManager#getDefaultOutgoingPhoneAccount(java.lang.String)) to determine the proper [PhoneAccountHandle](https://developer.android.com/reference/android/telecom/PhoneAccountHandle) to use for initiating the call. 

The specific account returned by `getDefaultOutgoingPhoneAccount` depends on the following priorities:

* If the user-selected default PhoneAccount supports the specified scheme, it will be returned.
* If there exists only one PhoneAccount that supports the specified scheme, it will be returned.

In the Android Automotive OS the last connected device with HFP enabled is considered as the user-selected phone account. The OEMs can override this default behavior by implementing their own phone selection UI and calling the system API `setUserSelectedOutgoingPhoneAccount` to set the user selected account whenever the user makes a selection.

>**Note:** The AACS Telephony Service does not cache the phone account handle. Each time it receives the `Dial` message, it calls the `getDefaultOutgoingPhoneAccount` to determine which phone account handle to use.

### Redial

Similar to the Dial action, each Redial action requires the AACS Telephony Service to call the `getDefaultOutgoingPhoneAccount` to get the proper phone account handle. The Telephony phone call controller queries the Android system on the head unit by using the `CallLog.Calls.getLastOutgoingCall` method, to get the last dialed number. 

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

* Use a direct API call. This method is applicable only if you put the Alexa Auto Telephony Library in your application. The following code shows how to use the `PhoneCallController` API:

```java
    // Instantiate PhoneCallController
    PhoneCallController phoneCallController = new PhoneCallController(context, aacsMessageSender);
    phoneCallController.updateDeviceConfiguration(deviceProperty, enable);
```

>**Tip:** All the key constants, intent actions, and categories are defined in the `TelephonyConstants` class in the AACS Constants Library.

## (Optional) Receiving Device Connection Changes

The Alexa Auto Telephony Library not only detects and reports the Bluetooth connection state changes to the Engine (and subsequently Alexa), but can also broadcast the changes to your application if you find the broadcasts useful. Whenever `BluetoothStateListener` in the AACS Telephony Service detects a connection change with the Phone Book Access Profile (PBAP), it sends an intent with the device name and address to the listeners. The attributes of the intent with the connection information are as follows:

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

## Messaging

This section describes the messaging actions and work flows, whether the messages are initiated by Alexa or mobile phone. Messaging with Alexa supports sending text messages, reading unread text messages, and replying to text message as they are read out.

### Sequence Diagram
The following sequence diagram illustrates the flow when the driver sends a text message with Alexa.

![Send Message](./docs/diagrams/AACSTelephony_sendMessage.png)

The following sequence diagram illustrates the flow when the driver asks Alexa to read text messages.

![Read Message](./docs/diagrams/AACSTelephony_readMessage.png)

### Send Message

When a user asks Alexa to send a text message to a number or an uploaded contact, Alexa will guide the user through a series of questions to complete the send message flow. This will include disambiguation questions in the case where multiple contact names match the request, or a contact contains multiple phone numbers. Alexa will also ask the user to confirm if the message should be sent, and if agreed, a directive will be sent to the device with the message information.

In the Android Automotive OS the last connected device with MAP enabled is considered as the user-selected phone account. This will be device used for sending the requested text message.
The AACS Telephony Service calls the Android BluetoothMapClient API [sendMessage](https://android.googlesource.com/platform/frameworks/base/+/android-9.0.0_r3/core/java/android/bluetooth/BluetoothMapClient.java#361) to deliver the text message.

### Reading Messages

When a user asks Alexa to read text messages, any previously uploaded conversations will be read out. The Alexa Auto Telephony Library handles listening for received message intents, building conversations, uploading to the cloud, and managing which messages have been read. As Alexa reads a message, it will send a directive to the device specifying which message was read. When message readout completes, Alexa will also send a request to the device to upload the latest conversations in order to stay in sync with the device.

The user has the option to reply to a text message only while Alexa reads out the message. Alexa will know the recipient since the reply is within the context of the message that is currently being read out.
## Known Issues

* When there is an active phone call, and if the application (either AACS or your application) containing Alexa Auto Telephony Library crashes, when it comes back, the InCallService defined in Alexa Auto Telephony Library would not automatically rebound. Therefore you cannot control the active call with Alexa. The automatic rebound does not occur because the InCallService in this library does not have the default dialer. New calls after the crash would trigger the InCallService to rebind to the system, and phone call controlling would work as usual. Besides, reconnecting Bluetooth also triggers rebinding in this case.
* Asking Alexa to send a message to a contact's home number will fail. This only happens if the word "home" is included in the request (i.e. Alexa send a message to Bob's home phone number).
* AAOS 10 and 11 do not support clearing the unread status from the mobile device once it has been read.
