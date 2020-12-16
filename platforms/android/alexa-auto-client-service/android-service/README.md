#  Configuration Reference for AACS (Alexa Auto Client Service)
This document explains the various fields of the AACS configuration. The AACS configuration is similar to the Auto SDK configuration, 
with a few additional fields unique to AACS.  
<!-- omit in toc -->
## Table of Contents

- [Auto SDK Modules](#auto-sdk-modules)
  - [localMediaSource](#localmediasource)
- [General](#general)
  - [persistentSystemService](#persistentsystemservice)
  - [startServiceOnBootEnabled](#startserviceonbootenabled)
  - [intentTargets](#intenttargets)
- [Default Platform Handlers](#default-platform-handlers)
  - [useDefaultLocationProvider](#usedefaultlocationprovider)
  - [useDefaultNetworkInfoProvider](#usedefaultnetworkinfoprovider)
  - [useDefaultExternalMediaAdapter](#usedefaultexternalmediaadapter)
  - [useDefaultPropertyManager](#usedefaultpropertymanager)
  - [audioInput](#audioinput)
  - [audioOutput](#audiooutput)

## Auto SDK Modules
You configure a module in AACS in a similar way as you configure a module in the Auto SDK. For example,  the Auto SDK specifies a module in the configuration as `aace.<module>`, and AACS specifies a module in the configuration as `aacs.<module>`. See the [Auto SDK's 
documentation](https://alexa.github.io/alexa-auto-sdk/docs/cpp/annotated.html) for information about the Auto SDK configuration.

The following example shows the syntax for configuring the CBL module in AACS, which is the same as the syntax in the Auto SDK: 
~~~
{
    "aacs.cbl" : {
        "enableUserProfile": false
    }
}
~~~

The `aacs.aasb` module requires a field `version`, as detailed in [AASB README](../../../../extensions/aasb/README.md).
~~~
{
    "aacs.aasb": {
        "version": "3.1"
    }
}
~~~

The `aacs.alexa` module has a structure that is slightly different from its Auto SDK counterpart as explained in the following list:

* In AACS, there is no 
`avsDeviceSDK` node, because most of the configuration required for this section is done within AACS. Because the only required configuration
from `avsDeviceSDK` is `deviceInfo`, it is specified directly under `aacs.alexa`.
* `aacs.alexa` contains a node called `localMediaSource`, which explicitly specifies which media sources are available.
  
The following example shows  `deviceInfo` and `localMediaSource` in the configuration for `aacs.alexa`: 
~~~
{
    "aacs.alexa": {
        "deviceInfo": {
            "clientId": "",
            "productId": "",
            "deviceSerialNumber": "",
            "manufacturerName": "",
            "description": ""
        },
        "localMediaSource": {
            "types": ["FM_RADIO"]
        }
    }
}
~~~

### localMediaSource
**Type:** JSON Object

Specifies which local media sources are available and handled in the application.

#### types
**Type:** String Array

Specifies the available local media sources. Possible values are `BLUETOOTH`, `USB`, `FM_RADIO`, `AM_RADIO`, `SATELLITE_RADIO`, 
`LINE_IN`, `COMPACT_DISC`, `SIRIUS_XM`, and `DAB`.

## General
`aacs.general` is used for most configurable values of AACS that are not required for the Auto SDK, as shown in the following example: 
~~~
{
    "aacs.general" : {
        "version": "1.0",
        "persistentSystemService": false,
        "startServiceOnBootEnabled": true,
        "intentTargets" : {
            "AASB" : {
                "type": [],
                "package": [],
                "class": []
            },
            "APL" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AddressBook" : {
                "type": [],
                "package": [],
                "class": []
            },
            "Alerts" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AlexaClient" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AlexaSpeaker" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AudioInput" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AudioOutput" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AudioPlayer" : {
                "type": [],
                "package": [],
                "class": []
            },
            "AuthProvider" : {
                "type": [],
                "package": [],
                "class": []
            },
            "Authorization" : {
                "type": [],
                "package": [],
                "class": []
            },
            "CarControl" : {
                "type": [],
                "package": [],
                "class": []
            },
            "CBL" : {
                "type": [],
                "package": [],
                "class": []
            },
            "Connectivity" : {
                "type": [],
                "package": [],
                "class": []
            },
            "DoNotDisturb" : {
                "type": [],
                "package": [],
                "class": []
            },
            "EqualizerController" : {
                "type": [],
                "package": [],
                "class": []
            },
            "ExternalMediaAdapter" : {
                "type": [],
                "package": [],
                "class": []
            },
            "GlobalPreset" : {
                "type": [],
                "package": [],
                "class": []
            },
            "LocalMediaSource" : {
                "type": [],
                "package": [],
                "class": []
            },
            "LocationProvider" : {
                "type": [],
                "package": [],
                "class": []
            },
            "Navigation" : {
                "type": [],
                "package": [],
                "class": []
            },
            "NetworkInfoProvider" : {
                "type": [],
                "package": [],
                "class": []
            },
            "Notifications" : {
                "type": [],
                "package": [],
                "class": []
            },
            "PhoneCallController" : {
                "type": [],
                "package": [],
                "class": []
            },
            "PlaybackController" : {
                "type": [],
                "package": [],
                "class": []
            },
            "PropertyManager" : {
                "type": [],
                "package": [],
                "class": []
            },
            "SpeechRecognizer" : {
                "type": [],
                "package": [],
                "class": []
            },
            "TemplateRuntime" : {
                "type": [],
                "package": [],
                "class": []
            },
            "TextToSpeech" : {
                "type": [],
                "package": [],
                "class": []
            }
        }
    }
}
~~~
### version
**Type:** String
Indicates the version of AACS to be used. Releases of AACS will have current and minimum-supported versions. Versions outside this range will not be compatible and the service will not start as a result.

### persistentSystemService
**Type:** Boolean

When `persistentSystemService` is set to `false` (default), AACS starts as a foreground service, which has higher priority on the Android system. If you have 
system-level control over your device, you may set this field to `true` to run AACS as a truly persistent service. Doing so also hides 
the AACS notification that is displayed when the service is run in the foreground.

### startServiceOnBootEnabled
**Type:** Boolean

When `startServiceOnBootEnabled` is set to `true`, AACS automatically starts running when the device is booted up. For the service to start on boot, it must have been run at least once after it was installed. When `startServiceOnBootEnabled` is set to `false`, AACS requires the application to send an intent to start the service. 

### intentTargets
**Type:** JSON Object

Specifies a target for messages of every topic. There is a JSON object for every message topic supported in AACS, where the key is the topic name. 
>**Note:** If there are any extensions that require message handling, their topic will need to be added here.

#### package
**Type:** String

Specifies the package name of the application that receives messages for this particular module.

#### class

**Type:** String

Specifies the class name within the application that receives messages for this particular module.

## Default Platform Handlers
AACS provides the default platform implementation for certain services, which you can enable through the configuration in 
`aacs.defaultPlatformHandlers`. For a full explanation of default platform handlers, see the [AACS README](../README.md).
~~~
"aacs.defaultPlatformHandlers": {
    "useDefaultLocationProvider": true,
    "useDefaultNetworkInfoProvider": true,
    "useDefaultExternalMediaAdapter": true,
    "useDefaultPropertyManager": true,
    "audioInput": {
        "audioType": {
            "VOICE": {
                "useDefault": true,
                "audioSource": "MediaRecorder.AudioSource.MIC"
            },
            "COMMUNICATION": {
                "useDefault": true,
                "audioSource": "MediaRecorder.AudioSource.MIC",
            }
        }
    },
    "audioOutput": {
        "audioType": {
            "TTS": {
                "useDefault": true
            },
            "ALARM": {
                "useDefault": true
            },
            "MUSIC": {
                "useDefault": false
            },
            "NOTIFICATION": {
                "useDefault": true
            },
            "EARCON": {
                "useDefault": true
            },
            "RINGTONE": {
                "useDefault": true
            },
            "COMMUNICATION": {
                "useDefault": true
            }
        }
    }
}
~~~

### useDefaultLocationProvider
**Type:** Boolean

Set to `true` to enable the default `LocationProvider` platform implementation in AACS. If `useDefaultLocationProvider` is set to `false`, your application must separately handle the messages for this topic. 

### useDefaultNetworkInfoProvider
**Type:** Boolean

Set to `true` to enable the default `NetworkInfoProvider` platform implementation in AACS. If `useDefaultNetworkInfoProvider` is set to `false`, your application must separately handle the messages for this topic.

### useDefaultExternalMediaAdapter
**Type:** Boolean

Set to `true` to enable the default `ExternalMediaAdapter` platform implementation in AACS. If `useDefaultExternalMediaAdapter` is set to `false`, your application must separately handle the messages for this topic.

### useDefaultPropertyManager
**Type:** Boolean

Set to `true` to enable the default `PropertyManager` platform implementation in AACS. This enables synchronous managing of properties using AACS's ContentProvider. If `useDefaultPropertyManager` is set to `false`, your application must separately handle the messages for this topic.

### audioInput
**Type:** JSON Object

Configures `AudioInput` in AACS based on the audio type. This JSON object consists of JSON nodes for the audio types that contain this information. Available audio types are `COMMUNICATION` and `VOICE`.

#### useDefault
**Type:** Boolean

Set to `true` to enable the default `AudioInput` platform implementation for the given audio type. If `useDefault` is set to `false`, `AudioInput` for the given audio type must be handled in your application.

#### audioSource
**Type:** String

Specifies the Android audio source for the given audio type. This assumes that the `useDefault` field is set to `true`. Available Audio sources are 
`MediaRecorder.AudioSource.MIC`, `MediaRecorder.AudioSource.DEFAULT`, `MediaRecorder.AudioSource.VOICE_RECOGNITION`,
`MediaRecorder.AudioSource.VOICE_COMMUNICATION` and `EXTERNAL`.

If you do not specify a value, the default audio source is  `MediaRecorder.AudioSource.MIC`.

Using `EXTERNAL` for `audioSource` means AACS fetches the audio stream from an external application, and it requires a valid `externalSource` object to be present in the JSON object.
The following sample configuration is for an external stream:
~~~
        "VOICE": {
          "useDefault": true,
          "audioSource": "EXTERNAL",
          "externalSource": {
            "type": "ACTIVITY",
            "package": "com.example.application",
            "class": ".MainActivity"
          }
        }
~~~

**Note**: When specifying both `VOICE` and `COMMUNICATION`'s  `audioSource` values as non-`EXTERNAL`, be sure that their `audioSource` values are the same. 

### audioOutput
**Type:** JSON Object

Configures `audioOutput` in AACS based on the audio type. This JSON object consists of JSON nodes for audio types that contain this information. Available 
audio types are `TTS`, `ALARM`, `MUSIC`, `NOTIFICATION`, `EARCON`, and `RINGTONE`.

#### useDefault
**Type:** Boolean

Set to `true` to enable the default `AudioOutput` platform implementation for the given audio type. If `useDefault` is set to `false`, `AudioOutput` for the given audio
type must be handled in your application.
