# Alexa Module Sequence Diagrams for Alexa Auto SDK

Here are a few sequence diagrams that are representative of some of the basic flows used in the Alexa Auto SDK.

## Login/Logout Sequence Diagram<a id="loginlogout"></a>

Alexa Auto SDK requires that you use access tokens for login. Application developers need to provide access so that users can register the head unit as an Alexa-enabled product with Login With Amazon (LWA). It is the OEM developer's responsibility to manage authentication and authorization.

Alexa Auto SDK doesn't maintain or manage access tokens. Maintaining and managing access tokens is the responsibility of the authorization app being used. To logout the authorization and refresh tokens need to be cleared. Anytime the authorization and refresh tokens are cleared, users must go through the authentication and authorization process again.

![sequence_loginlogout](./assets/aac-loginout.png)

**Additional Authorization Resources:**

* [Customer Experience in Android/Fire apps](https://developer.amazon.com/docs/login-with-amazon/customer-experience-android.html#login-flows)
* [Implement Authorization for AVS Using Login With Amazon](https://developer.amazon.com/alexa-voice-service/auth)
* [Understanding Login Authentication with the AVS Sample App and the Node.js Server](https://developer.amazon.com/blogs/alexa/post/bb4a34ad-f805-43d9-bbe0-c113105dd8fd/understanding-login-authentication-with-the-avs-sample-app-and-the-node-js-server)

## Tap to Talk Sequence Diagram<a id="taptotalk"></a>

This sequence diagram illustrates the initial sequence followed to access Alexa through the automotive tap to talk flow. The driver initiates the  action by pushing the voice button located in the car.

Once the driver pushes the voice button, Alexa is brought to a listening state ready for voice input, processing, and playback.

This diagram includes the actual names and syntax for the methods and functions used in the Alexa Auto SDK.

> **Note**: Tap to Talk can be used with PCM and OPUS audio formats.

![ToT Diagram](./assets/aac-seq-ttt.png)

> **Tip**: Tap to Talk can be used to initiate speech recognition from external services such as external wake word engines, physical voice command buttons, and on screen buttons from the In-vehicle Voice Infotainment (IVI) system.

### Tap to Talk Flow Summary

1. The driver pushes the voice button in the car. This specific human action puts Alexa into a listening state. There should be a visual indication to the driver that Alexa is now ready for audio input.  

1. The car's microphone is now actively receiving voice input and preparing an audio buffer to send to the Alexa Auto SDK Engine. The Alexa Auto SDK Engine audio input function loops until a directive is returned indicating that the audio input has finished.  

1. A visual indication should be displayed on the head unit screen indicating that Alexa is in a thinking state. In this state, Alexa is processing the input buffer and creating an appropriate response type. In this example Alexa is creating an audio out file that is sent to the cars head unit to play.  

## Wake Word Enabled Sequence Diagram<a id="wakewordenabled"></a>

This sequence diagram illustrates the initial sequence followed to access Alexa with the wake word enabled. The driver initiates the action by uttering the Alexa wake word.

This diagram includes the actual names and syntax for the methods and functions used in the Alexa Auto SDK.

> **Note**: WWE can be used with PCM audio format only.

![WWE Diagram](./assets/aac-seq-wwe.png)

### Wake Word Enabled Flow Summary

1. ```startAudioInput()``` is called when wake word detection is enabled. This occurs when the Engine is started if wake word is enabled by default, or when ```enabledWakewordDetection()``` is explicitly called by the application. When this happens, the application must start sending audio samples from the microphone to the Engine.

1. Audio is processed locally by the wake word engine until the wake word is detected. Once the wake word is detected, the Engine notifies the application that the dialog state has changed to "listening" and initiates a Recognize event with Alexa.

1. While in the listening state, audio data is sent to Alexa until the end of speech is detected. Once the end of speech is detected, Alexa sends a ```StopCapture``` directive to the Engine and the dialog state is changed to "thinking." Alexa then respond with additional directives in response to the speech request.

1. The application should continue to send audio to the Engine until ```stopAudioInput()``` is received. When wake word is enabled, this happens when the Engine is stopped, or when ```disableWakewordDetection()``` is explicitly called by the application.
