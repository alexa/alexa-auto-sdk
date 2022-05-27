# Alexa Auto Media Player

## Table of Contents
- [Alexa Auto Media Player](#alexa-auto-media-player)
- [Include Alexa Auto Media Player in the Application](#include-alexa-auto-media-player-in-the-application)
- [Enable Media Ducking](#enable-media-ducking)
- [Media Resume Alexa Music After Reboot](#media-resume-alexa-music-after-reboot)
- [Login from Android Automotive Media UI](#login-from-android-automotive-media-ui)
- [Alexa Music Certification](#alexa-music-certification)

## Alexa Auto Media Player
The following list describes the purposes of this package:

* It provides the audio player capability for Alexa Auto Client Service (AACS) by receiving all audio player intents and notifying AACS about the progress of media playback.
* It manages the underlying media player, which is ExoPlayer.
* It handles audio focus.
* It implements a media session on top of the media player so that media can be controlled with standard Android Media Session APIs. This capability allows Alexa Media to integrate with the Android Automotive Media UI.

## Include Alexa Auto Media Player in the Application
The Alexa Auto Media Player is by default enabled in the AACS Sample App. See the [AACS Sample App README](../../sample-app/README.md#building-and-signing-the-aacs-sample-app-apk) for build instructions.

If you want to use Alexa Auto Media Player in your application, build the following app components and include all the generated AARs in your application:

*  alexa-auto-apis
*  alexa-auto-apps-common-ui
*  alexa-auto-apps-common-util
*  alexa-auto-media-player

## Enable Media Ducking
You can enable audio ducking for the Alexa media using this configuration. By default, Alexa pauses `MUSIC` channel whenever Alexa `TTS` or `ALARM` channels are in the focus. Enabling ducking allows `MUSIC` channel to remain in the playing state when high priority channels like `TTS` and `ALARM` are active. For enabling ducking, please provide the following configuration:

>**Note:** Enable ducking only if your variant of android platform supports music stream ducking when TTS stream gets the audio focus. Those platforms do not provide a flag [AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK](https://developer.android.com/reference/android/media/AudioManager#AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK) [onAudioFocusChange](https://developer.android.com/reference/android/media/AudioManager.OnAudioFocusChangeListener#onAudioFocusChange(int)) is called and instead of that they provide a flag [AUDIOFOCUS_LOSS_TRANSIENT](https://developer.android.com/reference/android/media/AudioManager#AUDIOFOCUS_LOSS_TRANSIENT). In such cases, it is recommended to disable the ducking to avoid unexpected audio behavior.

```JSON
{
    "aacs.alexa" : {
        "audioOutputType.music": {
            "ducking": {
                "enabled" : true
            }
        }
    }
}
```

## Media Resume Alexa Music After Reboot
Please refer to [Media Resume Last Playing Media After Platform Reboot](../../sample-app/README.md#media-resume-last-playing-media-after-platform-reboot) for the details about media resume feature. This feature works out of the box on Automotive Android OS with this component. Following configuration is required to enable and use this feature:

```JSON
"aacs.alexa": {
  "requestMediaPlayback": {
      "mediaResumeThreshold": 50000
    }
}
```

## Login from Android Automotive Media UI
This library provides an optional feature that enables the Android Automotive Media UI to display the "not authenticated" message if the app is not authenticated. It then offers the option for the user to invoke the login UI workflow.

To enable this feature in the app with this library, implement `AlexaApp`, a registry interface defined in the Alexa Auto APIs package, and resolve dependencies by using the following interfaces:

 * `AuthController`: This interface provides business logic to monitor the current authentication state (the value of `loggedIn`). The interface is made available from `AlexaAppRootComponent`.
 * `AlexaSetupController`: This interface enables the media UI to launch the login UI activity if the authentication state indicates that the user is not logged in.

>**Note:** See the [alexa-auto-apis README](../alexa-auto-apis/README.md) for more information about consuming and publishing implementations.

## Alexa Music Certification
This version of the Alexa Auto Media Player doesn’t meet all the Alexa Music Certification requirements on Android Automotive OS. It passes API validation for Amazon Music, Audible, Kindle, TuneIn Radio Live, TuneIn Radio Custom, Music Skills (Deezer & SiriusXM), iHeart Radio Live, and iHeart Radio Custom. Pandora doesn’t have an API validation requirement.

The Media Player doesn’t pass Music Service Provider (MSP) logo attribution GUI validation for Amazon Music, TuneIn Radio Live, TuneIn Radio Custom, Music Skills (Deezer & SiriusXM), iHeart Radio Live, iHeart Radio Custom, and Pandora. Audible and Kindle don't have a GUI validation requirement. The Android Automotive OS doesn't have placeholder for showing MSP logo which is required for passing GUI validation. The Media Player provides text based MSP attribution. 

The Android Automotive OS displays album art as the background image in media player screen, which doesn't pass GUI validation of not altering album art in any way.

Due to the missing media controls in TemplateRuntime RenderPlayerInfo payload, the Media Player fails to display all the required media controls, which doesn't pass media controls GUI validation.

The standard certification process is required and simplified by the Alexa Auto Media Player because the above mentioned API validation has been completed. Contact your Solutions Architect (SA) or Partner Manager for information about how to obtain certification.

## Known Issues
* On low resolution screens, Music Service Provider (MSP) name may cut-off when artist string is long.