package com.amazon.alexa.auto.voiceinteraction.config

data class AACSConfigurationDeviceInfo (
    var clientId : String,
    var productId: String,
    var deviceSerialNumber: String,
    var manufacturerName: String,
    var description: String
)

data class AACSConfigurationGeneral (
    var startServiceOnBootEnabled: Boolean
)

data class AACSConfigurationIntentHandler (
    val type: String, // SERVICE/ACTIVITY/RECEIVER
    val androidPackage: String,
    val javaClass: String
)

data class AACSConfigurationAudioInputType (
    var useExternalSource: Boolean,
    var audioSource: String,
    var externalSource: AACSConfigurationIntentHandler?
)

data class AACSConfigurationAudioOutputType (
    var useDefault: Boolean
)

data class AACSConfigurationAudioInputTypes (
    val voice: AACSConfigurationAudioInputType
)

data class AACSConfigurationAudioOutputTypes (
    val tts: AACSConfigurationAudioOutputType,
    val alarm: AACSConfigurationAudioOutputType,
    val music: AACSConfigurationAudioOutputType,
    val notification: AACSConfigurationAudioOutputType,
    val earcon: AACSConfigurationAudioOutputType,
    val ringtone: AACSConfigurationAudioOutputType
)

data class AACSConfigurationPlatformHandlers (
    var useDefaultLocationProvider: Boolean,
    var useDefaultNetworkInfoProvider: Boolean,
    var useDefaultExternalMediaAdapter: Boolean,
    val audioInput: AACSConfigurationAudioInputTypes
)

/**
 * Part of the AACS configuration that is presented in Preference
 * Screen.
 */
data class AACSConfiguration (
    val generalConfig: AACSConfigurationGeneral,
    val deviceInfo: AACSConfigurationDeviceInfo,
    val platformHandlers: AACSConfigurationPlatformHandlers
)