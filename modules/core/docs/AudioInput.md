# AudioInput Interface <!-- omit in toc -->

## Overview

The core audio Engine service provides a mechanism for Engine components of any module to open audio input channels in your application. Each Engine component that requests an audio channel specifies its audio channel type so your application can provide a microphone implementation specific to the channel type. The [`AudioInput`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/AudioInput/index.html) interface provides AASB messages for your application to share audio data with the Engine when the Engine needs it.

## Understand AudioInput

Your application subscribes to the outgoing `AudioInput` AASB messages published by the Engine. When some Engine component requests audio input (for example, when the user presses the tap-to-talk button from `SpeechRecognizer`), the Engine publishes a `StartAudioInput` message that specifies the `audioType` and `streamId`. The Engine defines the following audio types for which it requests `AudioInput` streams:

- **VOICE—** This audio input type provides user speech audio data.

- **COMMUNICATION—** This audio input type provides user speech audio data specific to Alexa-to-Alexa calling. For example, the `Alexa Comms` module Engine components request audio input with this type.

- **LOOPBACK—** This audio input type provides audio data recorded from the device's own speakers. For example, the `Loopback Detector` module Engine components request audio input with this type to detect Alexa saying her own name in the audio output.

Regardless of the audio type, your application writes audio data to the stream with the specified ID until the Engine publishes a `StopAudioInput` message for the same stream ID. The audio data you provide must use the following format:

* 16bit Linear PCM
* 16kHz sample rate
* Single channel
* Signed, little endian byte order

The core audio Engine service enables multiple Engine components to share single producer, multi-consumer audio input in two key ways:

-  Multiple Engine components might request audio input of the same type. For example, the `Alexa` module and `Amazonlite` module Engine components both want `VOICE` audio input. When the first component requests to open a `VOICE` stream, your application receives a `StartAudioInput` message requesting to open a stream for the `VOICE` type. When the second Engine component needs the voice audio type, the Engine won't ask your application for voice audio again because your application is already providing it. The Engine takes care of providing the same audio data to both consumers. Similarly, your application will only receive a `StopAudioInput` message for the voice stream when the last Engine component has canceled its request to receive this type of audio.

- Multiple Engine components might request audio input of "different" types that your application considers the same. For example, the `Alexa` module and `Alexa Comms` module want `VOICE` and `COMMUNICATION` audio input, respectively. Your application's specific integration might have one implementation for producing the user speech audio data. In this case, your application takes care of providing the same audio data to both consumers in different streams opened by the Engine.

## Use the AudioInput interface in a native C++ application

To write the audio data to the Engine after receiving a `StartAudioInput` message, use the `MessageBroker::openStream()` function, specifying the same `streamId` from the `StartAudioInput` message and the operation mode `MessageStream::Mode::WRITE`. The `openStream()` call returns a `MessageStream` object. Provide audio data in repeated calls to `MessageStream::write()` until the Engine publishes a `StopAudioInput` message for the stream ID. 

The following C++ example code demonstrates how your application subscribes to `AudioInput` AASB messages, opens an input stream to provide audio when requested, and stops providing audio when requested.

```cpp
#include <AACE/Core/MessageBroker.h>
#include <AASB/Message/Audio/AudioInput/StopAudioInputMessage.h>
#include <AASB/Message/Audio/AudioInput/StartAudioInputMessage.h>

class MyAudioInputHandler {

    // Call this during the "subscribe to AASB messages" phase of the Engine lifecycle
    void MyAudioInputHandler::subscribeToAASBMessages() {
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleStartAudioInputMessage(message); },
            StartAudioInputMessage::topic(),
            StartAudioInputMessage::action());
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleStopAudioInputMessage(message); },
            StopAudioInputMessage::topic(),
            StopAudioInputMessage::action());
    }

    void MyAudioInputHandler::handleStartAudioInputMessage(const std::string& message) {
        StartAudioInputMessage msg = json::parse(message);
        // open the stream for writing
        auto streamId = msg.payload.streamId;
        auto stream = m_messageBroker->openStream(
            streamId,
            MessageStream::Mode::WRITE);
        if (stream == nullptr) {
            // handle the error condition
            return;
        }

        startAudioInput(streamId, stream)
    }

    void startAudioInput(const std::string& streamId, std::shared_ptr<MessageStream> stream) {
        // On another thread, write data to the stream until
        // you receive a StopAudioInput message with the same "streamId"
        // ...
        // Return quickly to avoid blocking the MessageBroker's outgoing thread!
        return;
    }    

    void MyAudioInputHandler::handleStopAudioInputMessage(const std::string& message) {
        StopAudioInputMessage msg = json::parse(message);
        auto streamId = msg.payload.streamId;
        stopAudioInput(streamId);
    }

    void stopAudioInput(const std::string& streamId) {
        // Stop writing audio data to the stream identified by "streamId"
        // ...
        // Return quickly to avoid blocking the MessageBroker's outgoing thread!
        return;
    }
}
```

## Use the AudioInput interface in an Android application

Alexa Auto Client Service (AACS) provides a default implementation of `AudioInput`. You can use the default implementation in your application instead of integrating directly with the `AudioInput` AASB messages yourself. See the [Android documentation](https://alexa.github.io/alexa-auto-sdk/docs/android/) for details about using the default implementation.
