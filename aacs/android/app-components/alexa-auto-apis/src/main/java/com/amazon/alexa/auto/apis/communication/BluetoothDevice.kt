package com.amazon.alexa.auto.apis.communication

data class BluetoothDevice (
        val deviceAddress : String,
        val deviceName: String,
        val contactsUploadPermission: Boolean
)