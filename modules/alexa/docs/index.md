# Alexa Module

## Overview

The Alexa Auto SDK Alexa module provides interfaces for standard Alexa features. The Engine handles steps to send events and sequence directives so you can focus on using the provided AASB messages to interact with Alexa.

> **Important!:** Not every interface of the `Alexa` module documentation is updated to reflect the Auto SDK 4.0 Message Broker message API. Some pages still include text, code samples, or diagrams that show deprecated platform interfaces rather than their corresponding AASB message interface equivalents. Your application will use the the AASB message interfaces with MessageBroker. The `Alexa` documentation will be fully updated in the next Auto SDK version.

## Configure the Alexa module

The `Alexa` module defines required and optional configuration objects that you include in the Engine configuration for your application. You can define the configuration objects in a file or construct them programmatically with the relevant configuration factory functions.

Your application must provide the `aace.alexa` configuration in the same format as the example specified below. Alternatively, use the [`AlexaConfiguration`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html) factory functions to generate individual elements of this configuration.

```
{
    "aace.alexa": {
        "avsDeviceSDK": {
            "deviceInfo": {
                "clientId": "${CLIENT_ID}",
                "productId": "${PRODUCT_ID}",
                "deviceSerialNumber": "${DEVICE_SERIAL_NUMBER}",
                "manufacturerName": "${MANUFACTURER_NAME}",
                "description": "${DEVICE_DESCRIPTION}"
            },
            "libcurlUtils": {
                "CURLOPT_CAPATH": "${CERTS_PATH}"
            },
            "miscDatabase": {
                "databaseFilePath": "${DATA_PATH}/miscDatabase.db"
            },
            "certifiedSender": {
                "databaseFilePath": "${DATA_PATH}/certifiedSender.db"
            },
            "alertsCapabilityAgent": {
                "databaseFilePath": "${DATA_PATH}/alertsCapabilityAgent.db"
            },
            "notifications": {
                "databaseFilePath": "${DATA_PATH}/notifications.db"
            },
            "capabilitiesDelegate": {
                "databaseFilePath": "${DATA_PATH}/capabilitiesDatabase.db"
            },
            "deviceSettings": {
                "databaseFilePath": "${DATA_PATH}/deviceSettings.db",
                "locales": ["{{STRING}}", ...],
                "defaultLocale":"en-US",
                "localeCombinations": [
                    ["{{STRING}}","{{STRING}}"],
                    ["{{STRING}}", "{{STRING}}"],
                    ...
                ],
                "defaultTimezone":"America/Vancouver"
            }
        },
        "requestMediaPlayback": {
            "mediaResumeThreshold": 20000
        }
    },
    "aasb.alexa": {
      "LocalMediaSource": {
        "types": ["FM_RADIO", "AM_RADIO","BLUETOOTH", "USB", "SATELLITE_RADIO",
                    "LINE_IN", "COMPACT_DISC", "DAB", "DEFAULT"]
      }
    }
}
```

The `deviceInfo` field contains the details of the device. The fields `libcurlUtils`, `miscDatabase`, `certifiedSender`, `alertsCapabilityAgent`, `notifications`, and `capabilitiesDelegate` specify the respective database file paths.

The `deviceSettings` field specifies the settings on the device. The following list describes the settings:

