# Auto SDK Core API Overview

The `Engine`, `MessageBroker`, and `AASB message interfaces` comprise the core Auto SDK API. An application uses these three components, alongside a custom platform-specific integration, to build a complete Alexa client implementation for the vehicle. An Alexa client system architecture built with Auto SDK may look something like the following diagram:

![Auto SDK system diagram](../../diagrams/auto-sdk-system.png)

### The Engine

The Auto SDK Engine is a system of components that provide the core implementation of all Auto SDK features. The Engine manages communication with Amazon services, such as Alexa, on behalf of your application. With respect to Alexa, your application's Alexa client stack uses the Engine as the layer that sets up the connection to Alexa, publishes device capabilities, sends Alexa events, processes Alexa directives, and more.

Your application creates and configures an instance of the Engine and uses a simple interface to manage the Engine lifecycle for the duration of the application run time. Aside from setting up the Engine, the primary responsibility of your application is to provide the platform-specific, custom integration details that make Alexa and other core SDK features work for *your* vehicle, in particular. Platform-specific integration might include building UI and interacting with external libraries, applications, or the underlying software frameworks of your operating system in order to complete the Auto SDK client stack with deep integration into your system.

The Engine implements as much of the general functionality as possible; for integration details that it can't implement, the Engine delegates responsibility to your application "handlers" via [AASB messages](#aasb-message-interfaces) published through the [MessageBroker](#the-messagebroker).

### The Message Broker

The Message Broker is the bridge in the Alexa Auto Services Bridge (AASB). The Message Broker provides a publish-subscribe API for the Engine and your application to communicate with each other by exchanging asynchronous AASB messages. In order to consume a message that the Engine publishes to your application, your application uses the `MessageBroker` class to subscribe to the message by specifying the message `topic` and `action` as well as a handler function for `MessageBroker` to invoke to deliver the message. Similarly, the Engine uses the Message Broker to subscribe to messages published by your application.

### AASB Message Interfaces

#### Overview 

A typical Auto SDK [module](./modules-overview.md) defines one or more "interfaces" that your application uses to communicate with the Engine. *Interface* refers to a logical grouping of related AASB messages that share the same `topic`. For example, messages pertaining to vehicle navigation belong to the `Navigation` interface, and each individual AASB message in the `Navigation` interface uses `topic` "Navigation". Within the `topic`, each interface has one or more `actions` to represent individual messages, so a `topic` + `action` combination identifies a single message. Some AASB messages are fire-and-forget, whereas others require a reply.

There are two directions AASB messages can travel:

* "Outgoing" AASB messages are messages that the Engine publishes to your application. There are several reasons why the Engine might publish an outgoing message, such as requesting your application to handle a platform-specific deep integration, react to a state change, display a custom UI, and more. Your application subscribes to outgoing messages and performs the necessary actions upon receipt.

* "Incoming" AASB messages are messages that your application publishes to the Engine. An incoming message might be an asynchronous response to a particular outgoing message that your application received and handled. Alternatively, an incoming message might request the Engine to perform an operation or react to a state change.

Auto SDK documentation refers to any component in your application that handles AASB messages for one particular interface as a "handler". For example, "Navigation handler" is the component in your application that handles outgoing and incoming AASB messages within the "Navigation" message topic.

#### Message Structure 

AASB messages use a standard JSON protocol. Each AASB message has the following structure:

```
{
    "header": {
        "version": {{STRING}},
        "messageType": {{STRING}},
        "id": {{STRING}},
        "messageDescription": {
            "topic": {{STRING}},
            "action": {{STRING}},
            "replyToId": {{String}}   
        }
    },
    "payload": {
        // payload as defined by the interface
    }
}
```

| Property | Type | Required | Description
|-|-|-|-|
| header | Object | Yes | Contains metadata about the message.
| header.<br>version | String | Yes | The version of the message `payload`. The version follows the "major.minor" convention.
| header.<br>messageType | String | Yes | The type of the message. <br><br>**Accepted values:**<ul><li>`Publish`: The standard message type. </li><li>`Reply`: The message type that correlates a reply to a previous message. E.g., a response to a request for data.</ul>
| header.<br>id | String | Yes | A universally unique identifier (UUID) for the message, generated to the [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122) specification.
| header.<br>messageDescription.<br>topic | String | Yes | The name of the interface.
| header.<br>messageDescription.<br>action | String | Yes | The name of the individual message, unique within the `topic`, which determines the contents of the `payload`.
| header.<br>messageDescription.<br>replyToId | String | Yes if `messageType` is `Reply`, no otherwise | The `id` of the message to which this message replies. Used to correlate a message to its response.
| payload | Object | No | The content of the message. The combination of `topic`, `action`, and `version` define the structure.

