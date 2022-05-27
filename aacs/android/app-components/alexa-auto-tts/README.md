# Text-to-Speech Service <!-- omit in toc -->

Text-to-Speech (TTS) Service provides an implementation on the Android platform to synthesize non-Alexa speech on demand from text. It implements the Android abstract `TextToSpeechService` class, which is part of Android TTS framework APIs, and in the backend uses the Text-to-Speech functionality provided by the Auto SDK. Android applications can interact with these standard Android TTS APIs to convert text to speech.

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Building the Library](#building-the-library)
- [Architecture](#architecture)
- [Configuration](#configuration)
- [Sample Usage](#sample-usage)
  - [Initialization](#initialization)
  - [Get Capabilities](#get-capabilities)
  - [Synthesize Audio](#synthesize-audio)
- [Known Issues](#known-issues)


## Overview

Text-to-Speech Service implements [TextToSpeechService](https://developer.android.com/reference/android/speech/tts/TextToSpeechService), which is an abstract base class for TTS engine implementations. The following list describes the responsibilities of the service and the Android TTS framework:

* The service is responsible for:

    * retrieving the TTS capabilities from AACS
    * preparing the TTS audio and vending out the stream to the Android TTS framework APIs 

* The Android TTS framework is responsible for:

    * handling the states of TTS requests
    * playing out the audio
    * saving the audio to file

The Text-to-Speech Service communicates with AACS to get the available locales and fetch the synthesized audio stream. Therefore, AACS must be started, configured, and running in connected state to ensure the Text-to-Speech Service can generate the designated TTS audio correctly. 

## Building the Library
Text-to-Speech Service is built as an Android Archive (AAR) to be included in your application along with the AACS AAR.
You can build the library locally using the following steps:

  1) Enter the following command to change the directory:
      ~~~
          cd ${AAC_SDK_HOME}/aacs/android/sample-app
      ~~~
  2) Enter the following command to build the TTS library:
      ~~~
          ./gradlew :alexa-auto-tts:assembleRelease
      ~~~
      Replace `assembleRelease` with `assembleDebug` if you want to build the debug version of the library. The generated AAR is available at `alexa-auto-sdk/aacs/android/app-components/alexa-auto-tts/aacstts/build/outputs/aar`.
      You must include the `AACSIPC`, `AACSConstants`, `AACSCommonUtils`, `AACS` and `Auto SDK` AARs in your application to use with the AACS TTS AAR.

AACS TTS is by default enabled in the AACS Sample App. See the [AACS Sample App documentation](../../sample-app/README.md#building-and-signing-the-aacs-sample-app-apk) for build instructions.
>**Important!** The Text-to-Speech Service requires the Local Voice Control and Alexa Custom Assistant extensions to function.

## Architecture
The following diagram shows the high-level architecture of TTS Service and AACS. The blue box in the diagram represents the Text-to-Speech Service and the white boxes in the diagram represent the other components developed by Amazon. These components are all packaged in AARs to be added along with AACS AAR into AACS Sample App or your application. The other boxes represent components that do not belong to Amazon: The yellow boxes represent the components from the Android TTS framework, and the green box represents the OEM application using Android TTS APIs.

![Android TTS](./docs/diagrams/Android_TTS.png)

1. **Android Application** is owned by OEMs to handle the Text-to-Speech interaction with the end user by creating an instance of the `TextToSpeech` object and issuing Text-to-Speech requests.
2. **TextToSpeech** is the Facade class that acts as a bridge between the Text-to-Speech application, which issues the TTS requests, and the underlying `TextToSpeechService`, which renders the response. See [Android documentation for TextToSpeech](https://developer.android.com/reference/android/speech/tts/TextToSpeech) here.
3. **TextToSpeechService** is an abstract base class for TTS engine implementations. For more information about this class, see [Android documentation for TextToSpeechService](https://developer.android.com/reference/android/speech/tts/TextToSpeechService).
4. **AmazonTextToSpeechService** is the actual implementation of `TextToSpeechService` that communicates with AACS via the IPC library to issue Text-to-Speech requests.
5. **Core Service** is responsible for accepting TTS requests from `AmazonTextToSpeechService` and for routing those requests to the Auto SDK's `TextToSpeech` platform interface, which issues a request to the appropriate TTS provider. For information about TTS providers, see Auto SDK `Text-To-Speech-Provider` module documentation.


## Configuration
To specify the intent targets for AASB message intents from AACS, follow the instructions in [AACS documentation](../../README.md). The TTS Service defines a list of intent filters in its Android manifest to subscribe to specific AASB message intents. If your application uses static configuration to specify the target for AASB topics, provide the following information as part of the AACS configuration to enable the communication between TTS Service and AACS.
​
```java
{ ...
  "aacs.general": {
    "intentTargets": {
      "AASB": {
        "type": [
          "RECEIVER"
        ],
        "package": [
          "com.amazon.alexaautoclientservice"
        ],
        "class": [
          "com.amazon.aacstts.TTSIntentReceiver"
        ]
      },
    "AlexaClient": {
        "type": [
          "RECEIVER"
        ],
        "package": [
          "com.amazon.alexaautoclientservice"
        ],
        "class": [
          "com.amazon.aacstts.TTSIntentReceiver"
        ]
      },
    "TextToSpeech": {
        "type": [
          "RECEIVER"
        ],
        "package": [
          "com.amazon.alexaautoclientservice"
        ],
        "class": [
          "com.amazon.aacstts.TTSIntentReceiver"
        ]
      },
      ...
    }
```
TTS Service subscribes to the AASB messages with the `AASB`, `AlexaClient`, and `TextToSpeech` topics by  using a broadcast receiver. Make sure that you properly populate the type, the package name, and the class name into the `intentTargets` JSON node for these topics, as shown in the JSON example above. 

## Sample Usage
Your application uses the Text-to-Speech Service in the same way as it would use any TTS engine. To synthesize speech, the application must create a `TextToSpeech` object and then set the input text. The application can also specify the speech pitch and rate.

### Initialization
Initialize the Text-to-Speech Service in one of two ways:

* If the Android application creates a `TextToSpeech` object without specifying the package name of the TTS engine to use, the default TTS engine is used.
```java
// Create the TextToSpeech using the default engine
TextToSpeech textToSpeech = new TextToSpeech(getApplicationContext(), new TextToSpeech.OnInitListener() {
    @Override
    public void onInit(int status) {
        if (status != TextToSpeech.ERROR) {
            // do things upon TextToSpeechService is initialized
        }
    }
});
```
You can change the default engine to Text-to-Speech Service by following these steps: 

    1. Open the Android Settings menu and navigate "Preferred Engine" as follows: 
 
       app → “General Management” → “Language and Input” → "Text-to-Speech" → "Preferred Engine"

    2. Choose "Amazon Text-to-Speech Engine" in the list.

* The Android application can also directly create Text-to-Speech Service by specifying the package name "com.amazon.alexaautoclientservice" when creating the `TextToSpeech` object.
```java
// Create the TextToSpeech by specifying the package name of the TextToSpeechService
TextToSpeech textToSpeech = new TextToSpeech(getApplicationContext(), new TextToSpeech.OnInitListener() {
    @Override
    public void onInit(int status) {
        if (status != TextToSpeech.ERROR) {
            // do things upon TextToSpeechService is initialized
        }
    }
}, "com.amazon.alexaautoclientservice");
```
You must initiate TTS Service at least once to warm up the language cache before making any synthesis requests.

### Get Capabilities
The Android applications may query the Text-to-Speech Service to get the available voices. For information about voices, see [Android Voice class](https://developer.android.com/reference/android/speech/tts/Voice). For information about languages, see [Locale class](https://developer.android.com/reference/java/util/Locale).
The following examples show how to get the available voices and languages from the Engine:
```java
// Query the engine about the set of available voices
Set<Voice> voices = textToSpeech.getVoices();
// Query the engine about the set of available languages.
Set<Locale> locales = textToSpeech.getAvailableLanguages();
// Check if the specified language as represented by the Locale is available and supported.
int supported = textToSpeech.isLanguageAvailable(Locale.US)
```

### Synthesize Audio
The Android TTS framework provides two ways of synthesizing TTS audio: 

* **Play out the audio immediately:** The Android applications may call the `speak` API.
```java
public int speak (CharSequence text, 
                int queueMode, 
                Bundle params, 
                String utteranceId)
```
* **Save to a WAV file:** The Android applications may call the `synthesizeToFile` API.
```java
public int synthesizeToFile (CharSequence text, 
                Bundle params, 
                File file, 
                String utteranceId)
```

## Known Issues
Conversion of MP3 to RAW Audio for TTS on the X86 platform is not yet supported.