* `databaseFilePath` is the path to the SQLite database that stores persistent settings. The database will be created on initialization if it does not already exist.
* `defaultLocale` specifies the default locale setting, which is Alexa's locale setting until updated on the device. The default value of `defaultLocale` is “en-US”.
* `locales` specifies the list of locales supported by the device. The default value is `["en-US","en-GB","de-DE","en-IN","en-CA","ja-JP","en-AU","fr-FR","it-IT","es-ES","es-MX","fr-CA","es-US", "hi-IN", "pt-BR"]`.
* `localeCombinations` specifies the list of locale pairs available on a device that supports multi-locale mode. Through the Dynamic Language Switching feature, Alexa can communicate with the user of such device in languages specified in the locale pairs. In each pair, the first value is the primary locale, which Alexa uses most often when interacting with the user. The second value is the secondary locale, which specifies an additional language that Alexa uses when responding to an utterance in the corresponding language. For example, if ["en-US", "es-US"] is declared in `localeCombinations` and the device specifies this pair as the current locale setting, Alexa primarily operates in English for the U.S. but can understand and respond to utterances in Spanish for the U.S., without requiring the device to update the locale setting.
  
    By default, `localeCombinations` is a list of the following combinations, which are also the supported combinations as of 2021-02-02. It is possible for the default value to be different from the list of supported combinations in the future. For updates to the supported combinations, see the [Alexa Voice Service documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/system.html#localecombinations).
    
    * ["en-US", "es-US"]
    * ["es-US", "en-US"]
    * ["en-IN", "hi-IN"]
    * ["hi-IN", "en-IN"]
    * ["en-CA", "fr-CA"]
    * ["fr-CA", "en-CA"]
    * ["en-US", "es-ES"]
    * ["es-ES", "en-US"]
    * ["en-US", "de-DE"]
    * ["de-DE", "en-US"]
    * ["en-US", "fr-FR"]
    * ["fr-FR", "en-US"]
    * ["en-US", "it-IT"]
    * ["it-IT", "en-US"]
    * ["en-US", "ja-JP"]
    * ["ja-JP", "en-US"]


    When a device operates in multi-locale mode, an application can select any locale pair in the list above as the locale setting if the following
    conditions are met:
    
    * The device's primary locale setting is the first locale in the selected pair. 
    * The device also supports the secondary locale in the pair.
    * The pair is specified in `localeCombinations`.
    
    **Note:** Dynamic Language Switching is only available in online mode. 

## Use the Alexa module interfaces

Explore the following interfaces to learn how to integrate Alexa features in your application.

### Invoke Alexa with SpeechRecognizer

Use `SpeechRecognizer` to capture the user's speech with the microphone when the user invokes Alexa.

[SpeechRecognizer interface>>](./SpeechRecognizer.md)

### Play Alexa speech to the user with SpeechSynthesizer

Use `SpeechSynthesizer` to provide an audio output channel for the Engine to play back Alexa's speech to the user.

[SpeechSynthesizer interface>>](./SpeechSynthesizer.md)

### Track Alexa state changes with AlexaClient

Use the `AlexaClient` interface to observe changes in Alexa's connection and attention state when building the UI for your application.

[AlexaClient interface>>](./AlexaClient.md)

### Start the out-of-box experience with DeviceSetup

Trigger an out-of-box introductory conversation with Alexa experience using the `DeviceSetup` interface.

[DeviceSetup interface>>](./DeviceSetup.md)

### Find things to try with FeatureDiscovery

Use `FeatureDiscovery` to display dynamic suggested utterances that help users discover new features.

[FeatureDiscovery interface>>](./FeatureDiscovery.md)

### Display cards on screen with TemplateRuntime

Provide a visual experience by building a UI based on the templates and media playback info provided by `TemplateRuntime`.

[TemplateRuntime interface>>](./TemplateRuntime.md)

### Stream Alexa media content with AudioPlayer

[AudioPlayer interface>>](./AudioPlayer.md)

### Press media playback control buttons with PlaybackController

[PlaybackController interface>>](./PlaybackController.md)

### Adjust equalizer settings with EqualizerController

[EqualizerController interface>>](./EqualizerController.md)

### Resume media playback at startup with MediaPlaybackRequestor

[MediaPlaybackRequestor interface>>](./MediaPlaybackRequestor.md)

### Control local media with LocalMediaSource

[LocalMediaSource interface>>](./LocalMediaSource.md)

### Deep link into external media apps with ExternalMediaAdapter

[ExternalMediaAdapter interface>>](./ExternalMediaAdapter.md)

### Control volume with AlexaSpeaker

[AlexaSpeaker interface>>](./AlexaSpeaker.md)

### Manage timers, alarms, and reminders with Alerts

[Alerts interface>>](./Alerts.md)

### Render notification indicators with Notifications

[Notifications interface>>](./Notifications.md)

### Block notifications with DoNotDisturb

[DoNotDisturb interface>>](./DoNotDisturb.md)