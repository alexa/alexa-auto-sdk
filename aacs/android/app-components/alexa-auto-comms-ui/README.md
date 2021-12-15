# Alexa Auto Comms UI

This library serves the following purposes:

* It handles Bluetooth-related directives. 
   When a Bluetooth device is connected or disconnected, the `BluetoothReceiver` class receives the intent with device connection status, and device MAC address and display name. It also calls the `BluetoothDirectiveHandler` class to update the Bluetooth device database based on the change.

* It handles contacts upload or removal requests to Alexa.
   This library starts the AACS contacts service intents to handle contacts upload or removal requests.

* Alexa setup screen provides options for the user to give or decline consent to contact uploads on the primary phone.

* The Alexa communication settings screen displays a list of connected devices with contacts upload permission status. The user can turn on or turn off the contacts upload permission for each device. If the contacts permission for the primary phone is enabled, the contacts in that device will be uploaded. If the contacts permission for a non-primary phone is enabled, the contacts in that phone will not be uploaded until the phone is selected as the primary.

On the Android Automotive OS, the last connected phone is considered as the primary device. If you want to change this default behavior, implement your own system UI for users to select the primary phone and call the Android system API `setUserSelectedOutgoingPhoneAccount` to set the user-selected outgoing phone account. Send a `com.amazon.alexa.auto.comms.primaryPhoneChanged` intent to Alexa Auto Comms UI to inform the change of the primary phone. 
```
ComponentName c = new ComponentName("com.amazon.alexa.auto.app", "com.amazon.alexa.auto.comms.ui.receiver.BluetoothReceiver");
intent.setComponent(c);
intent.addCategory("com.amazon.alexa.auto.comms");
intent.setAction("com.amazon.alexa.auto.comms.primaryPhoneChanged");
sendBroadcast(intent);
```


## Prerequisites
The following list describes the prerequisites for this library:
* AACS Telephony and Contacts libraries must be built.
* Your Android device must meet the prerequisites for using the [AACS Telephony library](../alexa-auto-telephony/README.md#prerequisites).