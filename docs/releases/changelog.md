# Change Log
## v4.1.1 released on 2022-08-08

### Enhancements
* Improved Auto SDK AACS Sample App Setup and Settings UX.
* Updated APL renderer app component, as well as dependent APL Viewhost Android libraries (AARs). It is highly recommended you update to release 4.1.1 for APL integrations.
>**Note:** All Auto SDK 4.1 extensions are compatible with 4.1.1.

### Resolved Issues
* Improved the settings menu by expanding the clickable area of settings, and added missing descriptions for menu items.
* Fixed a race condition in which updating the Alexa language setting, and then navigating away from the menu page could crash the application without switching the language.
* Fixed an issue in which the Alexa comms permission screen did not render properly. Improved the margin alignment issue in the setup screens.

### Known Issues
**General**

* The [Alexa Automotive UX guidelines](#https://developer.amazon.com/en-US/docs/alexa/alexa-auto/display-cards.html#dismiss-display-cards) specify when to automatically dismiss a `TemplateRuntime` display card for each template type. The Engine publishes the `TemplateRuntime` interface messages `ClearTemplate` and `ClearPlayerInfo` based on the timeouts configured in the `aace.alexa.templateRuntimeCapabilityAgent` Engine configuration. However, the configuration does not provide enough granularity to specify timeouts for different types of display cards. Consequently, there is no way for your application to configure automatically dismissing local search templates (e.g., `LocalSearchListTemplate2`) with a different timeout than other templates (e.g., `WeatherTemplate`). The configuration also does not provide a way for you to specify infinite timeout for `NowPlaying` cards. You must implement your application’s dismissal logic for display cards and media info accordingly.
* There is a rare race condition in which publishing the `AlexaClient.StopForegroundActivity` message does not cancel the active Alexa interaction. The race condition can happen when the application publishes the message at the beginning of the `THINKING` state `AlexaClient.DialogStateChanged` transition.

**Car control**

* If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in the set from Alexa. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, Alexa retains endpoint 1 from set A, which might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try

**Communications**

* If the user asks Alexa to redial the last called number when their phone is not connected to the head unit, Alexa is silent rather than prompting the user to connect their phone.

**Entertainment**

* When music is playing, repeatedly pressing the “next” button to advance in the playlist restarts the current song.
* When using the LVC extension, if the application publishes the `MediaPlaybackRequestor.RequestMediaPlayback` AASB message before the Auto SDK Engine connects to Alexa cloud, media playback will not automatically resume as expected. The workaround is to wait for the connection to Alexa cloud to complete before publishing the `RequestMediaPlayback` message.
* There is no AASB message to indicate to Alexa that the user switched the media player UI on the head unit from an Alexa-integrated local media source, such as FM radio, to Alexa cloud-based music service provider. The only way to switch the audio context between the two player types is through voice interaction explicitly requesting a particular player.
* If your application cancels an Alexa interaction by sending the `AlexaClient.StopForegroundActivity` message to the Engine during music playback, the Engine might erroneously request your application to dismiss the `NowPlaying` media info by publishing the `TemplateRuntime.ClearPlayerInfo` message. Your application should not dismiss the media info in this scenario.

**Local Voice Control**

* In offline mode with LVC, after the user requests a list of POIs with an utterance such as “Alexa, find a nearby Starbucks”, Alexa does not recognize follow up requests such as "Alexa, select the first one" and does not display or read detailed information about the requested selection.

**AACS sample app**

* Sometimes the sample app will display an error page during sign-in if the user launched the app with the launcher icon. The recommended workaround is to set Alexa as the default assistant in the settings menu to guarantee AACS initializes properly before sign in.
* APL Card is prematurely closed if there is music playing in the background and APL command `SpeakItem` or `SpeakList`is executed.
* The voice interaction UI does not match the automotive UX guidelines for touching the screen during the interaction. The UX guidelines state that the interaction should continue if the user taps or scrolls, but the sample app cancels the interaction when the user taps or scrolls.
* The volume Alexa uses to read a shopping list is louder than the volume set for other Alexa responses.
* When the device has internet disconnected and the user sets the system language to a language not supported by Alexa, the sample app does not always display the language selection screen automatically.
* When the user revokes Alexa permission to use the microphone and then re-enables the permission, Alexa does not respond to utterances until the user restarts the app.
* If an alert is going off while Alexa is speaking, the timer audio cancels the Alexa speech. timer is going off while Alexa is speaking, the timer audio cancels the Alexa speech.

**C++ sample app**

* The sample app may fail to handle synchronous-style `AASB messages` within the required timeout to construct device context for Alexa. As a result, some utterances may not work as expected.

## v4.1.0 released on 2022-05-27

#### Enhancements

**Auto SDK**

* Added support to display a smart home dashboard using voice requests such as “Alexa, show me my smart home dashboard”. The user can use the dashboard to monitor and control the states of their smart home devices such as lights, plugs, switches, and thermostats. The smart home dashboard is powered by APL and updates to reflect the vehicle’s settings for day and night mode, custom themes, and driving state.
* Added the Feature Discovery interface, enabling users to learn about Alexa features by providing an API for your application to retrieve and display suggested utterances dynamically. For more information, see the [Feature Discovery documentation](../explore/features/alexa/FeatureDiscovery.md).
* Deprecated the Local Voice Control (LVC) APK in favor of a new LVC AACS App Component that integrates LVC directly into AACS. See the Local Voice Control extension documentation for more information.
* Enhanced LVC offline local search to support voice-based search and navigation to cheap gas stations and electric vehicle charging stations for en-US locale.
* Enhanced LVC offline navigation feature to show/navigate to previous destinations, show alternate route,  route-based ETA, and  add/remove waypoints to POI, user favorites, and address for en-US locale.
* Updated Alexa to disallow Alexa Presentation Language (APL), directives from skills that are not explicitly certified as safe for automotive. Certified skills may render APL, and uncertified skills fall back to experiences with TemplateRuntime (if supported) or voice-only.
* Updated the aace.vehicle.info configuration to include two additional optional fields, engineType and rseEmbeddedFireTvs. See the [Core module documentation](../explore/features/core/index.md) for additional information.
* Updated the Auto SDK Builder Tool to use a custom Python interpreter /usr/bin/env python3 rather than the previously hardcoded /usr/bin/python3
* Updated the Auto SDK Builder Tool to use the additional compiler and linker flags that enable exploit mitigation techniques, including safe stack, stack canary, fortifying source, and RELRO.
* Updated the following dependency versions:
    * [Android NDK r21e](https://github.com/android/ndk/wiki/Changelog-r21)
    * [Curl 7.81.0](https://curl.se/changes.html#7_81_0)
    * [SQLite 3.37.2](https://www.sqlite.org/releaselog/3_37_2.html)
* The Auto SDK build system was updated to support QNX 7.0 and QNX 7.1 SDP cross-compilation.
* Added the option `libcurl:openssl_version` to the Auto SDK build system recipes to specify the `OpenSSL` version.
* Enhanced Auto SDK logs to display thread ID and uses different colors per log level.
* Added support to build Auto SDK using a custom toolchain. See the [Build Alexa Auto SDK documentation](../native/building.md) for more information.

**AACS Sample App for Android Automotive OS**

* Added an Alexa app icon that allows users to launch the AACS sample app directly from the app launcher instead of the system settings menu.
* Added Things-to-try in setting menu that displays a list of utterances for user to try out in different domains.
* Enhanced the Navigation app component that provides a plugin framework for 3P navigation provider. See Alexa Auto Navigation app-component for details.
* Interruption Behavior - Push-to-talk (PTT) now interrupts while Alexa is speaking/thinking, and cancels when Alexa is listening. Barge-in sounds now play, in the previous version the new dialog would start silently.
* Alexa setup flow is now interrupted if the vehicle is in motion, and setup flow is resumed when the vehicle returns to the parked state. Implemented a BACK button that returns the user to the previous activity when pressed.
* Enhanced the contacts sharing consent UI to display the consent screen when a new phone is paired and to persist consent for subsequent pairings. Previously, the consent UI only displayed as part of the setup flow.
* Enhanced the communications screen UI to display all paired phones instead of only connected phones. This enables the user to enable or disable contacts from a paired phone at any time.
* Added a short "exit" animation to the voice chrome UI that displays on transitions from Speaking to Idle or Listening to Idle.
* Added support for handling Alexa's Language selection mismatch between system and Alexa supported languages during first-time user experience (FTUE) and subsequent language changes.

### Resolved Issues

**Auto SDK**

* Fixed an issue in which the CBL module did not check the network connection status when attempting to refresh an access token. If there was no network connection when the refresh was attempted, the token would not refresh immediately when connection was restored. 
* Fixed an issue in which the “Alexa, stop” utterance did not stop music playback when audio ducking is enabled.
* Fixed periodic Engine shutdown crashes in `ContextManager`, `ExternalMediaPlayer`, and AACS.
* Fixed an issue in which the `Navigation module` inserted an invalid error code in the payload of the `ShowAlternativeRoutesFailed` event. Additionally added the `NOT_NAVIGATING` error code to the `Navigation AASB` interface. See the Navigation module documentation for info about which error codes to use.
* Fixed issues that could cause the Engine to hang indefinitely at shutdown.
* Fixed an issue in the Local Navigation module of the LVC extension that could cause Engine restart to fail after a previous Engine stop.
* Fixed an issue in which applications had to manually include header files from the Nlohmann - JSON for Modern C++ library (https://github.com/nlohmann/json) because the Auto SDK build did not export them
* Fixed an issue in which offline local search and navigation for POIs was not working.
* Fixed an issue in which the C++ sample app crashed during launch on the Poky Linux 32-bit platform.

**AACS Sample App**

* Fixed the language selection screen in the AACS sample app when the Preview Mode feature is enabled.  
* Fixed an issue in which the AACS sample app did not play alarms when the device is offline.
* Fixed an issue in which the display card for a second weather utterance closed too soon.
* Fixed an issue which the AACS sample app did not reset the contact permissions when switching accounts
* Fixed an issue in which the AACS sample app stopped music playback when the user tapped the screen showing a display card.
* Fixed an issue in which AACS did not play the Alexa confirmation speech when the user creates a notification while music is playing.

### Known Issues

**General**

* The [Alexa Automotive UX guidelines](#https://developer.amazon.com/en-US/docs/alexa/alexa-auto/display-cards.html#dismiss-display-cards) specify when to automatically dismiss a `TemplateRuntime` display card for each template type. The Engine publishes the `TemplateRuntime` interface messages `ClearTemplate` and `ClearPlayerInfo` based on the timeouts configured in the `aace.alexa.templateRuntimeCapabilityAgent` Engine configuration. However, the configuration does not provide enough granularity to specify timeouts for different types of display cards. Consequently, there is no way for your application to configure automatically dismissing local search templates (e.g., `LocalSearchListTemplate2`) with a different timeout than other templates (e.g., `WeatherTemplate`). The configuration also does not provide a way for you to specify infinite timeout for `NowPlaying` cards. You must implement your application’s dismissal logic for display cards and media info accordingly.
* There is a rare race condition in which publishing the `AlexaClient.StopForegroundActivity` message does not cancel the active Alexa interaction. The race condition can happen when the application publishes the message at the beginning of the `THINKING` state `AlexaClient.DialogStateChanged` transition.

**Car control**

* If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in the set from Alexa. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, Alexa retains endpoint 1 from set A, which might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try

**Communications**

* If the user asks Alexa to redial the last called number when their phone is not connected to the head unit, Alexa is silent rather than prompting the user to connect their phone.

**Entertainment**

* When music is playing, repeatedly pressing the “next” button to advance in the playlist restarts the current song.
* When using the LVC extension, if the application publishes the `MediaPlaybackRequestor.RequestMediaPlayback` AASB message before the Auto SDK Engine connects to Alexa cloud, media playback will not automatically resume as expected. The workaround is to wait for the connection to Alexa cloud to complete before publishing the `RequestMediaPlayback` message.
* There is no AASB message to indicate to Alexa that the user switched the media player UI on the head unit from an Alexa-integrated local media source, such as FM radio, to Alexa cloud-based music service provider. The only way to switch the audio context between the two player types is through voice interaction explicitly requesting a particular player.
* If your application cancels an Alexa interaction by sending the `AlexaClient.StopForegroundActivity` message to the Engine during music playback, the Engine might erroneously request your application to dismiss the `NowPlaying` media info by publishing the `TemplateRuntime.ClearPlayerInfo` message. Your application should not dismiss the media info in this scenario.

**Local Voice Control**

* In offline mode with LVC, after the user requests a list of POIs with an utterance such as “Alexa, find a nearby Starbucks”, Alexa does not recognize follow up requests such as "Alexa, select the first one" and does not display or read detailed information about the requested selection.

**C++ sample app**

* The sample app may fail to handle synchronous-style `AASB messages` within the required timeout to construct device context for Alexa. As a result, some utterances may not work as expected.

**AACS sample app**

* Sometimes the sample app will display an error page during sign-in if the user launched the app with the launcher icon. The recommended workaround is to set Alexa as the default assistant in the settings menu to guarantee AACS initializes properly before sign in.
* APL Card is prematurely closed if there is music playing in the background and APL command `SpeakItem` or `SpeakList`is executed.
* The voice interaction UI does not match the automotive UX guidelines for touching the screen during the interaction. The UX guidelines state that the interaction should continue if the user taps or scrolls, but the sample app cancels the interaction when the user taps or scrolls.
* The volume Alexa uses to read a shopping list is louder than the volume set for other Alexa responses.
* When the device has internet disconnected and the user sets the system language to a language not supported by Alexa, the sample app does not always display the language selection screen automatically.
* When the user revokes Alexa permission to use the microphone and then re-enables the permission, Alexa does not respond to utterances until the user restarts the app.
* If an alert is going off while Alexa is speaking, the timer audio cancels the Alexa speech. timer is going off while Alexa is speaking, the timer audio cancels the Alexa speech.

## v4.0.0 released on 2021-12-15

### Enhancements

* Deprecated the C++ and Java platform interfaces in favor of an asynchronous message-based API. Auto SDK client applications use the new `MessageBroker` to publish and subscribe to Alexa Auto Services Bridge (AASB) messages. The C++ sample app is refactored to use the new API to provide a reference implementation for Linux platforms. The Alexa Auto Client Service (AACS) sample app provides the reference implementation for Android platforms. See the [Auto SDK Migration Guide](./migration.md) for help migrating your application to use the new API.

* Enhanced the Auto SDK build system with the Conan package manager. The new build system introduces modular builds, better dependency management, and simpler build artifacts. The Auto SDK build system includes the Auto SDK Builder Tool script, which wraps the Conan build commands with a simple interface similar to the previous version of Auto SDK Builder. See the [Build Alexa Auto SDK documentation](../native/building.md) for details about the build system and the [migration guide](./migration.md) for help migrating your build to the new version of Builder Tool.

* Extended the features of Alexa Presentation Language (APL) support for automotive. The `APL` module provides messages to report vehicle properties such as the display theme, driving state, and ambient light conditions. The property settings affect how APL documents render on screen; for example, some APL content is automatically hidden when the vehicle starts moving, and the display contrast updates with the day or night mode setting. Auto SDK 4.0 supports APL 1.9. For more information about the Auto SDK `APL` interface, see the [APL module documentation.](../explore/features/apl/index.md)

* Added the `CustomDomain` interface, which establishes a bidirectional communication channel between your Auto SDK client application and your custom cloud skill. `CustomDomain` includes messages for exchanging directives, events, and context between the vehicle and your skill, achieving a fully customizable experience. For more information about the Auto SDK `CustomDomain` interface, See the [Custom Domain module documentation.](../explore/features/custom-domain/index.md)

* Added the `MediaPlaybackRequestor` interface, which enables Alexa to play the user’s favorite media content as soon as they start their vehicle. `MediaPlaybackRequestor` simplifies content selection for the user by removing the need for the user to use buttons or voice commands to resume the Alexa media content that was playing when they stopped the vehicle. For more information about the Auto SDK `MediaPlaybackRequestor` interface, See the [Alexa module documentation.](../explore/features/alexa/index.md)

* Extended the `AudioOutput` interface and added configuration to allow ducking Alexa media. Your application can use this feature for enhanced control of Alexa content audio focus according to your platform requirements. For more information about audio ducking, see the [Core module documentation.](../explore/features/core/index.md)

* Updated the Auto SDK to use AVS Device SDK Version 1.25.0. For information about this version of AVS Device SDK, see the [AVS Device SDK release notes.](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/release-notes.html#version-1250)

* Added LVC support for Alexa Custom Assistant specialized handoffs. You can configure the default fallback and self-introduction prompts for your custom assistant while offline. For more information, see the `Alexa Custom Assistant` extension documentation.

* Integrated the Auto SDK Conan build system enhancements to AACS and the AACS sample app. You can use a single Gradle command to build AACS and the AACS sample app without using the Auto SDK Builder Tool directly. For build instructions, see the [AACS documentation](../android/aacs/README.md).

* Added the following enhancements to the AACS sample app:

    * **Additional languages—** The AACS sample app supports the following languages: *US English* (`en-US`), *Australian English* (`en-AU`), *Canadian English* (`en-CA`), *Indian English* (`en-IN`), *British English* (`en-GB`), *German* (d`e-DE`), *Spanish* (`es-ES`), *Mexican Spanish* (`es-MX`), *US Spanish* (`es-US`), *French* (`fr-FR`), *Canadian French* (`fr-CA`), *Hindi* (`hi-IN`), *Italian* (`it-IT`), *Japanese* (`ja-JP`), and *Brazilian Portuguese* (`pr-BR`).

        The sample app language setting matches the device’s system language setting and syncs the with Alexa as long as the setting is in the supported language list. If Alexa does not support the system language, the sample app GUI defaults to en-US and presents a list of languages for the user to choose from. Once the user selects the language override, the system language does not sync with the sample app again until the user logs out or disables Alexa.

    * **Network error prompts—** You can configure the sample app to provide feedback to the user when Alexa cannot respond due internet connection issues. The feedback is a voice prompt or an error screen depending on the user action.

    * **Alexa app assets—** The sample app can show Alexa logos (assets) on the setup screen and display cards instead of showing placeholder assets.

    * **Comms UI improvements—** Updated the contacts uploading logic in the `Comms UI` AACS app component to ensure the sample app only uploads the contacts for the primary phone.

* Updated the AACS Telephony library to get the outgoing phone account using the Android standard API `getDefaultOutgoingPhoneAccount`. AACS Telephony no longer sends an account query intent when receiving the `PhoneCallController.Dial` message from the Auto SDK Engine.

* Added a new intent `com.amazon.aacstelephony.bluetooth.connectionCheckCompleted`, which AACS Telephony service broadcasts when it finishes the initial bluetooth connection check.

* Updated the `alexa-auto-lwa-auth` app component to use the `Authorization` Auto SDK interface for CBL authorization.

### Other changes

* Removed support for the Android 32-bit ARM architecture (i.e., [`armeabi-v7a`](https://developer.android.com/ndk/guides/abis#v7a)).

* Moved several source code directories within the `aac-sdk` root directory to support the enhanced build system.

    * Removed `aac-sdk/platforms/android/`. The deprecated Java platform interfaces and JNI are in their respective modules. For example, the Alexa module Java interfaces and JNI are moved from `aac-sdk/platforms/android/modules/alexa/` to `aac-sdk/modules/alexa/android/`

    * Removed `aac-sdk/extensions/aasb/` because using AASB messages with MessageBroker is the primary Auto SDK API. AASB code for each module is in the respective module directory. For example, the AASB code for the Alexa module is in `aac-sdk/modules/alexa/aasb/`. Note that the AASB message headers to include in your application are not in this directory since they are generated as part of the Auto SDK build output.

    * Moved `aac-sdk/extensions/system-audio/` to `aac-sdk/modules/system-audio/`

    * Moved `aac-sdk/extensions/bluetooth/` to `aac-sdk/modules/bluetooth/`

    * Moved `aac-sdk/extensions/loopback-detector/` to `aac-sdk/modules/loopback-detector/`

    * Moved  `aac-sdk/platforms/android/alexa-auto-client-service/` to `aac-sdk/aacs/android/`

    * Moved `aac-sdk/platforms/android/alexa-auto-client-service/app-components/` to `aac-sdk/aacs/android/app-components/`

    * Moved `aac-sdk/samples/android-aacs-sample-app/` to `aac-sdk/aacs/android/sample-app/`

    * Moved `aac-sdk/platforms/android/alexa-auto-client-service` `/commonutils/` , `/ipc/`, and `/constants/` to `aac-sdk/aacs/android/common/`

    * Moved AACS media player files to a directory `audioOutput` within `aac-sdk/platforms/android/alexa-auto-client-service/service/`

    * Moved the Media App Command and Control Android library from `aac-sdk/platforms/android/maccandroid/` to `aac-sdk/aacs/android/service/modules/maccandroid/`

* In the LVC extension, the `LocalSearchProvider` AASB messages now have topic `LocalNavigation`. For example, the existing message `LocalSearchProvider.SearchRequest` in 3.3 is `LocalNavigation.SearchRequest` in 4.0. The next major release version of Auto SDK will change the topic back to `LocalSearchProvider`.

* Deprecated the option to build AACS as an APK. Starting from Auto SDK 4.0, you can only build AACS as an AAR.

* Removed the Android sample app based on the Java platform interfaces. The AACS sample app demonstrates using Auto SDK on Android.

* Updated AASB configuration fields used for AACS. See the [migration guide](./migration.md) for details.

### Resolved issues

* Fixed an issue preventing the generic `DEFAULT` type `LocalMediaSource` from working in offline mode with LVC.

* Fixed a race condition in `SpeechRecognizer` in which enabling wake word detection immediately after calling `startCapture()` resulted in a missing call to `stopAudioInput()` when wake word detection was later disabled.

* Fixed a deadlock that could occur in an application that uses the deprecated `AuthProvider` interface and starts, stops, and restarts the Engine in quick succession.

* Fixed an issue in which Spotify playback commands were delayed on QNX.

* Fixed an issue in which the Engine added malformed `PhoneCallController` context to `PhoneCallController` events sent to Alexa.

* Fixed an issue in which AACS did not acquire audio focus prior to playing Alexa speech.

### Known issues

**General**

* If you do not specify the `deviceSettings.locales` field of the Alexa module configuration, the Engine automatically declares support for the following locale combinations: ["en-US", "es-US"], ["es-US", "en-US"], ["en-IN", "hi-IN"], ["hi-IN", "en-IN"], ["fr-CA", "en-CA"], ["en-CA", "fr-CA"].
    The Engine does not automatically declare support for default locale combinations if you assign an empty value to the `locales` field.

* The Engine does not persist the `aace.alexa.wakewordEnabled` Engine property setting across device reboots. Your application has to persist the setting and set the property again at each Engine start. AACS implements persisting this property and hence does not have this issue.

* If your Linux platform does not use AVX2 instructions, the Amazonlite wake word library initialization causes an illegal instruction error.

* When using LVC and stopping the Engine, the `AlexaClient` connection status remains `CONNECTED` because the connection to LVC is not disabled. Your application should not accept user utterances while the Engine is stopped despite the connection status showing `CONNECTED`.

* The [Alexa Automotive UX guidelines](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/display-cards.html#dismiss-display-cards) specify when to automatically dismiss a `TemplateRuntime` display card for each template type. The Engine publishes the `TemplateRuntime` interface messages `ClearTemplate` and `ClearPlayerInfo` based on the timeouts configured in the `aace.alexa.templateRuntimeCapabilityAgent` Engine configuration. However, the configuration does not provide enough granularity to specify timeouts for different types of display cards. Consequently, there is no way for your application to configure automatically dismissing local search templates (e.g., `LocalSearchListTemplate2`) with a different timeout than other templates (e.g., `WeatherTemplate`). The configuration also does not provide a way for you to specify infinite timeout for `NowPlaying` cards. You must implement your application’s dismissal logic for display cards and media info accordingly.

* When the user requests to view their list of timers on an APL-enabled application, they cannot use an utterance such as “Alexa, scroll up” to scroll through the list shown on the APL card.

* There is a rare race condition in which publishing the `AlexaClient.StopForegroundActivity` message does not cancel the active Alexa interaction. The race condition can happen when the application publishes the message at the beginning of the `THINKING` state `AlexaClient.DialogStateChanged` transition.

* On the Poky Linux 32-bit platform, the C++ sample app shuts down with an error on launch.

* In offline mode with LVC, you might not see the `AlexaClient.DialogStateChanged` `THINKING` state transition if the user invokes Alexa with hold-to-talk and your application provides the audio input data in one large chunk.

* In offline mode with LVC, Alexa gets stuck in the `THINKING` state and does not respond after changing the locale setting. The state recovers after a few minutes.

* The `CBL` module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the Engine attempts the refresh, it might take up to a minute to refresh the token after the internet connection is restored.

* Some `Core` module messages published by the Engine do not have a corresponding message for the application to report a handling failure. For example, if the user invokes Alexa by tap-to-talk, and the application cannot handle the `AudioInput.StartAudioInput` message, the Engine assumes the application handled the message properly and will provide audio data. As a result, the Engine state and application state might become out of sync. The affected messages are the following:
    * `AudioInput`:
        * `StartAudioInput`
    * `AudioOutput`:
        * `SetPosition`
        * `VolumeChanged`
        * `MutedStateChanged`

**Car control**

* If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in the set from Alexa. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, Alexa retains endpoint 1 from set A, which might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.

**Communications**

* Alexa does not understand DTMF utterances that include letters. For example, "press A" and "dial 3*#B" do not result in the correct DTMF directives.

* The user might experience unexpected results by trying to dial or place calls in the following ways:
    * Using utterances that include “double”, “triple”, “hundred”, or “thousand.” For example, calling a number such as 1-800-xxx-xxxx by saying “Alexa call one eight *double oh*...”
    * Pressing special characters such has “#” or “*” by saying "Alexa press * #."

* The user cannot accept or reject incoming Alexa-to-Alexa calls by voice while playing a skill with extended multi-turn dialogs, such as Jeopardy or Skyrim.

**Entertainment**

* If the user requests Alexa to read notifications while music is playing, they might hear the music play for a split second between the end of one notification and the start of the next.

* When an external media player authorization is in progress during Engine shutdown, a rare race condition might cause the Engine to crash.

* If your application cancels an Alexa interaction by sending the `AlexaClient.StopForegroundActivity` message to the Engine during music playback, the Engine might erroneously request your application to dismiss the` NowPlaying` media info by publishing the `TemplateRuntime.ClearPlayerInfo` message. Your application should not dismiss the media info in this scenario.

* When using the `System Audio` module, Audible and Amazon music might not play correctly on i.MX8 boards.

**Local search and navigation**

* In offline mode with LVC, after the user requests a list of POIs with an utterance such as “Alexa, find a nearby Starbucks”, Alexa does not recognize followup requests such as "Alexa, select the first one" and does not display or read detailed information about the requested selection.

**AACS**

* If you do not use the default audio output implementation (i.e., your application handles `AudioOutput` AASB messages), your application will not receive the `AudioOutput.Stop` message if Alexa media is playing when AACS shuts down. As a workaround, your application can listen to `AASB.StopService` or adopt `AACSPinger` to listen to the `STOPPED` state of AACS and stop the media accordingly.

**AACS Sample App**

* The AACS Sample App does not show the language selection screen when the app is built with Preview Mode.

* The AACS Sample App only shows the language selection screen if there is a language mismatch with the system language setting at the first app launch.


## v3.3.0 released on 2021-09-30
### Enhancements
* Added the `DeviceUsage` platform interface to provide the Alexa application network usage data to the Auto SDK Engine. The Auto SDK Engine emits this data as a metric to Amazon if Auto SDK is built with the `Device Client Metrics` extension. For more information, see the [Core module documentation](../explore/features/core/index.md)

* Extended the features of the `Local Navigation` module for the `Local Voice Control (LVC)` extension. The `LocalSearchProvider` platform interface now enables you to provide customers with offline navigation to street addresses, cities, and neighborhoods in addition to the existing support for local search and navigation to points of interest. See the Local Navigation module README for information about integrating the features.
  >**Note:** There are updates to the `LocalSearchProvider` APIs. See the [Auto SDK Migration Guide](./migration.md) for details.

* Added a new generic `DEFAULT` media source to the list of sources supported by the `LocalMediaSource` platform interface. The DEFAULT source can be used for voice playback control of any arbitrary media sources on the infotainment system outside of deep-linked MACC applications using the `ExternalMediaAdapter` interface and existing sources supported by name through the `LocalMediaSource` interface. For details about integrating a default media source, see the [Alexa module documentation](../explore/features/alexa/index.md).

* Added offline LVC support for tuning to station names on terrestrial radio and SiriusXM. E.g., “Play CNN on Sirius XM” and “Play KISS FM”. This feature is already available in online mode.

* Enhancements for AACS:

    * Added an app component called `alexa-auto-carcontrol` that deeply integrates Auto SDK car control features into the Android Automotive OS. For more information about AACS deep integration to Car Control, please refer to this [README](https://github.com/alexa/alexa-auto-sdk/blob/3.3/platforms/android/app-components/alexa-auto-carcontrol/README.md).

    * Added an enhancement in which AACS can automatically sync Alexa’s timezone and locale properties with the device system settings when you set the `syncSystemPropertyChange` field to true in your AACS configuration file. If you set the field to false or omit it, you still have flexibility to change the properties in your own implementation.

* Enhancements for AACS Sample App:

    * Added a location sharing consent screen in Alexa setup and settings wherein the user has the option to enable or disable location sharing.

    * Added support for rendering for `TemplateRuntime` display cards for the weather domain.

    * Added support for rendering `Amazon Presentation Language (APL)` documents.

    * Added media player transport control improvements. For example, shuffle and loop transport controls are added, and disabled transport controls are displayed.

    * Added support for setup and setting menu specific to the Alexa Custom Assistant extension.

### Resolved Issues
* Android 11 requires the attribute `android:foregroundServiceType` to be defined in services that require permissions such as microphone and location. This is added to the AACS Android Manifest file. Also, the `compileSdkVersion` and `targetSdkVersion` to are updated to 30 in `build.gradle`.

* Added a `UserIdentity` value in AACS `AuthStatus` when the user finishes CBL login.

* Made the 'stateOrRegion' field optional in the AACS `StartNavigation` directive JSON parser.

* Implemented the AASB `SetUserProfile` message in the CBL module to ensure the user email and username will be sent to the client application after user login when `enableUserProfile` is set to true.

* Fixed an issue that blocked a valid transition from the `THINKING` to `LISTENING` `AlexaClient` dialog states.

* Updated the `PhoneCallControllerCapabilityAgent` to include context in `PhoneCallController` events per the `PhoneCallController` API specification.

* Fixed a memory leak observed during Engine shutdown in the `Local Voice Control` extension.

* Fixed a rare deadlock issue during Engine stop and start when using the `AuthProvider` interface.

* Fixed an issue in which the Engine erroneously allowed 3,000 coordinates in the "shapes" array of navigation state queried via `Navigation::getNavigationState()`. The limit is updated to 100 coordinates.

### Known Issues
* General
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

    * The `wakewordEnabled` property is not persistent across device reboots. If you use AACS, however, this issue does not occur.

    * For Linux platforms, if your hardware does not use AVX2 instructions, the wake word library initialization causes an illegal instruction error.

    * When using LVC and calling Engine::stop(), the AlexaClient connection status remains CONNECTED because the connection to LVC is not disabled. Your implementation should not accept user utterances while the Engine is stopped despite the connection status showing CONNECTED.

    * The [automotive HMI guidelines](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/display-cards.html#dismiss-display-cards) for display cards state that actionable display cards should be dismissed automatically after 30 seconds, and non-actionable display cards should be dismissed automatically after 8 seconds. This guideline is not descriptive enough since it does not clarify what is actionable and non-actionable content. The UX team is working on correcting the guideline to specify specific template types. The current automatic dismissal time for all Template Runtime display cards is 8 seconds.

* Car Control
    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.

* Communications

    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.

    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.

    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.

    * The word, "line-in," in an utterance is sometimes misinterpreted as "line" or other words. For example, if the user says, "Switch to line-in," the misinterpretation of "line-in" might cause an incorrect response.

    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.

    * If your application displays the NowPlaying `TemplateRuntime` display card when Alexa plays media, the card might be erroneously dismissed by the Auto SDK Engine with a call to `TemplateRuntime::clearPlayerInfo()` if your application calls `AlexaClient::stopForegroundActivity()` to cancel an Alexa interaction. For example, the user might initiate an Alexa interaction during music playback and then cancel it by pressing a cancel button while Alexa is listening, thinking, or speaking. The media display card should not be dismissed in this scenario.

    * The generic `DEFAULT` `LocalMediaSource` type is not supported offline with LVC. If user gives a generic playback control request like "Alexa, play" when the Alexa application is operating in the offline mode with LVC, Alexa responds "Sorry, something went wrong". Other named players like USB work as expected in the offline mode.

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.

* Local Search and Navigation
    * When using LVC in offline mode, after requesting a list of POIs (e.g., "find Starbucks nearby"), Alexa does not recognize utterances like "select the first one" and does not display or read detailed information about the requested selection.

* AACS

    * For some platform interface APIs in the Core module, when an application fails to handle a directive, there is no way to report the failure to the Engine. This is because AASB assumes that the application always handles messages correctly. When AASB incorrectly reports how the application handles the message, the Engine state might become inconsistent with the application state. For example, suppose the Engine sends a directive to the application to set the audio volume but the application fails to make the change. AASB does not report the failure to the Engine. As a result, the Engine's and the application's settings become out of sync. The following list shows the affected APIs:
        * `AudioInput`:
            * `startAudioInput()`
        * `AudioOutput`:
            * `setPosition(int64_t position)`
            * `volumeChanged(float volume)`
            * `mutedStateChanged(MutedState state)`

    * If you are not using the default audio output implementation (i.e. your application handles `AudioOutput` AASB messages) and even though you are playing the Alexa pushed media content, `Stop` message would not be sent from AACS when AACS shuts down. e.g. If you are playing an audio stream for AmazonMusic, if AACS is stopped, AASB `AudioOutput.Stop` message would not be received. As a result, the media playing from your application would not be stopped. This issue will be fixed in the next release. As a workaround, your application can listen to `AASB.StopService` message or adopt `AACSPinger` to listen to the `STOPPED` state of AACS and stop the media accordingly.

## v3.2.1 released on 2021-08-06
>**Note:** All Auto SDK 3.2 extensions are compatible with 3.2.1.

### Enhancements

* Added additional APIs to the `Connectivity` module, which enable the voice up-sell conversation between the user and Alexa to activate a trial data plan or a paid subscription plan. Your implementation should call `AlexaConnectivity::sendConnectivityEvent()` to notify the Engine of the data plan type. To respond, the Engine calls `AlexaConnectivity::connectivityEventResponse()`.

* Added the configuration field `aace.addressBook.cleanAllAddressBooksAtStart` to Engine configuration. This field specifies whether to automatically delete address books each time the Engine starts.

### Resolved Issues
Fixed an issue in which wake words cannot be detected correctly when using the `SpeechRecognizer::startCapture()` API with an external wake word engine.

### Known Issues
* General
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

    * The `wakewordEnabled` property is not persistent across device reboots. If you use AACS, however, this issue does not occur.

    * For Linux platforms, if your hardware does not use AVX2 instructions, the wake word library initialization causes an illegal instruction error.

    * When using LVC and calling Engine::stop(), the AlexaClient connection status remains CONNECTED because the connection to LVC is not disabled. Your implementation should not accept user utterances while the Engine is stopped despite the connection status showing CONNECTED.

* Car Control
    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.

* Communications

    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.

    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.

    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.

    * The word, "line-in," in an utterance is sometimes misinterpreted as "line" or other words. For example, if the user says, "Switch to line-in," the misinterpretation of "line-in" might cause an incorrect response.

    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.

    * If your application displays the NowPlaying `TemplateRuntime` display card when Alexa plays media, the card might be erroneously dismissed by the Auto SDK Engine with a call to `TemplateRuntime::clearPlayerInfo()` if your application calls `AlexaClient::stopForegroundActivity()` to cancel an Alexa interaction. For example, the user might initiate an Alexa interaction during music playback and then cancel it by pressing a cancel button while Alexa is listening, thinking, or speaking. The media display card should not be dismissed in this scenario.

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.

* AACS

    * For some platform interface APIs in the Core module, when an application fails to handle a directive, there is no way to report the failure to the Engine. This is because AASB assumes that the application always handles messages correctly. When AASB incorrectly reports how the application handles the message, the Engine state might become inconsistent with the application state. For example, suppose the Engine sends a directive to the application to set the audio volume but the application fails to make the change. AASB does not report the failure to the Engine. As a result, the Engine's and the application's settings become out of sync. The following list shows the affected APIs:
        * `AudioInput`:
            * `startAudioInput()`
        * `AudioOutput`:
            * `setPosition(int64_t position)`
            * `volumeChanged(float volume)`
            * `mutedStateChanged(MutedState state)`

## v3.2.0 released on 2021-05-19

### Enhancements
* Added the `DeviceSetup` platform interface that handles events and directives related to device setup during or after an out-of-the-box experience (OOBE). After the user login, Alexa is informed that device setup is complete and starts the on-boarding experience, for example, by starting a short first-time conversation. For more information, see the [Alexa module documentation](../explore/features/alexa/index.md).

* Added support in the Connectivity module to provide the network identifier from the vehicle to Alexa, which enables automakers to offer full connectivity plans to customers. For connectivity status, the module supports sending the version of the terms and conditions through a field called `termsVersion`. Also, the `termsStatus` field accepts `DEFERRED`, which means Alexa can remind users to respond to the terms and conditions at a later time.

* Added the Mobile Authorization extension, which enables applications running on the vehicle's head unit to simplify the login experience. To log in to Alexa, the user uses the Alexa mobile app on a paired smartphone, instead of opening a web browser and entering a code.

* Added the Bluetooth extension, which allows the Alexa Auto SDK to connect to devices through the Bluetooth Classic or Bluetooth Low Energy (BLE) protocol.

* Added the Geolocation extension, which provides geolocation consent support. The user can grant consent to location sharing with Alexa from your application.

* Added the `locationServiceAccessChanged(LocationServiceAccess access)` API in the `LocationProvider` interface, which allows the Engine not to query the device location when the location service access is turned off on the device.

* Added the APL Render module, which enables APL rendering capabilities in an Android application.
  >**Note:** This module is for you to experiment with APL document rendering on an automotive device. Do not use the module to render APL documents in a production vehicle.

* Added support in the Address Book module for a phonetic field. The phonetic field is required for resolving the name of a contact or navigation favorite if the name uses Kanji characters in Japanese.

* Updated the Docker container for the Auto SDK builder script to use OpenSSL 1.1.1k by default. Added an environment variable for you to change the OpenSSL version, if desired. For information about the OpenSSL version, see the Builder README.

* Updated the Auto SDK to use AVS Device SDK Version 1.22.0. For information about the AVS Device SDK, see the [AVS Device SDK Release Notes](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/release-notes.html#version-1220).

* Enhancements for AACS:

    * Added AACS instrumentation, which enables you to better understand the interactions between your application and AACS. Through instrumentation, you log Alexa Auto Service Bridge (AASB) messages to a file, which you can review for debugging purposes. For information about AACS instrumentation, see the AACS documentation.

    * Added an app component called `alexa-auto-telephony`, which enables you to pre-integrate Alexa Phone Call Controller functionalities with Android Telephony.

    * Added an app component called `alexa-auto-contacts` to enable AACS Core Service to fetch contact information from the vehicle's head unit and send it to Alexa. The AACS Core Service can also use this library to remove from Alexa the uploaded contact information.

    * Added the AACS AAR, which you can include in your application.

    * The timeout for AASB synchronous messages is now configurable. For information about configuring the timeout, see the AACS documentation.

* Enhancements for AACS Sample App:

    * Added support for new features in the AACS Sample App. For example, it includes a menu for the user to select a language if the in-vehicle infotainment (IVI) language is not supported by Alexa, and it supports authorization with Preview Mode.

    * Added support for the Alexa Custom Assistant extension to the Alexa Auto Client Service (AACS) Sample App. The sample app demonstrates how an application can use AACS with this extension. With app components included with the sample app, you can develop an application that handles assistant handoff and displays custom animation for your custom assistant.

        >**Note:** In order to use the Alexa Custom Assistant extension with the AACS Sample App, you must install an extra component in the Auto SDK. Contact your Amazon Solutions Architect (SA) or Partner Manager for details.

* Enhancements for metrics uploading:

    * The Auto SDK emits only registration metrics before user login is complete. Other metrics are emitted after user login.

    * The Device Client Metrics (DCM) extension supports uploading more metrics from the vehicle than in previous versions.

    * The DCM extension supports anonymizing all Auto SDK metrics.

* Enhancements for car control:

  * Added prompt improvements. Alexa can provide a recommendation or ask for clarification after receiving an invalid or ambiguous user request. Suppose a user request targets the wrong mode, setting, or value for an appliance, such as "Alexa, set fan speed to 100", Alexa responds, "Sorry, you can only set the fan between 1 and 10". When the target in a user request is ambiguous, Alexa prompts for more information to determine the exact meaning of the request. For example, when a user says, "Turn on fan" (when the fan's default zone is not set), Alexa responds, "For the driver, the passenger, or the rear?" This feature is supported online and offline.

  * Improved asset management for car control, which enables Alexa to accept utterances only a few seconds after the user logs in. Previously, the user had to wait up to 20 seconds for Alexa to accept utterances.

* Improved the Auto SDK Voice Chrome extension to allow the height and width of the linear voice chrome to be controlled by the parent layout. Previously, the dimensions were fixed.

### Resolved Issues
* Disabled APL by default in AACS to make sure utterances like "tell me a joke" work correctly without handling APL. If your platform wants to implement APL, see the AACS [Configuration README](../android/aacs/service/README.md) to enable it.

* An SMS message can be sent to an Alexa contact correctly. A user request to send an SMS message to an Alexa contact no longer results in an Alexa-to-Alexa message.

* For car control, there is no longer a limit of two Device Serial Numbers (DSN) per account or Customer ID (CID).

* After the AmazonLite Wake Word locale model is switched from the default (en-US) to another locale model (e.g., de-DE), the newly selected locale remains in effect after the user quits and then restarts the application.

* Numeric weather IDs are passed to AVS for the `TemplateRunTime` API, making it easier for you to display weather icons that are consistent with your user interface.

* After the user disconnects the phone, if the user tries to use Alexa to make a call, Alexa responds correctly by reminding the user to connect the phone. Previously, Alexa tried to dial the number.

* After the user pauses on Spotify and presses “Play” to resume, the player starts correctly from the point where the player stops. Previously the player skipped ahead, resuming from an incorrect place.

* `AutoVoiceChromeController` and `StateChangeAnimationScheduler` of the Voice Chrome extension are thread-safe now, preventing the Alexa app from crashing in different scenarios (e.g. when changing to the previous music track).

* Fixed a race condition in `AuthorizationManager` during the Engine shutdown.

### Known Issues
* General
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

    * The `wakewordEnabled` property is not persistent across device reboots. If you use AACS, however, this issue does not occur.

    * For Linux platforms, if your hardware does not use AVX2 instructions, the wake word library initialization causes an illegal instruction error.

    * When using LVC and calling Engine::stop(), the AlexaClient connection status remains CONNECTED because the connection to LVC is not disabled. Your implementation should not accept user utterances while the Engine is stopped despite the connection status showing CONNECTED.

* Car Control
    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.

    * Car control utterances that are variations of supported utterances but do not follow the supported utterance patterns return errors. Examples include “please turn on the light in the car” instead of the supported “turn on the light“, and ”put on the defroster“ or “defrost the windshield” instead of the supported ”turn on the defroster”.

* Communications

    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.

    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.

    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.

    * The word, "line-in," in an utterance is sometimes misinterpreted as "line" or other words. For example, if the user says, "Switch to line-in," the misinterpretation of "line-in" might cause an incorrect response.

    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.

    * If your application displays the NowPlaying `TemplateRuntime` display card when Alexa plays media, the card might be erroneously dismissed by the Auto SDK Engine with a call to `TemplateRuntime::clearPlayerInfo()` if your application calls `AlexaClient::stopForegroundActivity()` to cancel an Alexa interaction. For example, the user might initiate an Alexa interaction during music playback and then cancel it by pressing a cancel button while Alexa is listening, thinking, or speaking. The media display card should not be dismissed in this scenario

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.

* AACS

    * For some platform interface APIs in the Core module, when an application fails to handle a directive, there is no way to report the failure to the Engine. This is because AASB assumes that the application always handles messages correctly. When AASB incorrectly reports how the application handles the message, the Engine state might become inconsistent with the application state. For example, suppose the Engine sends a directive to the application to set the audio volume but the application fails to make the change. AASB does not report the failure to the Engine. As a result, the Engine's and the application's settings become out of sync. The following list shows the affected APIs:
        * `AudioInput`:
            * `startAudioInput()`
        * `AudioOutput`:
            * `setPosition(int64_t position)`
            * `volumeChanged(float volume)`
            * `mutedStateChanged(MutedState state)`

## v3.1.0 released on 2020-12-15

### Enhancements
* Added the Authorization platform interface that replaces the CBL platform interface and the AuthProvider platform interface. For information about how the Alexa Auto SDK Engine handles authorization with the Authorization platform interface, see the [Core module documentation](../explore/features/core/index.md).

>**Note:** Logging out from CBL or Auth Provider authorization clears
the databases that store user data, such as alerts and settings. For example,
when the user logs out, pending alerts in the database are cleared to ensure that the next user who logs in does not receive the alerts. In addition, upon logout,
the locale setting is reset to the default value in the Engine configuration.
Therefore, if the current device locale is different from the default locale, you must set the locale before starting an authorization flow.

* Added the Text-To-Speech module that exposes the platform interface for requesting synthesis of Alexa speech on demand from a text or Speech Synthesis Markup Language (SSML) string. Added the Text-To-Speech Provider module that synthesizes the Alexa speech. The Text-to-Speech provider requires Auto SDK to be built with the Local Voice Control extension. For information about these modules, see the [Text-To-Speech module documentation](../explore/features/text-to-speech/index.md) and [Text-To-Speech Provider documentation](../explore/features/text-to-speech-provider/index.md).
  >**Note:** This feature may only be used with voice-guided turn-by-turn navigation.

* Added the Connectivity module that creates a lower data consumption mode for Alexa, allowing automakers to offer tiered functionality based on the status of their connectivity plans. By using this module, you can send the customer's connectivity status from the vehicle to Alexa, which determines whether the customer can enjoy a full or partial set of Alexa features. For information about the Connectivity module, see the [Connectivity documentation](../explore/features/connectivity/index.md).

* Added the Local Navigation module for the Local Voice Control (LVC) extension. This module enables you to provide customers with offline search and navigation to points of interest (POI) by leveraging the POI data of an onboard navigation provider. The POIs include categories, chains, and entities. The Local Voice Control (LVC) extension is required for the Local Navigation module.

>**Note:** Offline search with the Local Navigation module is only supported in the en-US locale.

* Added the Alexa Auto Client Service (AACS) sample app that demonstrates how an application uses AACS. The Auto SDK includes the app components used by the AACS sample app, which you can also use when developing an application that communicates with AACS. For information about the AACS sample app, see the [AACS sample app documentation](../android/aacs/sample-app/README.md)

* Added support for Digital Audio Broadcasting (DAB) radio. For more information about the DAB local media source, see the [Alexa module documentation](../explore/features/alexa/index.md).
* Enhancements for AACS:

  * Enhanced the file sharing protocol of AACS by using Android's FileProvider. This enhancement grants AACS permission to access files within your AACS client application, which are required by configuration fields for the Auto SDK.

  * Added support for the Android `ContentProvider` class, which is a standard Android mechanism for performing CRUD (Create, Read, Update, Delete) operations on stored data. By extending this class, you can use a content provider, instead of AACS messages, to manage Auto SDK properties and retrieve state information.

    For information about how AACS uses `FileProvider` and `ContentProvider`, see the AACS documentation.

  * Added support for a `ping` broadcast to check the AACS connection state. For more information about how to use `ping`, see the AACS documentation.

  * Added support for caching AASB message intent targets based on AASB Action. This enables you to define an intent filter with a subset of the possible actions for an AASB topic. For more information on specifying intent targets, see the AACS documentation.

  * Added support for Text-to-Speech Service, which allows Android applications to interact with Android TTS APIs to convert text to speech. For information about the Text-to-Speech Service, see the [AACS TTS app component documentation](../android/aacs/app-components/alexa-auto-tts/README.md).


### Resolved Issues
* On Android, the Engine returns the correct value (`UNDEFINED`) for requests to `LocationProvider.getLocation()` when the device does not have access to location. Previously the Engine populated the user geolocation with a default value when `Location.UNDEFINED` was returned in `LocationProvider.getLocation()`.

* In the AACS commonutils library, the JSON parser (`RenderPlayerInfo.kt`) for the `renderPlayerInfo` message of `templateRuntime` could only parse the `payload` field of the AASB `RenderPlayerInfo` message payload. Now it can parse the overall AASB payload.


* Notifications sound plays correctly. Previously, the sound did not play as expected due to improper channel configuration.

* The CBL module code request flow correctly applies the locale setting to the Login With Amazon (LWA) code request. Previously, the URL returned by LWA was always in the en-US locale.

* If you log out and log in, the client-side Do Not Disturb (DND) state is synchronized with Alexa.

### Known Issues
* General
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

    * The `wakewordEnabled` property is not persistent across device reboots. If you use AACS, however, this issue does not occur.

* Car Control
    * For car control, there is a limit of two Device Serial Numbers (DSN) per account or Customer ID (CID). Limit the number of devices for testing with a single account accordingly. If you use the Android sample app, be sure to configure a specific DSN.

    * It can take up to 20 seconds from the time of user login to the time Alexa is available to accept utterances. The cloud uses this time to ingest the car control endpoint configurations sent by Auto SDK after login.

    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.

    * Car control utterances that are variations of supported utterances but do not follow the supported utterance patterns return errors. Examples include “please turn on the light in the car” instead of the supported “turn on the light“, and ”put on the defroster“ or “defrost the windshield” instead of the supported ”turn on the defroster”.
    * The air conditioner endpoint supports only Power Controller and Mode Controller capabilities, not Range Controller for numeric settings.


* Communications
    * A user request to send an SMS to an Alexa contact results in an Alexa-to-Alexa message instead. However, ‘send message’ instead of ‘send SMS’ to a contact works.

    * When using LVC in online mode, users can redial a call when the phone connection state is OFF.

    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.

    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.

    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.

    * The word, "line-in," in an utterance is sometimes misinterpreted as "line" or other words. For example, if the user says, "Switch to line-in," the misinterpretation of "line-in" might cause an incorrect response.

    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.

* AACS

    * For some platform interface APIs in the Core module, when an application fails to handle a directive, there is no way to report the failure to the Engine. This is because AASB assumes that the application always handles messages correctly. When AASB incorrectly reports how the application handles the message, the Engine state might become inconsistent with the application state. For example, suppose the Engine sends a directive to the application to set the audio volume but the application fails to make the change. AASB does not report the failure to the Engine. As a result, the Engine's and the application's settings become out of sync. The following list shows the affected APIs:
        * `AudioInput`:
            * `startAudioInput()`
        * `AudioOutput`:
            * `setPosition(int64_t position)`
            * `volumeChanged(float volume)`
            * `mutedStateChanged(MutedState state)`
    * AACS enables APL by default, but it does not have a default implementation for APL. AACS expects the client application to handle the messages or directives from the Engine. If APL is not handled on the client side, utterances that trigger APL capabilities, such as "tell me a joke," fail. To disable APL, add the lines below to the AACS configuration file.

~~~
        "aasb.apl": {
            "APL": {
                   "enabled" : false
               }
        }
~~~

### Additional Changes
Starting with v3.1.0, the Local Voice Control (LVC) extension is no longer supported on ARM32 platforms.

## v3.0.0 released on 2020-10-09

### Enhancements
* Added Alexa Auto Client Service (AACS), which enables OEMs of Android-based devices to simplify the process of integrating the Auto SDK. For more information about AACS, see the [AACS documentation](https://github.com/alexa/alexa-auto-sdk/blob/3.0/platforms/android/alexa-auto-client-service/README.md).

* Added support for removing local media sources at runtime, such as a USB drive or a Bluetooth device. Previously, if a user removed a USB drive and then requested to play music from the USB drive, the Auto SDK would attempt to play and not return an appropriate error message. This feature is enabled with an existing field in the `LocalMediaSource` platform interface state. For information about the platform interface state, see the Alexa module documentation.

### Resolved Issues
* On QNX, when a portion of music on Spotify is skipped, either by the user saying, "Skip forward," or by the user skipping to a different song, the volume is no longer reset to the default level.
* A user barging in when music is playing no longer hears an Alexa response to the barge-in request. Previously, this issue happened if the System Audio extension was used.
* When streaming music from Alexa, the user can switch to a local media source by using one utterance, such as "Alexa, play radio." Previously, Alexa would not switch to the local media source after the first utterance. The user needed to issue the request again before Alexa could play from the local media source.

### Known Issues
* General
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

* Car Control
    * For car control, there is a limit of two Device Serial Numbers (DSN) per account or Customer ID (CID). Limit the number of devices for testing with a single account accordingly. If you use the Android sample app, be sure to configure a specific DSN.
    * It can take up to 20 seconds from the time of user login to the time Alexa is available to accept utterances. The cloud uses this time to ingest the car control endpoint configurations sent by Auto SDK after login.
    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.
    * Car control utterances that are variations of supported utterances but do not follow the supported utterance patterns return errors. Examples include “please turn on the light in the car” instead of the supported “turn on the light“, and ”put on the defroster“ or “defrost the windshield” instead of the supported ”turn on the defroster”.
    * The air conditioner endpoint supports only Power Controller and Mode Controller capabilities, not Range Controller for numeric settings.

* Communications
    * A user request to send an SMS to an Alexa contact results in an Alexa-to-Alexa message instead. However ‘send message’ instead ‘send SMS’ to a contact works.
    * When using LVC in online mode, users can redial a call when the phone connection state is OFF.
    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.
    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.
    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.
    * The word, "line-in," in an utterance is sometimes misinterpreted as "line" or other words. For example, if the user says, "Switch to line-in," the misinterpretation of "line-in" might cause an incorrect response.
    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.
    * If you log out and log in, the client-side Do Not Disturb (DND) state may not be synchronized with the Alexa cloud.

* AACS

    * For some platform interface APIs in the Core module, when an application fails to handle a directive, there is no way to report the failure to the Engine. This is because AASB assumes that the application always handles messages correctly. When AASB incorrectly reports how the application handles the message, the Engine state might become inconsistent with the application state. For example, suppose the Engine sends a directive to the application to set the audio volume but the application fails to make the change. AASB does not report the failure to the Engine. As a result, the Engine's and the application's settings become out of sync. The following list shows the affected APIs:
        * `AudioInput`:
            * `startAudioInput()`
        * `AudioOutput`:
            * `setPosition(int64_t position)`
            * `volumeChanged(float volume)`
            * `mutedStateChanged(MutedState state)`

     * In the commonutils library, the JSON parser (`RenderPlayerInfo.kt`) for the `renderPlayerInfo` message of `templateRuntime` can only parse the `payload` field of the AASB `RenderPlayerInfo` message payload. The `payload` field of `RenderPlayerInfo` is the inner payload of the nested payload structure. When using `TemplateRuntime.parseRenderInfo(String json)`, provide it with the embedded JSON as a string of the string value whose key is `payload` in the `RenderPlayerInfo` message’s payload instead of the overall AASB payload.

### Additional Changes
Starting with Auto SDK v3.0, we no longer support the Automotive Grade Linux (AGL) Alexa Voice agent in the Auto SDK. If you intend to use the AGL Alexa Voice Agent, continue using Auto SDK v2.3.0, which is the last version that provides AGL support.

## v2.3.0 released on 2020-07-31

### Enhancements
* Added a new Messaging module that provides support for Short Message Service (SMS) to allow a user to send, reply to, and read messages through Alexa.
* Added support for zones to car control for online-only devices so the customer can target endpoints by location (e.g., “set the front fan to 7”). This feature was supported only with the Local Voice Control (LVC) extension, and endpoints belonged to exactly one zone. The features for online-only and LVC devices are at parity and now include assigning an endpoint to multiple zones and setting a default zone. Endpoints in the default zone take higher priority than endpoints not in the default zone when no zone is specified in an utterance.
* Added support for “semantics” for car control to enable “open”, “close”, “raise”, and “lower” utterances to control endpoints.
* Added a method to the 'AlexaClient' platform interface to stop foreground-focused Alexa activity on the device (e.g., locally canceling ongoing TTS when the user selects a list item or presses a cancel button).
* Added support for Dynamic Language Switching. Previously, Alexa could only understand and respond in one language at a time. Now Alexa supports two languages at once and automatically detects the user's spoken language and responds in the same language as the utterance. The supported locale pairs are the following:
    * [ "en-US", "es-US" ]
    * [ "es-US", "en-US" ]
    * [ "en-IN", "hi-IN" ]
    * [ "hi-IN", "en-IN" ]
    * [ "en-CA", "fr-CA" ]
    * [ "fr-CA", "en-CA" ]

  **Note:** Dynamic Language Switching works online only. For hybrid systems using the LVC extension, offline Alexa understands and responds in the language of the primary locale.
* Updated radio tuning increments for “AM_RADIO” and “FM_RADIO” Local Media Source types to support the en_IN locale.
* Alexa Voice Agent now supports AGL Itchy Icefish v9.0.2.
* Language models for the Local Voice Control extension are now decoupled from the LVC.sh (Linux) binary.

### Resolved Issues
* Fixed an issue in which navigation road regulation and maneuver events resulted in “INVALID_REQUEST_EXCEPTION" or "INTERNAL_SERVICE_EXCEPTION" error logs.
* Fixed several failing car control utterances including those for offline AC controls and those using the words “my” or “lights.”
* Fixed an issue in External Media Player that caused the “NEXT” play control request to be issued twice for ExternalMediaAdapter (e.g., MACC) and LocalMediaSource platform interface handlers.
* Fixed an issue in which the Engine did not stop music playback after user logout.
* Fixed an issue that caused Spotify to play at an increased and unsteady rate on QNX.
* Fixed an issue with the  `--use-mbedtls` build option that caused a crash in the Android sample app at startup.
* Fixed an issue in the Engine metrics implementation in which regular expression matching with a large number of data points caused a crash.
* Fixed an issue in MACC in which players removed while the Engine was running (such as by the uninstallation of a linked MACC-compliant app) could not be rediscovered properly and used again, even if the player was restored (such as by the reinstallation of the app and user login). Previously, the rediscovery logic left insufficient time to process the player removal event before trying to discover players again, resulting in a loop. Now the rediscovery step runs at 5-minute intervals.
* Fixed an issue with the Engine's SQLite local storage database in which concurrent access to the database caused a crash.
* Fixed various memory leaks and intermittent crashes caused by race conditions at Engine shutdown.
* Fixed an issue on Android API 25 in which a large number of emitted logs could cause a crash due to a JNI local reference table overflow.
* Fixed an issue in which you experienced unexpected results if the local timezone of your device differed from the timezone configured through the Alexa companion app.
### Known Issues
* General
    * A user barging in when music is playing sometimes hears the Alexa response to the barge-in request and the music at the same time if System Audio extension is used.
    * If the "locales" field of the "deviceSettings" node of the Alexa module configuration JSON is not specified, the Engine automatically declares support for the following locale combinations:
        ["en-US", "es-US"],
        ["es-US", "en-US"],
        ["en-IN", "hi-IN"],
        ["hi-IN", "en-IN"],
        ["fr-CA", "en-CA"],
        ["en-CA", "fr-CA"].

      The Engine does not declare support for locale combinations if the "locales" field is assigned an empty value.

* Car Control
    * For car control, there is a limit of two Device Serial Numbers (DSN) per account or Customer ID (CID). Limit the number of devices for testing with a single account accordingly. If you use the Android sample app, be sure to configure a specific DSN.
    * It can take up to 20 seconds from the time of user login to the time Alexa is available to accept utterances. The cloud uses this time to ingest the car control endpoint configurations sent by Auto SDK after login.
    * If you configure the Auto SDK Engine and connect to Alexa using a set of endpoint configurations, you cannot delete any endpoint in a set in the cloud. For example, after you configure set A with endpoints 1, 2, and 3, if you change your car control configuration during development to set B with endpoints 2, 3, and 4, endpoint 1 from set A remains in the cloud and might interfere with resolving the correct endpoint ID for your utterances. However, any endpoint configurations with matching IDs override previous configurations. For example, the configuration of endpoint 2 in set B replaces endpoint 2 in set A. During development, limit configuration changes to create only supersets of previous endpoint configurations. Work with your Solutions Architect or Partner Manager to produce the correct configuration on the first try.
    * Car control utterances that are variations of supported utterances but do not follow the supported utterance patterns return errors. Examples include “please turn on the light in the car” instead of the supported “turn on the light“, and ”put on the defroster“ or “defrost the windshield” instead of the supported ”turn on the defroster”.
    * The air conditioner endpoint supports only Power Controller and Mode Controller capabilities, not Range Controller for numeric settings.

* Communications
    * A user request to send an SMS to an Alexa contact results in an Alexa-to-Alexa message instead. However ‘send message’ instead ‘send SMS’ to a contact works.
    * When using LVC in online mode, users can redial a call when the phone connection state is OFF.
    * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.
    * Calling numbers such as 1-800-xxx-xxxx by using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, when you call numbers by using utterances that include "triple," "hundred," and "thousand," or press special characters such as # or * by saying "Alexa press *#", you may experience unexpected results. We recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.
    * A user playing any skill with extended multi-turn dialogs (such as Jeopardy or Skyrim) cannot use voice to accept or reject incoming Alexa-to-Alexa calls.

* Entertainment
    * A user playing notifications while music is playing hears the music for a split second between the end of one notification and the start of the next.
    * The user must enunciate “line-in” in utterances targeting the “LINE_IN” Local Media Source type in order for Alexa to recognize the intent.
    * When an external player authorization is in progress at the exact moment of shutdown, a very rare race condition might occur, causing the Engine to crash.
    * On QNX, when a portion of music on Spotify is skipped, either by the user saying "Skip forward" or by the user skipping to a different song, the volume is reset to the default level.

* Authentication
    * The CBL module uses a backoff when refreshing the access token after expiry. If the internet is disconnected when the refresh is attempted, it could take up to a minute to refresh the token when the internet connection is restored.
    * If you log out and log in, the client-side Do Not Disturb (DND) state may not be synchronized with the Alexa cloud.

## v2.2.1 released on 2020-05-29

### Enhancements
* Added enhancements to the maccandroid module to allow `SupportedOperations` to be overridden to support custom actions.
* Enhanced the `TemplateRuntime` platform interface to support focus and audio player metadata in renderTemplate and renderPlayerInfo methods. This is a backward compatible change, see the [migration guide](migration.md#migrating-from-auto-sdk-v22-to-v221) for details.
* `SpeakerManager` is now a configurable option, enabled by default. When not enabled, user requests to change the volume or mute now have an appropriate Alexa response, e.g. "Sorry, I can't control the volume on your device".

### Resolved Issues
* Fixed issues in the maccandroid module to a) rediscover media apps after getting the app removed callback, and b) change the behavior to only report unauthorized when the user specifically asks to play a media app.
* On the QNX platform, prevent unnecessary flushing for audio output.

### Known Issues
* On the Android Sample App, media playback gets into "No Content Playing" state where all GUI playback control breaks, when pressing next after force closing an external media app.
* Playback controls in the C++ Sample App Playback Controller Menu are static text items and do not change visual state (e.g. add/remove, hilite, select) based on audio player metadata.

## v2.2.0 released on 2020-04-15

### Enhancements
* Added a Car Control module to support online-only car control use cases without the optional Local Voice Control (LVC) extension. The Car Control module provides the car control functionality introduced in Auto SDK 2.0.0 but does not require the LVC extension.
* Made various enhancements to the External Media Player (EMP) Adapter to improve EMP behavior and facilitate implementation of Alexa audio focus.
* Introduced the Property Manager, a new platform interface that allows you to set and retrieve Engine property values and be notified of property value changes.
* Added support for setting the timezone of a vehicle. The `PropertyManager` interface supports a new a `"TIMEZONE"` property setting.
* Added support for specifying a custom volume range for voice interactions in implementations that use the optional Local Voice Control (LVC) extension.
* Separated the LVC language models into independent APKs rather than providing them directly in the LVC APK as was done in previous releases. One language model APK is provided for each supported locale (currently en-US, en-CA, and fr-CA).

### Resolved Issues
* Fixed an issue where the CBL state did not change to stopped when you cancelled login with `CBL::cancel()`.
* Fixed an issue where volume adjustments were lost when pausing and resuming music.
* Fixed an External Media Player (EMP) Engine implementation that caused an unexpected sequence of Local Media Source playControl() method invocations such as play, then pause, followed by play again in quick succession.
* Fixed an issue where the Engine might hang during shutdown if it was shut down while TTS was being played or read.
* Fixed an issue where Auto SDK initialization failed at startup when applications using the optional LVC extension didn't register a NetworkInfoProvider platform interface.
* Fixed an issue where building the Auto SDK with sensitive logging enabled was not working as expected.
* Added alerts error enums (`DELETED` and `SCHEDULED_FOR_LATER`) to the `Alerts` platform interface.
* With the exception of road regulation and maneuver events, the Alexa cloud no longer returns an `INVALID_REQUEST_EXCEPTION` or `INTERNAL_SERVICE_EXCEPTION` in response to navigation events sent by the Auto SDK.
* Alexa now prompts or notifies the clients and rejects the ping packet when the user deregisters from the companion app.

### Known Issues
* General
  * If the local timezone of your device differs from the timezone that was configured through the Alexa companion app, the user may experience unexpected behavior. For example, if your device shows 12pm PST, but the device on the Alexa companion app is configured with an EST timezone, then asking "Alexa set an alarm for 1pm today," will return, "Sorry I can't set alarms in the past". Auto SDK v2.2.0 adds support for setting the timezone of the vehicle, which allows your device to synchronize with the timezone set in the Alexa companion app; however, the Auto SDK currently does not receive a `SetTimeZone` directive when the timezone is changed from the companion app.
* Navigation
  * The Alexa cloud currently returns an `INTERNAL_SERVICE_EXCEPTION` in response to any navigation road regulation or maneuver event sent by the Auto SDK (triggered by an utterance such as "which lane should I take", for example). You may see a harmless error/exception in the logs.
* Car Control
  * Certain car control utterances return errors. Problematic utterances include natural versions of certain test utterances (for example, “turn on the light“ instead of “please turn on the light in the car”); utterances that include the words “lights” or “my”; and utterances to control the defroster or defogger that use “put on” or “set on” rather than “turn on” or “switch on”.
  * Setting the air conditioner using range controller control capabilities (for example “set the air conditioner to 65” or “set the air conditioner to low”) is not currently supported.
  * In offline mode, the utterances "turn ac on”, “turn off ac”, “turn ac off”, and “turn up ac" return errors.
* Communications
  * When using LVC in online mode, users can redial a call when the phone connection state has been switched to OFF.
  * DTMF utterances that include the letters "A", "B", "C", or "D" (for example "press A" or "dial 3*#B") are ignored.
  * Calling numbers such as 1800xxxxxxx using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, calling numbers using utterances that include "triple", "hundred" and "thousand" and pressing special characters such as # or * using utterances such as "Alexa press *#" may return unexpected results. Therefore we recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.
* Entertainment
  * A user playing any skill with extended multi-turn dialogues (such as Jeopardy or Skyrim) cannot accept or reject incoming Alexa-to-Alexa calls using voice.
  * A user playing notifications while music is playing will hear the music for a split second between the end of one notification and the start of the next.
  * When online, Alexa does not recognize the utterance “Switch to line In.”
* Authentication
  * The CBL module uses a backoff when refreshing the access token after expiry. If internet is disconnected when the refresh is attempted, it could take up to a minute for the token to refresh when the internet connection is restored.
  * If you log out and log in, the client-side Do Not Disturb (DND) state may not be synchronized with the Alexa cloud.

## v2.1.0 released on 2019-12-19:

### Enhancements
* Added Navigation enhancements to support the following features:
    * *Add a waypoint* - Enables users to search and add waypoints to their current route along the way or start a new route with a given set of waypoints.
    * *Cancel a waypoint* - Enables users to cancel a waypoint with voice.
    * *Show/Navigate to previous destinations* - Enables users to view previous destinations and navigate to any of their previous destinations..
    * *Turn and Lane Guidance* - Enables users to ask Alexa for details about their next navigation instruction.
    * *Control Display* - Enables users to interact with their onscreen map applications.

    >**Note:** The Navigation enhancements are not backward-compatible with previous versions of the Auto SDK. The `startNavigation()` method supersedes the `setDestination()` method, and many new methods have been implemented. See the [Auto SDK Migration Guide](./migration.md) for details.

* Added support for Alexa Presentation Language (APL) rendering to present visual information and manage user interactions with Alexa.

    >**Note:** In order to use APL rendering with the Android Sample App, you must install an extra component in the Auto SDK. Contact your Amazon Solutions Architect (SA) or Partner Manager for details.

* Added support for the Alexa DoNotDisturb (DND) interface, which allows users to block all incoming notifications, announcements, and calls to their devices, and to set daily recurring schedules that turn DND off and on. For details, see the [DND Interface documentation](https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html).

    >**Note:** Alexa does not notify the user of the DND state.

* Added a System Audio extension to provide the default audio capturing and playback functionality for various platforms, including audio input/output on QNX platforms. The Alexa Auto SDK Builder automatically includes the System Audio extension when you build the Auto SDK.

* Added local media sources (LMS) and hybrid car control support to the Automotive Grade Linux (AGL) Alexa Voice Agent.

* Added `onAuthFailure()` to the `AuthProvider` platform interface and an `AUTHORIZATION_EXPIRED` argument to the `cblStateChanged()` method of the CBL platform interface to expose 403 unauthorized request exceptions from Alexa Voice Service (AVS). These may be invoked, for example, when your product makes a request to AVS using an access token obtained for a device which has been deregistered from the Alexa companion app.

* Added support for call display information change notifications (caller ID) to the optional Alexa Communication extension.

### Resolved Issues

* Fixed an issue where contact uploading failed for contacts without addresses.
* Fixed an issue where if the user rejected an incoming Alexa-to-Alexa call via voice, ringtones did not sound for subsequent incoming calls until the user either answered an incoming call via voice or made an outbound call.
* Fixed an issue that required you to assign unique entry IDs to contacts and navigation favorites to ensure that the ID space used for contacts and navigation favorites did not collide.
* Fixed an issue where multiple automotive devices using the same account at the same time could access contacts from phones paired across those devices.
* Fixed an issue where uttering "stop" when a timer sounded during an Alexa-to-Alexa call ended the call, not the timer.
* Added enhancements to the maccandroid module (Spotify) to simplify the `MACCPlayer` handler implementation. Rediscovery now occurs automatically, and the authorization TTS error events no longer occur repeatedly.

### Known Issues
* The Alexa cloud currently returns an `INVALID_REQUEST_EXCEPTION` or `INTERNAL_SERVICE_EXCEPTION` in response to any navigation event sent by the Auto SDK. You may see a harmless error/exception in the logs.
* The CBL module uses a backoff when refreshing the access token after expiry. If internet is disconnected when the refresh is attempted, it could take up to a minute for the token to refresh when the internet connection is restored.
* If the user deregisters from the companion app, Alexa does not prompt or notify the clients and does not reject the ping packet.
* If you log out and log in, the Do Not Disturb (DND) state is not synchronized with Alexa.
* When you cancel login with `CBL::cancel()`, the CBL state does not change to stopped.
* Calling numbers such as 1800xxxxxxx using utterances such as “Alexa call one eight double oh...” may return unexpected results. Similarly, calling numbers using utterances that include "triple", "hundred" and "thousand" and pressing special characters such as # or* using utterances such as "Alexa press *#" may return unexpected results. Therefore we recommend that your client application ignore special characters, dots, and non-numeric characters when requesting Alexa to call or press digits.
* The Engine may sometimes stop abruptly on shutdown due to a race condition. However, since shutdown is triggered when the car ignition is turned off, no direct customer impact is expected.
* The Engine may hang during shutdown if it is shut down while TTS is being played or read. Therefore, you should avoid calling the shutdown method while loading or playing SpeechSynthesizer audio.
* When online, Alexa does not recognize the utterance “Switch to line In.”
* A user playing Jeopardy or Skyrim cannot accept or reject incoming Alexa-to-Alexa calls using voice.
* If the local timezone of your device differs from the timezone that was configured through the Alexa companion app, the user may experience unexpected behavior. For example, if your device shows 12pm PST, but the device on the Alexa companion app is configured with an EST timezone, then asking "Alexa set an alarm for 1pm today," will return, "Sorry I can't set alarms in the past".
* When pausing and resuming music, volume adjustments are lost.
* A user playing notifications while music is playing will hear the music for a split second between the end of one notification and the start of the next.
* The External Media Player (EMP) Engine implementation does not wait for a dialog channel focus change to complete, such as after TTS, before executing an EMP directive, such as playing the CD player. As a result, you may see an unexpected sequence of Local Media Source playControl() method invocations such as play, then pause, followed by play again in quick succession.

## v2.0.0 released on 2019-09-10:

### Enhancements
* Added **offline enhancements** to improve offline car control support and add support for:
    * **offline car control enhancements** - to support generic controls that represent what can be controlled in a vehicle; for example: interior lighting, fans, temperature zone (driver and passenger), vent position, defroster, air conditioner, and recirculation.
        >**Note**: The car control enhancements are not backward compatible with previous versions of car control. The configuration and platform interface have changed.

    * **offline entertainment** - to support tuning to a specific frequency or SiriusXM channel, tuning to radio presets, switching between car audio sources (bluetooth, radio, satellite radio, CD player, etc.), and controlling local audio sources (pause, shuffle, loop, etc.)
    * **offline communications** - to support uploading contacts, calling a number or a contact, answering, declining, redialing, or ending a call, and dialing digits during a call
    * **offline navigation** - to support navigating to favorite locations and canceling navigation
* Added **online entertainment enhancements** to support tuning to a specific frequency or SiriusXM channel and tuning to radio presets.
* Added **online navigation enhancements** to support navigating to favorite locations and answering ETA and time to destination questions.
* Introduced the **Address Book module**, which includes a common platform interface that you can implement to either upload user data to the Alexa cloud or allow the local communications and navigation capabilities to access user data for offline use cases (supported by the optional Local Voice Control (LVC) module). The Address Book module supersedes the Contacts Uploader module, which supports only phone contacts and only online use cases.
* Introduced a **new core Audio service and API** to implement audio input and output providers, and deprecated the existing MediaPlayer and Speaker platform interfaces in the Alexa module. This redesign simplifies integration with platform-specific audio capabilities and enables implementation of new, advanced audio features.
>**NOTE:** The new core Audio service and APIs are not backward compatible with previous versions of the Alexa Auto SDK (prior to version 2.0.0).

* Added a library to support the **Device Client Metrics (DCM) extension for additional platforms** such as Linux and QNX in addition to Android, which was supported in release 1.5. This library is required to upload metrics and vehicle information to the Amazon cloud.
* Added support for **Voice Chrome for Android**, an extension available through your Solutions Architect or Partner Manager that provides a Voice Chrome library for the Android platform. This library allows you to display voice chrome animations of different Alexa states to the user on screen.
* Added an **integrated wake word enhancement to ignore Alexa waking itself up**. In order to implement this enhancement, you must provide audio loopback via the platform or application.
* Added **local pause handling to the PlaybackController** to allow non-voice interactions to pause media playback from the AudioPlayer source immediately, without waiting for a response from the cloud.
* Added **Geolocation support** to the Navigation module. Geolocation support enables location-based services (which are used to answer questions such as “where am I” or “what’s the weather”) to use the location information provided by the platform.

    >**Note:** In order to make use of this functionality, you must register the Navigation platform interface for Geolocation support.
* **Enhanced the builder scripts** to simplify the build process by removing unnecessary options and including the default components for different targets. For details see the Builder documentation.
* **Refactored the Java Native Interface (JNI) code** used for Android platform interfaces for more modular deployment. In place of a single AAR including all Auto SDK native libraries, the Alexa Auto SDK now generates multiple AARs (one per module). See the builder documentation and the [Android Sample App documentation](https://github.com/alexa/alexa-auto-sdk/blob/2.0/samples/android/README.md) for details.

### Resolved Issues
* Fixed an issue where music streaming from online music service providers continued to play when the user switched to a local media source.
* Fixed an issue where an MACC app (Spotify) could automatically play after the first utterance.
* Fixed a race condition in the Navigation module that occasionally caused Cancel Navigation to fail.
* Fixed broken links in the documentation.

### Known Issues

* Calling numbers such as 1800xxxxxxx using utterances such as "Alexa call one eight double oh..." may return unexpected results. Similarly, calling numbers using utterances that include "triple", "hundred" and "thousand" and pressing special characters such as # or &ast; using utterances such as "Alexa press &ast;#" may return unexpected results. Therefore, when requesting Alexa to call or press digits, we recommend that your client application ignore special characters, dots, and non-numeric characters if not relevant to the context.
* The Engine may crash during shutdown due to a race condition in the Local Media Source Engine implementation. However, since shutdown is triggered when the car ignition is turned off, no direct customer impact is expected.
* The Engine may hang during shutdown if it is shut down while TTS is being played or read. Therefore, you should avoid calling the shutdown method while loading or playing SpeechSynthesizer audio.
* In online mode, Alexa does not recognize the utterance "Switch to line in."
* A user interacting with multiturn skills such as Jeopardy cannot accept or reject incoming Alexa-to-Alexa calls using voice.
* If the user rejects an incoming Alexa-to-Alexa call via voice, ringtones do not sound for subsequent incoming calls until the user either answers an incoming call via the VUI or makes an outbound call.
* If you change your Car Control configuration or custom assets during development after Local Voice Control (LVC) was previously running, you should stop your application and LVC, change the configuration or custom assets, uninstall and reinstall LVC, and relaunch your application to ensure the changes are applied.
* To ensure that the ID space used for contacts and navigation favorites does not collide, you must assign unique `entryId`s to contacts and navigation data. If you use the same `entryId`, re-uploading contacts may cause navigation favorites to become unavailable to Alexa, and re-uploading navigation favorites may cause contacts to become unavailable.
* If the local timezone of your device differs from the timezone that was configured through the Alexa companion app, the user may experience unexpected behavior. For example, if your device shows 12pm PST, but the device on the Alexa companion app is configured with an EST timezone, then asking "Alexa set an alarm for 1pm today," will return, "Sorry I can't set alarms in the past."
* Alexa uses different audio channels, such as dialog (user utterance or TTS) and content (music), and shuffles between them to respond to user requests. As a result of this shuffling, content (such as music playback) that gets paused to accommodate higher priority channels may regain foreground audio focus and resume content in bursts between the outputs of higher priority channels (such as Alexa TTS or ongoing alerts). To avoid this, platforms should maintain the audio focus for a few extra milliseconds.
* The External Media Player (EMP) Engine implementation does not wait for a dialog channel focus change to complete, such as after TTS, before executing an EMP directive, such as playing the CD player. As a result, you may see an unexpected sequence of Local Media Source `playControl()` method invocations such as play, then pause, followed by play again in quick succession
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.6.3 released on 2019-12-02:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues

* Fixed a race condition that could cause follow-ons to a TTS request (for example asking for movies nearby) not to play while Alexa is speaking or playing something.

### Known Issues

All known issues from v1.6.0.

## v1.6.2 released on 2019-10-11:

### Enhancements

Added online entertainment enhancements to support tuning to a specific frequency or SiriusXM channel and tuning to radio presets.

### Resolved Issues

n/a

### Known Issues

All known issues from v1.6.0.
## v1.6.1 released on 2019-06-21:

### Enhancements

This release of Alexa Auto SDK includes updates for music certification.

### Resolved Issues

Resolved issues are limited to music certification updates:

* Added fixes from AVS Device SDK v1.12.1 for music certification.
* Fixed live radio offset for stations that use a dynamic window (`mime=audio/mp4a-latm`).
* Documentation updates.

### Known Issues

All known issues from v1.6.0.

## v1.6.0 released on 2019-05-16:

### Enhancements

* General availability for Linux target platforms, including: Linux x86-64, Linux ARM 64 (armv8a), and Linux ARM 32 (armv7a).
* Alexa Auto SDK v1.6.0 enhances the C++ Sample App by improving the reference implementation for Linux platforms. See the C++ sample app documentation for details.

### Resolved Issues

* Fixed an issue where Alexa Auto SDK Engine becomes unresponsive if it receives a `Play` directive during shutdown.
* Made changes to External Media Player events to send the service id and agent details, which are now mandated by the Alexa Music service. If you are using previous versions with Local Media Source switching or third-party app with MACC, you should upgrade to Alexa Auto SDK v1.6.0 to continue using the corresponding functionality.

### Known Issues

* If the local timezone of the device differs from the timezone that was configured through the Alexa companion app, you may experience unexpected behavior. For example, if your device shows 12pm PST, but the device on the Alexa companion app is configured with an EST timezone, then asking "Alexa set an alarm for 1pm today," will return, "Sorry I can't set alarms in the past.”
* If you play your notifications while music is playing, you will hear the music for a split second between the end of one notification and the start of the next.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.5.0 released on 2019-03-06:

### Enhancements

* Added a C++ sample application to demonstrate use cases that the Alexa Auto SDK supports. For details, see the C++ sample app documentation.
* Released the code for the AGL Alexa Voice Agent, a binding for Automotive Grade Linux powered by Alexa Auto SDK v1.5. The software is shipped as a standard AGL binding that exposes an API for speech recognition using the Alexa Voice Service. Please refer to the [AGL Alexa Voice Agent documentation](https://github.com/alexa/alexa-auto-sdk/blob/1.5/platforms/agl/alexa-voiceagent-service/README.md) for instructions to build, install, and test the binding on an R-Car M3 board.
* Added support for runtime selection of the AmazonLite wake word locale. The AmazonLite locale will automatically switch when the AVS locale is switched.
* Added support for optionally logging and uploading Alexa Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and UPL between the request and Alexa’s response. Please contact your SA or Partner Manager for details or to request this package for Android.
* Added support for an optional platform interface `EqualizerController`. The Equalizer Controller enables Alexa voice control of device audio equalizer settings by making gain adjustments to three frequency bands (“BASS”, “MIDRANGE”, and/or “TREBLE”).
* Added an optional Code-Based Linking (CBL) authorization implementation in the Engine. With the new `cbl` module, the Engine handles acquiring access tokens. A `CBL` platform implementation should be registered with the Engine in place of an `AuthProvider` implementation to use this method for authorization.
* Improved the usage and deployment of the Local Voice Control extension on Android. Please contact your SA or Partner Manager for more information.
* Updated the vehicle information configuration API to include a vehicle identifier. An `aace.vehicle.info.vehicleIdentifier` property of vehicle configuration is now available through the existing `VehicleConfiguration`.

### Resolved Issues

* Fixed an issue where barging in while many unprocessed Speak directives are queued could cause SpeechSynthesizer to become unresponsive or crash
* Added an `EXPECTING` state to the `AlexaClient DialogState` to accommodate multi-turn for hold-to-talk interactions. When more user input is required during an interaction, tap-to-talk interactions will transition directly from `EXPECTING` to `LISTENING` whereas hold-to-talk will remain in the `EXPECTING` state until listening is manually triggered.
* Fixed an issue where the Android Sample App could get stuck in a loop of INVALID_REQUEST_EXCEPTION errors being thrown repeatedly after MACCAndroidClient reported an error. Note: To fix this, the C++ `ExternalMediaAdapter::getState` method signature changed to allow the implementation to say whether the state it provides is valid. This change is not backward compatible.
* Fixed an issue where the Android Sample App created a syslog sink and logged VERBOSE in release builds. Note: As part of the fix, the default Engine logger sink id changed from *console* to *default*. Existing calls to `LoggerConfiguration::createLoggerRuleConfig` with sink id `"console"` should be changed to sink id `"default"`.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a `Play` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.4.0 released on 2018-12-17:

### Enhancements

* The Alexa Auto SDK now supports the Local Voice Control extension. The Local Voice Control extension enhances the Alexa Auto experience by providing voice-based car controls whether connected to the internet or not. In this release, the Local Voice Control extension will provision access only to the car’s climate control.

    > **Note**: This extension is available on request - Please contact your Amazon Solutions Architect (SA) or Partner Manager for more information.

### Resolved Issues

No resolved issues.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a ```Play``` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.3.1 released on 2019-06-21:

### Enhancements

This release of Alexa Auto SDK includes updates for music certification.

### Resolved Issues

Resolved issues are limited to music certification updates:

* Migrated to AVS Device SDK v1.12.1 for music certification. As part of the migration there is a new dependency on `openssl`. Developers using their own build system may need to make changes in order to accommodate this new dependency when linking AVS Device SDK.
* Fixed ExternalMediaPlayerAdapter getState() failure that triggered `INVALID_REQUEST_EXCEPTION/Bad Request` exceptions.
* Fixed live radio offset for stations that use a dynamic window (`mime=audio/mp4a-latm`).
* Updated the Android Sample App log view implementation for improved stability and performance.
* Bug fixes and documentation updates:
  - Additional test in `AuthProviderEngineImpl::doShutdown()` to avoid null pointer exception.
  - Fixed an issue with `SQLiteStorage::removeKey()` where the `DELETE FROM` statement repeated the `FROM`.
  - Fixed a race condition in `AudioChannelEngineImpl::setSource()` with back to back TTS.
  - Internal calls to `AudioChannelEngineImpl::executePlaybackFinished()` now save the player offset.
  - Internal calls to `AudioPlayerEngineImpl::removeObserver()` now remove an AudioPlayerObserverInterface observer instance instead of adding it.
  - Use `static_cast<unsigned char>` for upper/lower character conversions.

The platform interfaces have not changed, however the following C++ and Android enums are updated:
* The enum class `DialogState` inserts the `EXPECTING` enum constant.
* The enum class `ConnectionChangedReason` inserts `NONE`, `SUCCESS`, and `UNRECOVERABLE_ERROR` enum constants.

### Known Issues

All known issues from v1.3.0.

## v1.3.0 released on 2018-11-20:

### Enhancements

* Android 8 and ARM v8a platform support.
* Making calls to contacts from a locally-paired mobile phone as long as the Alexa Auto SDK has a valid auth token. For details, see the [Contact Uploader documentation](https://github.com/alexa/alexa-auto-sdk/blob/1.3/modules/contact-uploader/README.md).
* Redial, answer, terminate, and decline calls using voice. End users can also send dual-tone multi-frequency (DTMF) via voice to interact with Interactive Voice Responders (IVRs). For details, see the [Phone Control documentation](https://github.com/alexa/alexa-auto-sdk/blob/1.3/modules/phone-control/README.md).
* Switching to local media sources, generic controls and deep linking into 3rd party media applications compatible with the Amazon Media App Command and Control (MACC) specification using the External Media Player Interface 1.1. This allows customers to switch between a CD player, AM/FM player, and auxiliary input that is MACC-compliant. For details, see the [Alexa documentation](https://github.com/alexa/alexa-auto-sdk/blob/1.3/modules/alexa/README.md).
* Enhancement for 3rd party wake word engine to enable cloud based verification.
* Provides a way to override Template Runtime display card timeout values for RenderTemplate and RenderPlayerInfo by updating the `templateRuntimeCapabilityAgent` Engine configuration values.

### Resolved Issues

No resolved issues.

### Known Issues

* The Alexa Auto SDK Engine becomes unresponsive if it receives a ```Play``` directive during shutdown. However, since shutdown is triggered when car ignition is turned off, there is no direct customer impact expected.
* When a timer sounds during an Alexa-to-Alexa call, uttering "stop" ends the call, not the timer.
* Multiple automotive devices using the same account at the same time can access contacts from phones paired across those devices.

## v1.2.0 released on 2018-10-15:

### Enhancements

* Additional information related to the presentation of alerts is now available. The extended interface now includes Alert token, type, rendering time, and label if applicable when an alert is set and notification when an alert is deleted.
* In the Navigation platform interface, ```SetDestination``` now provides business hours and contact information for a returned location when available.

### Resolved Issues

* If a location is not available, the location state is set to ```unavailable```. Previously it was treated as ```(0,0)```, which was a valid value for longitude and latitude.
* Fixed an issue related to stopping an alert where there could be up to a 10 second delay before the alert completely stopped.
* Fixed issue where the ```TemplateRuntime``` platform interface could not be registered before ```AudioPlayer```.

### Known Issues

There are no known issues in this release.

## v1.1.1 released on 2018-09-10:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues


* Updated a dependency build recipe to skip the checksum verification to allow for document changes in the current tag.

### Known Issues

There are no known issues in this release.

## v1.1.0 released on 2018-08-31:

### Enhancements

* Added support for choosing one of multiple network adaptors before starting the Engine.
* Added support for the latest Amazon Wakeword Engine.
* Added custom volume control support for infotainment system's native input volume range. The range that comes down to the device will be 0 to 100.
* Added support for encoding the utterance in OPUS format with the Amazon Wakeword Engine as well as PTT. Our builder pulls the libopus source code as a part of build process.
* Added Locale API to return the list of Alexa-supported locales.
* Updated Vehicle Information API to capture the microphone details.
* Added support for routines, music alarms, timers and alarms volume management, and deleting all timers and alarms.
* Added support for TemplateRuntime Interface 1.1, which provides visual playback control for Alexa-enabled products with TemplateRuntime Interface support. This includes upgrades to PlaybackController Interface 1.1 and TemplateRuntime Interface 1.1.
    * > **Note**: The older button-press APIs (`playButtonPressed()`, `nextButtonPressed()`, etc.) have been deprecated in favor of the new generic `buttonPressed(PlaybackButtonType)`.
* Updated the builder script to confirm compliance with open source component licenses.

### Resolved Issues

There are no resolved issues in this release.

### Known Issues

There are no known issues in this release.

## v1.0.2 released on 2018-08-08:

### Enhancements

This release is only for documentation updates. There are no new features or enhancements.

### Resolved Issues

Only name change updates were made to the documentation. There are no resolved issues in this release.

### Known Issues

There are no known issues in this release.

## v1.0.1 released on 2018-07-31:

### Enhancements

This release is for bug fixes only. There are no new features or enhancements.

### Resolved Issues

* The Engine now reconnects to Alexa when the `NetworkInfoProvider` updates the network status.
* All shared memory objects are now freed when the Engine object is disposed.
* We fixed a media playback state issue in the Engine that caused an unexpected pause and resume for a media stream that was already stopped.
* We added AudioPlayer::playerActivityChanged to the Android APIs.
* Updated the `AuthError` enumeration with additional error types.
* Removed deprecated `createAuthConfig()` configuration method.
* Fixed issue in the JNI where trying to create a UTF-8 string with invalid characters caused a crash, seen when sensitive logging is enabled.
* Improved JNI thread handling.
* Enabled capability registration for phone call control.
* We fixed an issue where the Android platform build failed on the initial attempt when using clean code.

### Known Issues

There are no known issues in this release.

## v1.0.0 released on 2018-06-29:

### **Enhancements**

* Alexa Auto SDK now supports two ```Navigation``` directives.
    * **`SetDestination`**
    * **`CancelNavigation`**
* Added support for phone control APIs. The ```PhoneCallController``` platform interface supports the **`Dial`** directive with three events:
    * **`CallActivated`**
    * **`CallTerminated`**
    * **`CallFailed`**
* Support for Amazon Wake Word Engine (WWE)

### **Known Issues**

* The Engine doesn't immediately reconnect to AVS when the **`NetworkInfoProvider`** updates network status.
* Some shared memory objects are not freed when the Engine object is disposed.

Sample App issues are documented in the [Sample App README](https://github.com/alexa/alexa-auto-sdk/blob/1.0/samples/android/README.md).

## v1.0.0 Beta released on 2018-04-29:

### **Enhancements**

The following enhancements were added to the Alexa Auto SDK since the last Alpha release (binary).

 * `SetDestination()` API added to the Navigation module
 * Android Sample Application updated with a number of features such as rendering of Display Cards (Shopping List, Coffee Shops Nearby, etc), handling of the `SetDestination()` API, Notifications, LWA (Login with Amazon)

### **Known Issues**

***SDK:***

* While the SDK does build against Android API22 and above and runs successfully on Android O devices, our current testing shows a native-code linking error when actually running on API22 devices.

***Android Sample App:***

* M3U and PLS based stream URLs are not parsed before sent to the Android Mediaplayer. Affects live streams typically coming from TuneIn and IHeartRadio services
* Media playback can take a long time to start sometimes for iHeartRadio and TuneIn
* The Android Alexa Auto SDK Sample App was developed on an Android tablet with 2048 x 1536 resolution screen size. It can run on smaller devices, but some of the display cards may not display correctly
* During Playing Media in the Sample App we see the following messages (none of these will cause any issues):
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=streamFormat
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=progressReportDelayInMilliseconds
  * E/AVS:JsonUtils:findNodeFailed:reason=missingDirectChild,child=expectedPreviousToken
  * E/AAC:aace.alexa.AudioChannelEngineImpl:validateSource:reason=invalidSource
  * E/AAC:aace.alexa.AudioChannelEngineImpl:pause:reason=invalidSource,expectedState=X
* On App startup we see the following messages (none of these will cause any issues):
   * E/AVS:SQLiteAlertStorage:openFailed::File specified does not exist.:file path=/data/user/0/com.amazon.sampleapp/cache/appdata/alerts.sqlite
* Several minor documentation issues that will be addressed in the GA release
