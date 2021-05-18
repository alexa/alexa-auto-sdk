# Alexa Auto Comms UI

This library serves the following purposes:

* It handles Bluetooth-related directives.
   When a Bluetooth device is connected or disconnected, the `BluetoothReceiver` class receives the intent with device connection status, and device MAC address and display name. It also calls the `BluetoothDirectiveHandler` class to update the Bluetooth device database based on the change.

* It provides the screen for the user to give consent to contact uploads. It also provides the associated business logic during Alexa setup. If the user chooses a connected Bluetooth device as a primary device, Alexa setup displays the contacts consent screen to give the `Skip` and `Yes` options for the user to give or decline consent to contact uploads.
   The Alexa communication settings screen displays a list of connected devices with contacts upload permission status. The user can turn on or turn off the contacts upload permission for each device.

* It handles contacts upload or removal requests to Alexa.
   This library starts the AACS contacts service intents to handle contacts upload or removal requests.

## Prerequisites
The following list describes the prerequisites for this library:

* AACS must be built with the AACS telephony library.
* Your Android device must meet the prerequisites for using the AACS telephony library.