#### Example

The following example describes a sample AASB message exchange when a user in the vehicle invokes Alexa with a button press.

Your application provides an Alexa invocation button in its UI, and when the user taps the button, your application publishes a `SpeechRecognizer.StartCapture` message:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "7604450c-61c1-11ec-90d6-0242ac120003",
        "messageDescription": {
            "topic": "SpeechRecognizer",
            "action": "StartCapture"
        }
    },
    "payload": {
        "initiator": "TAP_TO_TALK"
    }
}
```

The Engine subscribes to this message at startup time, so it is ready to consume the message when published by your application. In response to the message, the Engine determines everything it needs in order to invoke Alexa as a result of this request from your application, such as an access token, an audio stream, and states of components on the head unit. If the Engine needs something from your application, it will publish AASB messages; for example, if your application isn't already providing the user speech audio, the Engine publishes an `AudioInput.StartAudioInput` message to open the audio stream:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "9d4cedf3-eccd-4851-9c77-c1cec8af76e4",
        "messageDescription": {
            "topic": "AudioInput",
            "action": "StartAudioInput"
        }
    },
    "payload": {
        "name": "SpeechRecognizer",
        "audioType": "VOICE",
        "streamId": "f10dc9b6-eab2-4143-8378-cf6477f63fbb"        
    }
}
```

Your application receives this message from the Message Broker if it subscribed to the `AudioInput` topic and `StartAudioInput` action. The Message Broker delivers the message as an argument to the function specified when subscribing. Your application then records the user's audio from the microphone and provides it to the Engine through `MessageBroker`.

Since Alexa will also need to know the state of the head unit in order to properly respond to the user, the Engine may request states from your application with additional AASB messages. For example, the Engine might publish a `Navigation.GetNavigationState` message because it needs to know details about any active navigation session in case the user is asking Alexa something about the route:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "50d7397a-6408-4754-a694-35b6c633b756",
        "messageDescription": {
            "topic": "Navigation",
            "action": "GetNavigationState"
        }
    }
}
```
Your application will receive this request if supports navigation and subscribed to the outgoing `Navigation` messages. Your application publishes a reply message with the current state:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Reply",
        "id": "539abbb5-2a4d-43cf-b867-840f2b206f07",
        "messageDescription": {
            "topic": "Navigation",
            "action": "GetNavigationState",
            "replyToId": "50d7397a-6408-4754-a694-35b6c633b756"        
        }
    },
    "payload": {
        "navigationState": "<the state details are a JSON in this field>"       
    }
}
```
Once the Engine has everything it needs, it forwards the user's request to Alexa. Alexa processes the speech, and when she detects the user has finished speaking, the Engine publishes a `SpeechRecognizer.EndOfSpeechDetected` message to your application:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "8028e40c-61c1-11ec-90d6-0242ac120003",
        "messageDescription": {
            "topic": "SpeechRecognizer",
            "action": "EndOfSpeechDetected"
        }
    }
}
```

Your application might use this as a trigger to play an end of listening audio cue. If the Engine doesn't need the audio stream any more (e.g., when hands-free listening is disabled), the Engine will tell your application to close the stream with an `AudioInput.StopAudioInput` message:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "1f3bfc25-d4cb-4b88-b7bc-a536f36e4402",
        "messageDescription": {
            "topic": "AudioInput",
            "action": "StopAudioInput"
        }
    },
    "payload": {
        "streamId": "f10dc9b6-eab2-4143-8378-cf6477f63fbb"     
    }
}
```

Alexa might send directives to the Engine depending what the user asked for. For example, if the user said "turn on the fan", the Engine publishes a message requesting your application's deep integration with the vehicle hardware to perform the action:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "b033e4b9-5420-47b4-949c-666e0c2d6c36",
        "messageDescription": {
            "topic": "CarControl",
            "action": "SetControllerValue"
        }
    },
    "payload": {
        "capabilityType": "POWER",
        "endpointId": "default.fan",
        "turnOn": true   
    }
}
```