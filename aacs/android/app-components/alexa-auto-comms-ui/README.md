# Alexa Auto Comms UI

This library serves the following purposes:

* It handles Bluetooth-related directives. 
   When a Bluetooth device is connected or disconnected, the `BluetoothReceiver` class receives the intent with the device's connection status, MAC address, and display name. When a device is paired, the class receives the intent for a device pairing, with the device's bond state, MAC address, and display name. The `BluetoothReceiver` class calls  the `BluetoothDirectiveHandler` class to update the Bluetooth device database based on the change. 

* It handles contacts upload or removal requests to Alexa.
   This library starts the AACS contacts service intents to handle contacts upload or removal requests.

On the Android Automotive OS, the last connected phone is considered the primary device. Only one device can upload the address book to the cloud at a time. If the primary phone changes, the old address book is removed and the new address book is scheduled to upload after 30 seconds. If you want to change this default behavior, implement your own system UI for users to select the primary phone and call the Android system API `setUserSelectedOutgoingPhoneAccount` to set the user-selected outgoing phone account. Send a `com.amazon.alexa.auto.comms.primaryPhoneChanged` intent to Alexa Auto Comms UI to inform the change of the primary phone.
```
ComponentName c = new ComponentName("com.amazon.alexa.auto.app", "com.amazon.alexa.auto.comms.ui.receiver.BluetoothReceiver");
intent.setComponent(c);
intent.addCategory("com.amazon.alexa.auto.comms");
intent.setAction("com.amazon.alexa.auto.comms.primaryPhoneChanged");
sendBroadcast(intent);
```

* A contacts consent screen is displayed during Alexa setup on the primary phone if the device does not have contacts upload permission. The contacts consent screen is also shown if a phone is paired after setup and does not have contacts upload permission. 

* The Alexa communication settings screen displays a list of paired devices, both connected and disconnected, with contacts upload permission status. The user can enable or disable the contacts upload permission for each device. If the contacts permission for the primary phone is enabled, the contacts in that device will be uploaded. If the contacts permission for a non-primary phone is enabled, the contacts in that phone will not be uploaded until the phone is selected as the primary. The permission is retained until logout when it is reset.

## Databases
There are two databases where devices are stored:

* `BTDeviceRepository` stores device information for every device that is paired. The devices are never removed from this database. 
* `ConnectedBTDeviceRepository` stores device information for currently connected devices only. If a phone is disconnected, it will be removed and will only be found in the `BTDeviceRepository`. If a phone is disconnected and a user enables/disables contacts for the device in the communications screen, the permission is stored in the BTDeviceRepository device and the ConnectedBTDevice is updated with the BTDevice permission on the next connection. 

## Prerequisites
The following list describes the prerequisites for this library:

* AACS Telephony and Contacts libraries must be built.
* Your Android device must meet the prerequisites for using the [AACS Telephony library](../alexa-auto-telephony/README.md#prerequisites).

## Known Issues
* On a reboot, Android automatically connects any phone that is paired which could cause the primary phone to change and contacts sync issues.
