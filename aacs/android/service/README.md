#  Configuration Reference for AACS (Alexa Auto Client Service)

## Overview
This document explains the various fields of the AACS configuration. The AACS configuration is similar to the Auto SDK configuration, with a few additional fields unique to AACS.

## Auto SDK Modules
You configure a module in AACS in a similar way as you configure a module in the Auto SDK. For example, Auto SDK specifies a module in the configuration as `aace.<module>`, and AACS specifies a module in the configuration as `aacs.<module>`. See the [Auto SDK module documentation](https://alexa.github.io/alexa-auto-sdk/docs/explore/features) for information about the Auto SDK configuration for each module.

The following example shows the syntax for configuring the CBL module in AACS, which is the same as the syntax in the Auto SDK: 
~~~
{
    "aacs.cbl" : {
        "enableUserProfile": false
    }
}
~~~

Optionally, you can also configure the timeout value of AASB synchronous messages in `aacs.messageBroker`, as detailed in `Core` module documentation for configuring the MessageBroker. The default timeout duration is 500 ms. 
~~~
{
    "aacs.messageBroker": {
        ...
        "defaultMessageTimeout": 1000
    }
}
~~~

The `aacs.alexa` module has a structure that is slightly different from its Auto SDK counterpart as explained in the following list:

* In AACS, there is no 
`avsDeviceSDK` node, because most of the configuration required for this section is done within AACS. The only required configuration
from `avsDeviceSDK` is `deviceInfo`, and it is specified directly under `aacs.alexa`.
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

Specifies which local media sources are available and handled in the application. If your application prefers AACS to handle the local media sources, use `useDefaultLocalMediaSource` instead.

#### types
**Type:** String Array

Specifies the available local media sources. Possible values are `BLUETOOTH`, `USB`, `FM_RADIO`, `AM_RADIO`, `SATELLITE_RADIO`, 
`LINE_IN`, `COMPACT_DISC`, `SIRIUS_XM`, `DAB`, and `DEFAULT`

## AACS Module Enablement
AACS allows your application to enable/disable certain modules using AACS configuration file. To enable or disable certain modules,
add certain JSON blocks to the AACS configuration inside "aacs.modules" block, as shown in the following examples:

- APL module is disabled by default. To enable APL, add the following configuration to the configuration file:

```
"aacs.modules": {
    "aacs.apl": {
        "APL": {
            "enabled": true
        }
    }
}
```

- Custom Domain module is disabled by default. Similarly to APL, to enable Custom Domain, add the following configuration to `aacs.modules` in the configuration file:
```
"aacs.customDomain": {
  "CustomDomain": {
      "enabled": true
  }
}
```
>**Note**: If Custom Domain module is enabled, you must provide a valid `aacs.customDomain` configuration to configure the engine with your custom interfaces. Otherwise, the Engine will fail to start. See the `Custom Domain` module documentation for the required configuration.  

- CBL interface is deprecated in version 3.1, your application should use the Authorization interface instead for both alexa:cbl and alexa:auth_provider services.

If you want to disable CBL module  add the following configuration to `aacs.modules` in the configuration file:
```
"aacs.cbl": {
  "CBL": {
    "enabled": false
  }
}
```
If your application wants to disable the `Authorization` module in the configuration:
```
"aacs.authorization": {
  "Authorization": {
    "enabled": false
  }
}
```
**Note**: Both CBL and Alexa Authorization modules are needed to support alexa:cbl authorization

For configuring module enablement, more information can be found in the `Core` module documentation.

## General
`aacs.general` is used for most configurable values of AACS that are not required for the Auto SDK, as shown in the following example: 
~~~
{
    "aacs.general" : {
        "version": "1.0",
        "persistentSystemService": false,
        "startServiceOnBootEnabled": true,
        "syncSystemPropertyChange": false,
        "intentTargets" : {...}
    }
}
~~~
### version
**Type:** String
Indicates the version of AACS to be used. Releases of AACS will have current and minimum-supported versions. Versions outside this range will not be compatible and the service will not start as a result.

>**Note:** `persistentSystemService` configuration is deprecated. You no longer need to specify this field to run AACS as a persistent system service. If you have root access on the device and your application containing AACS AAR is a system application, then AACS is run as a system service.

### startServiceOnBootEnabled
**Type:** Boolean

When `startServiceOnBootEnabled` is set to `true`, AACS automatically starts running when the device is booted up. For the service to start on boot, it must have been run at least once after it was installed. When `startServiceOnBootEnabled` is set to `false`, AACS requires the application to send an intent to start the service. 

### syncSystemPropertyChange
**Type:** Boolean

This field is optional. When `syncSystemPropertyChange` is set to `true`, AACS handles synchronizing the time zone and locale settings of Alexa with the device settings so your application does not need to implement this feature if it is expected in your UX. When it's not present, it's default to `false`.

### updateSystemPropertyAllowed
**Type:** Boolean

This field is optional. When `updateSystemPropertyAllowed` is set to `true`, AACS updates the system settings if the corresponding Alexa's property is changed, only if AACS AAR is in a system application. For this release, only time zone property is supported. When it's not present, it's default to `false`.

### intentTargets
**Type:** JSON Object

This field is optional. Specifies a target for messages of every topic. There is a JSON object for every message topic supported in AACS, where the key is the topic name. 
>**Note:** If there are any extensions that require message handling, their topic will need to be added here.

#### package
**Type:** String

Specifies the package name of the application that receives messages for this particular module.

#### class

**Type:** String

Specifies the class name within the application that receives messages for this particular module.

## Default Platform Handlers
AACS provides the default platform implementation for certain services, which you can enable through the configuration in 
`aacs.defaultPlatformHandlers`. For a full explanation of default platform handlers, see the [AACS documentation](../README.md#default-platform-implementation).
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

### useDefaultCustomDomainMessageDispatcher
**Type:** Boolean

Set to `true` to enable the default `CustomDomain` message dispatcher in AACS. If `CustomDomain` module is enabled and `useDefaultCustomDomainMessageDispatcher` is set to false, your application must separately handle the messages for topic `CustomDomain`.

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

If you do not specify a value, the default audio source is  `MediaRecorder.AudioSource.MIC`. It is recommended to set `handleAudioFocus` to `true`. This will ensure that When Alexa is in `LISTENING`, `THINKING` and `EXPECTING` state, AACS will request audio focus resulting in other playing media to be ducked or paused.

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

> **Note:** When specifying both `VOICE` and `COMMUNICATION`'s  `audioSource` values as non-`EXTERNAL`, be sure that their `audioSource` values are the same. 

### audioOutput
**Type:** JSON Object

Configures `audioOutput` in AACS based on the audio type. This JSON object consists of JSON nodes for audio types that contain this information. Available 
audio types are `TTS`, `ALARM`, `NOTIFICATION`, `EARCON`, and `RINGTONE`.

> **Note:** AACS default audio output does not support `MUSIC` type of audio output. See [alexa-auto-media-player README](../app-components/alexa-auto-media-player/README.md) for more information on the `MUSIC` type implementation.

#### useDefault
**Type:** Boolean

Set to `true` to enable the default `AudioOutput` platform implementation for the given audio type. If `useDefault` is set to `false`, `AudioOutput` for the given audio
type must be handled in your application.

### useDefaultLocalMediaSource

**Type:** Boolean
(Followed with detailed `localMediaSourceMetadata` JSON array configuration if set `true`)

Set to `true` to enable the default `LocalMediaSource` platform implementation to configure local media sources. By default `useDefaultLocalMediaSource` is treated `false` so if not included in the config file or set to `false` explicitly, please define `localMediaSource` JSON array in the `aacs.alexa` node to enable AASB LocalMediaSource messages to be delivered to your application.
Refer the following sample configuration for the `useDefaultLocalMediaSource`.
~~~
    "useDefaultLocalMediaSource" : true,
    "localMediaSourceMetadata": [
      {
        "sourceType":"BLUETOOTH",
        "supported": true,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"USB",
        "supported": true,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"FM_RADIO",
        "supported": true,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>",
        "supportsSetPreset": true,
        "supportsSetFrequency": true
      }, {
        "sourceType":"AM_RADIO",
        "supported": false,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>",
        "supportsSetPreset": true,
        "supportsSetFrequency": true
      }, {
        "sourceType":"SATELLITE_RADIO",
        "supported": false,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"LINE_IN",
        "supported": false,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"COMPACT_DISC",
        "supported": true,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"SIRIUS_XM",
        "supported": false,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      }, {
        "sourceType":"DAB",
        "supported": false,
        "mediaPackageName":"<Package Name>",
        "mediaServiceClass":"<Media Browser service class name>"
      },{
        "sourceType":"DEFAULT",
        "supported": true,
        "mediaPackageName":"",
        "mediaServiceClass":"",
        "supportsSetPreset": true
      }
    ]
~~~
`sourceType` Specifies the available local media sources. Possible values are `BLUETOOTH`, `USB`, `FM_RADIO`, `AM_RADIO`, `SATELLITE_RADIO`, 
`LINE_IN`, `COMPACT_DISC`, `SIRIUS_XM`, `DAB`, and `DEFAULT`. The `DEFAULT` source provides the facility to support all the media sources which are not listed in the given list.

>**Note:** This feature uses [NotificationListenerService](https://developer.android.com/reference/android/service/notification/NotificationListenerService) to monitor active sessions, provide [BIND_NOTIFICATION_LISTENER_SERVICE permission](https://developer.android.com/reference/android/Manifest.permission#BIND_NOTIFICATION_LISTENER_SERVICE) to the application which includes AACS AAR to support the default local media source handling. If access is not provided, AACS would ignore the `"useDefaultLocalMediaSource" : true` configuration. This access is generally given by enabling the application with AACS AAR in `Settings >> Apps >> Special Access >> Notification access`.

>**Note:** If OEM wishes to make the application with AACS AAR as a system application, they can avoid the `Notification Access` step. Please add a line `<uses-permission android:name="android.permission.MEDIA_CONTENT_CONTROL" />` in the AACS `AndroidManifest.xml` file and provide all the required permissions to the system application in the Android operating system.

Refer `Local Media Source` interface documentation `DEFAULT` media source.
`supported` configures the given Local Media Source. if `supported` is set `true`, that media source would be handled and controlled through AACS. If `supported` is set `false`, AACS would ignore the media source.

`mediaPackageName` and `mediaServiceClass` are mandatory configuration keys.  `mediaPackageName` represents the package name of the media source and `mediaServiceClass` represents the The name of the class inside of package that implements the component of the media browser service. This is a requirement of the [ComponentName](https://developer.android.com/reference/android/content/ComponentName). Please ensure that right data is provided here. Since `DEFAULT` player can act on behalf of all latest the media sources except Alexa music, MACC supported players and other configured local media sources, it is not full time associated to any package name and MediaBrowserService. It always represents 0th media controller of the [onActiveSessionsChanged](https://developer.android.com/reference/android/media/session/MediaSessionManager.OnActiveSessionsChangedListener#onActiveSessionsChanged(java.util.List%3Candroid.media.session.MediaController%3E)) controller list.

Besides these mandatory configuration keys, following optional keys are useful for the correct mapping of metadata.

`metadataTitleKey` By default AACS uses [METADATA_KEY_TITLE](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_TITLE) to extract the title data from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

For example, Consider sub title as title for some reason
~~~
"metadataTitleKey":"android.media.metadata.DISPLAY_SUBTITLE" 
~~~ 

`metadataTrackIdKey` By default AACS uses [METADATA_KEY_MEDIA_ID](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_MEDIA_ID) to extract the trackId from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

`metadataTrackNumberKey` By default AACS uses [METADATA_KEY_TRACK_NUMBER](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_TRACK_NUMBER) to extract the track number from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

`metadataArtistKey` By default AACS uses [METADATA_KEY_ARTIST](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_ARTIST) to extract the artist from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

`metadataAlbumKey` By default AACS uses [METADATA_KEY_ALBUM](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_ALBUM) to extract the title data from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

`metadataDurationKey` By default AACS uses [METADATA_KEY_DURATION](https://developer.android.com/reference/android/media/MediaMetadata#METADATA_KEY_DURATION) to extract the title data from the Local Media Source. If any of the media source provides this data through the different key, use this configuration field.

`supportsSetFrequency` This takes a boolean value. It should be set `true` for AM or FM where Alexa can set the frequency in the AM or FM application.

Local Media Player like FM or AM application should be able to handle this request. To support it, these app needs to implement [onPrepareFromSearch](https://developer.android.com/reference/kotlin/android/support/v4/media/session/MediaSessionCompat.Callback#onpreparefromsearch) and [onPlayFromSearch](https://developer.android.com/reference/kotlin/android/support/v4/media/session/MediaSessionCompat.Callback#onplayfromsearch) methods. They will receive a query string containing a json in following format.
~~~
{
    "ContentSelector":"FREQUENCY",
    "payload":"98.7 FM HD 1"
}
~~~ 
>**Note** Refer `Local Media Source` interface documentation for more information of the ContentSelector and payload.

`supportsSetPreset` This takes a boolean value. It should be set `true` if media source can play media by preset number.
>**Note** `onPrepareFromSearch` and `onPlayFromSearch` related details given in the above section are applicable for content type PRESET as well.

`supportsSetChannel` This takes a boolean value. It should be set `true` if media source like Sirius XM which can play media by channel name.
>**Note** `onPrepareFromSearch` and `onPlayFromSearch` related details given in the above section are applicable for content type CHANNEL as well.
