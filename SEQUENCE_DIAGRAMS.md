# Alexa Automotive Core (AAC) Sequence Diagrams

Here are a few sequence diagrams that are representative of some of the basic flows used in the AAC SDK.

## Login/Logout Sequence Diagram<a id="loginlogout"></a>

Alexa Auto requires that you use access tokens for login. Application developers need to provide access so that users can register the head unit as an Alexa-enabled product with Login With Amazon (LWA). It is the OEM developer's responsibility to manage authentication and authorization.

AAC doesn't maintain or manage access tokens. Maintaining and managing access tokens is the responsibility of the authorization app being used. To logout the authorization and refresh tokens need to be cleared. Anytime the authorization and refresh tokens are cleared, users must go through the authentication and authorization process again.

![sequence_loginlogout](./assets/aac-loginout.png)

**Additional Authorization Resources:**

* [Customer Experience in Android/Fire apps](https://developer.amazon.com/docs/login-with-amazon/customer-experience-android.html#login-flows)
* [Implement Authorization for AVS Using Login With Amazon](https://developer.amazon.com/alexa-voice-service/auth)
* [Understanding Login Authentication with the AVS Sample App and the Node.js Server](https://developer.amazon.com/blogs/alexa/post/bb4a34ad-f805-43d9-bbe0-c113105dd8fd/understanding-login-authentication-with-the-avs-sample-app-and-the-node-js-server)

## Tap to Talk Sequence Diagram<a id="taptotalk"></a>

This sequence diagram illustrates the initial sequence followed to access Alexa through the automotive tap to talk flow. The driver initiates the  action by pushing the voice button located in the car.

Once the driver pushes the voice button, Alexa is brought to a listening state ready for voice input, processing, and playback.

This diagram includes the actual names and syntax for the methods and functions used in the Alexa Automotive Core (AAC) SDK.

> **Note**: Tap to Talk can be used with PCM and OPUS audio formats.

![ToT Diagram](./assets/aac-seq-ttt.png)

### Tap to Talk Flow Summary

![numberone](./assets/number-1.png) &ensp; The driver pushes the voice button in the car. This specific human action puts Alexa into a listening state. There should be a visual indication to the driver that Alexa is now ready for audio input.  
![numbertwo](./assets/number-2.png) &ensp; The cars microphone is now actively receiving voice input and preparing an audio buffer to send to the AAC Engine. The AAC Engine audio input function loops until a directive is returned indicating that the audio input has finished.  
![numberthree](./assets/number-3.png) &ensp; A visual indication should be displayed on the head unit screen indicating that Alexa is in a thinking state. In this state, Alexa is processing the input buffer and creating an appropriate response type. In this example Alexa is creating an audio out file that is sent to the cars head unit to play.  

## Wake Word Enabled Sequence Diagram<a id="wakewordenabled"></a>

This sequence diagram illustrates the initial sequence followed to access Alexa with the wake word enabled. The driver initiates the action by uttering the Alexa wake word.

This diagram includes the actual names and syntax for the methods and functions used in the Alexa Automotive Core (AAC) SDK.

> **Note**: WWE can be used with PCM audio format only.

![WWE Diagram](./assets/aac-seq-wwe.png)

### Wake Word Enabled Flow Summary

![numberone](./assets/number-1.png) &ensp; OEM developers can configure the head unit to be "wake word enabled." When the Alexa wake word is enabled the microphone is alway opened and a listen loop continuously runs in the background. As soon as the wake work is detected, AAC sends an event notification to the Alexa Cloud and the state is changed to listening. A listening voice chrome should be displayed.  
![numbertwo](./assets/number-2.png) &ensp; Alexa starts building an audio data buffer until the audio request is complete.  
![numberthree](./assets/number-3.png) &ensp; Once the audio request is complete, a thinking state voice chrome should be displayed.  
![numberfour](./assets/number-4.png) &ensp; After the wake word detection is disabled, the head unit goes back to a passive listening state.  
