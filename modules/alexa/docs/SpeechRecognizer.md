# SpeechRecognizer Interface

The `SpeechRecognizer` interface is one of the key required interfaces in the Alexa experience. Integrate with the `SpeechRecognizer` AASB message interface (and the additional required interfaces as described in the following sections) to allow the user to invoke Alexa.

## Provide audio data to the Engine

At Engine startup time, the `SpeechRecognizer` component in the Engine opens an audio input channel of type `VOICE` for your application to provide the user speech to Alexa. Your application subscribes to the [`AudioInput.StartAudioInput`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/AudioInput/index.html#startaudioinput) and [`AudioInput.StopAudioInput`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/AudioInput/index.html#stopaudioinput) messages as outlined in the [AudioInput](https://alexa.github.io/alexa-auto-sdk/docs/explore/features/core/AudioInput/) interface documentation. When the Engine expects to receive audio from your application, the Engine publishes a `StartAudioInput` message with `audioType` set to `VOICE`. Your application provides the voice audio input until the Engine publishes the `StopAudioInput` message for the same audio type.

The user decides when to speak to Alexa by invoking her with a tap-to-talk GUI button press, a push-to-talk physical button press, or—in vehicles supporting voice-initiated listening—an "Alexa" utterance. 

## Invoke Alexa with tap-and-release

For button press-and-release Alexa invocation, your application publishes the [`SpeechRecognizer.StartCapture` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#startcapture) with `initiator` set to `TAP_TO_TALK` to tell the Engine that the user pressed the Alexa invocation button and wants to speak to Alexa. When requested, your application provides audio to the Engine until Alexa detects the end of the user's speech. The Engine publishes a [`SpeechRecognizer.EndOfSpeechDetected` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#endofspeechdetected) to your application and requests your application to stop providing audio if no other Engine components require it.

## Invoke Alexa with press-and-hold

For button press-and-hold Alexa invocation, your application publishes the [`SpeechRecognizer.StartCapture` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#startcapture) with `initiator` set to `HOLD_TO_TALK` to tell the Engine that the user is holding down the Alexa invocation button and wants to speak to Alexa until releasing the button. When requested, the application provides audio to the Engine. When the user finishes speaking and releases the button, your application notifies the Engine by publishing the [`SpeechRecognizer.StopCapture` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#stopcapture), and the Engine requests your application to stop providing audio if no other Engine components require it.

## Invoke Alexa with voice using Amazonlite wake word engine

>**Note:** To use the Amazonlite wake word engine in your application, contact your Amazon Solutions Architect or partner manager.

When the application uses the `Amazonlite` Auto SDK module for wake word detection, your application notifies the Engine when the user has hands-free listening enabled (i.e., privacy mode is off) by publishing a [`PropertyManager.SetProperty` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/PropertyManager/index.html#setproperty) with `property` set to `aace.alexa.wakewordEnabled` and `value` set to `true`. The Engine enables Amazonlite wake word detection and requests audio input from your application. Your application provides audio to the Engine for continuous wake word detection until your application disables hands-free listening by setting the `aace.alexa.wakewordEnabled` property to `false`. After disabling Amazonlite wake word detection, the Engine requests your application to stop providing audio if there no other Engine components require it.

When Amazonlite detects the "Alexa" wake word in the continuous audio stream provided by your application, the Engine publishes the [`SpeechRecognizer.WakewordDetected` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#wakeworddetected) and starts an interaction similar to one triggered by tap-to-talk invocation. When Alexa detects the end of the user's speech, the Engine publishes the [`SpeechRecognizer.EndOfSpeechDetected` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/SpeechRecognizer/index.html#endofspeechdetected) but keeps the audio input stream open for further wake word detection.

## Reduce data usage with audio encoding

To save bandwidth when the Engine sends user speech to Alexa in `SpeechRecognizer.Recognize` events, you can configure the Engine to encode the audio with the [Opus audio encoding format](https://www.opus-codec.org/docs/html_api/group__opusencoder.html) by adding the following object to your Engine configuration:

```
{
    "aace.alexa": {
       "speechRecognizer": {
           "encoder": {
                "name": "opus"
           }
       }
    }
}
```
When you set this configuration in your application, the Engine still expects your application to provide audio in the Linear PCM format specified in the [AudioInput](https://alexa.github.io/alexa-auto-sdk/docs/explore/features/core/AudioInput/) interface documentation; the Engine internally changes the encoding to Opus prior to including the audio attachment in the `Recognize` event.

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory function</summary>

If your application generates Engine configuration programmatically instead of using a JSON file, you can use the [`aace::alexa::config::AlexaConfiguration::createSpeechRecognizerConfig`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html#aacb872514a75a1fdc3d54d17b64a2ec6) factory function to create the `EngineConfiguration` object.

```cpp
#include <AACE/Alexa/AlexaConfiguration.h>

std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurations;

auto speechRecognizerConfig = aace::alexa::config::AlexaConfiguration::createSpeechRecognizerConfig("opus");
configurations.push_back(speechRecognizerConfig);

// ... create other EngineConfiguration objects and add them to configurations...

m_engine->configure(configurations);

```

</details>
<br/>

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programatically with the Java factory method</summary>

AACS is the recommended way to integrate Auto SDK for Android, so your application should provide the `aacs.alexa` configuration in the AACS configuration file. However, if your application does not use AACS, and it generates Engine configuration programmatically instead of using a JSON file, you can use the [`com.amazon.aace.alexa.config.AlexaConfiguration.createSpeechRecognizerConfig`](https://alexa.github.io/alexa-auto-sdk/docs/android/classcom_1_1amazon_1_1aace_1_1alexa_1_1config_1_1_alexa_configuration.html#a8d70d4b326745849824994e380d7d5f9) factory method to create the `EngineConfiguration` object.

```java
import com.amazon.aace.alexa.config.AlexaConfiguration;

EngineConfiguration speechRecognizerConfig = AlexaConfiguration.createSpeechRecognizerConfig("opus");

mEngine.configure(new EngineConfiguration[]{
	// ...add other EngineConfiguration objects...
	speechRecognizerConfig
});

```
</details>
<br/>