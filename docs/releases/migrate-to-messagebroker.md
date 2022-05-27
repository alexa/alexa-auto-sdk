# Migrate to the MessageBroker API

## Overview
Auto SDK 4.0 introduces a new MessageBroker API for applications to subscribe to and publish AASB messages. This API replaces the existing platform interfaces that developers use to integrate platform-specific functionality such as audio, location, and Alexa capabilities. MessageBroker also replaces the [deprecated AASB interface](https://github.com/alexa/alexa-auto-sdk/blob/v3.3.0/extensions/aasb/modules/aasb/platform/include/AACE/AASB/AASB.h) used in previous Auto SDK versions.

Developers integrating with Auto SDK for the first time should only use the MessageBroker API. Developers who upgrade to Auto SDK 4.0 (and plan to continue to upgrade beyond 4.0), should migrate their existing applications as soon as possible. The next major release of Auto SDK will remove the platform interface API without maintaining backward compatibility with older versions of the SDK.

## Application architecture
In most cases, the interface changes in Auto SDK do not require modifying the architecture of the existing Auto SDK client application. The following diagram shows an application with a typical architecture based on Auto SDK 3.3 next to the same application using the Auto SDK 4.0 MessageBroker API:

![Migration ApplicationArch](../diagrams/Migration-ApplicationArch.png)

In the example above, the key difference is that rather than creating handlers that extend platform interfaces, the new implementation uses a loosely coupled MessageBroker API to subscribe to and publish messages. It is straightforward to adapt the existing application handlers to MessageBroker by using a simple adapter pattern that does not require completely redesigning the application.

## Migrating existing platform interface handlers

The following diagram highlights the core differences between using the old platform interfaces and the new MessageBroker API. The left side shows the steps for creating the Engine and handlers and invoking interface methods in Auto SDK 3.3. The right side shows the equivalent steps using the MessageBroker API required for Auto SDK 4.0.

![Migration Steps](../diagrams/Migration-Steps.png)

Even though the MessageBroker API provides flexibility for how to design an application, it may be easier to adapt an existing implementation rather than redesigning it. 

The following example demonstrates how to modify a DoNotDisturb platform interface handler to use MessageBroker. 

**Example implementation of a DoNotDisturb handler in Auto SDK 3.3:**

In Auto SDK 3.3, the DoNotDisturb platform interface has the following methods: 

```c++
    /**
     * Handle setting of DND directive. 
     * 
     * @param [in] doNotDisturb setting state
     */
    virtual void setDoNotDisturb(const bool doNotDisturb) = 0;

    /**
     * Notifies the Engine of a platform request to set the DND State
     * 
     * @param [in] doNotDisturb setting state
     * @return true if successful, false if change was rejected
     */
    bool doNotDisturbChanged(const bool doNotDisturb);

```

The implementation overrides the `setDoNotDisturb()` platform interface method to provide application-specific behavior (in this case, logging a message to the console) and calls the Engine interface method `doNotDisturbChanged` to request a change to the DoNotDisturb setting.

```c++
#include <AACE/Alexa/DoNotDisturb.h>

class DoNotDisturbHandler : public DoNotDisturb {
    public:
        DoNotDisturbHandler() = default;
        void setDoNotDisturb(bool doNotDisturb) override;
        void notifyDoNotDisturbSettingChange(bool doNotDisturb);
};


void DoNotDisturbHandler::setDoNotDisturb(bool doNotDisturb) {
    std::cout << "setDoNotDisturb: " << doNotDisturb << std::endl;
}

void DoNotDisturbHandler::notifyDoNotDisturbSettingChange(bool doNotDisturb) {
    // Notify the Engine of a request to change the DND setting by calling
    // the Engine interface method implemented in the DoNotDisturb base class
    doNotDisturbChanged(bool doNotDisturb);
}

```

**Example implementation of a DoNotDisturb handler in Auto SDK 4.0:**

In Auto SDK 4.0 the [SetDoNotDisturbMessage](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/DoNotDisturb/index.html#setdonotdisturb) and the [DoNotDisturbChangedMessage](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/DoNotDisturb/index.html#donotdisturbchanged) replace `setDoNotDisturb` the `doNotDisturbChanged` methods, respectively. 

The following example shows the same core logic in the handler, but it uses the MessageBroker API instead of extending a platform interface.

```c++
#include <AASB/Message/Alexa/DoNotDisturb/SetDoNotDisturbMessage.h>
#include <AASB/Message/Alexa/DoNotDisturb/DoNotDisturbChangedMessage.h>
#include <AACE/Core/MessageBroker.h>

class DoNotDisturbHandler {
    public:
        DoNotDisturbHandler(std::shared_ptr<MessageBroker> messageBroker);
        void setDoNotDisturb(bool doNotDisturb);
        void doNotDisturbChanged(bool doNotDisturb);
    private:
        std::shared_ptr<MessageBroker> m_messageBroker;
};


DoNotDisturbHandler::DoNotDisturbHandler( 
    std::shared_ptr<MessageBroker> messageBroker) : 
    m_messageBroker(messageBroker) {
        
    // subscribe to the "SetDoNotDisturb" message
    m_messageBroker->subscribe(
        [=](const std::string& msg) {
            SetDoNotDisturbMessage _msg = json::parse(msg);
            setDoNotDisturb(_msg.payload.doNotDisturb);
        },    
        SetDoNotDisturbMessage::topic(),
        SetDoNotDisturbMessage::action());
}

void DoNotDisturbHandler::setDoNotDisturb(bool doNotDisturb) {
    std::cout << "setDoNotDisturb: " << doNotDisturb << std::endl;
}

void DoNotDisturbHandler::notifyDoNotDisturbSettingChange(bool doNotDisturb) {
    // Notify the Engine of a request to change the DND setting by publishing
    // a "DoNotDisturbChanged" message
    DoNotDisturbChangedMessage _msg;
    _msg.payload.doNotDisturb = doNotDisturb;
    m_messageBroker->publish(_msg.toString()); // publish is fire and forget
}

```
A reference to the MessageBroker is required. This can be accessed from the Engine object and provided when creating the DoNotDisturbHandler instance in the main application code:

```c++
auto handler = std::make_shared<DoNotDisturbHandler>(engine->getMessageBroker());
```

## Handling audio and stream based interfaces

Auto SDK 4.0 replaces audio stream platform interfaces `AudioInput` and `AudioOutput` with the MessageBroker's `MessageStream` API and corresponding AASB messages with "AudioInput" and "AudioOutput" topics. When the application receives a message that requires it to read from or write to a stream, the message payload includes a stream ID. The application uses the stream ID to “open” the stream for I/O. Developers with existing handlers for media players or microphone, for example, should migrate their handlers to use the new MessageBroker and MessageStream API. 

>**Note** In previous versions of Auto SDK, the Engine "opened" audio channels through the `AudioInputProvider` and `AudioOutputProvider` platform interfaces prior to requesting audio input or output through the `AudioInput` and `AudioOutput` platform interface instances representing each channel. In Auto SDK 4.0, there is no AASB message equivalent of `AudioInputProvider` or `AudioOutputProvider`. When the Engine needs audio input from a particular channel, it sends the `AudioInput.StartAudioInput` message with the channel type specified in the payload. Similarly, when the Engine needs to play audio for a particular channel, it sends the `AudioOutput.Prepare` message with the channel type specified in the payload.

The following example demonstrates how the application would open an input stream after receiving the `StartAudioInput` message, and write data to the stream until a `StopAudioInput` message is received:

```c++
#include <AASB/Message/Audio/AudioInput/StartAudioInputMessage.h>
#include <AASB/Message/Audio/AudioInput/StopAudioInputMessage.h>

// subscribe to the StartAudioInput message
messageBroker->subscribe([=](const std::string& msg) {
        // parse the json message
        StartAudioInputMessage _msg = json::parse(msg);
        // open the stream for writing
        auto streamId = _msg.payload.streamId;
        auto stream = messageBroker->openStream(
            streamId,
            MessageStream::Mode::WRITE);
        startAudioInput(streamId, stream)
    }),
    StartAudioInputMessage::topic(),
    StartAudioInputMessage::action());

// subscribe to the StopAudioInput message
messageBroker->subscribe([=](const std::string& msg) {
        // parse the json message
        StopAudioInputMessage _msg = json::parse(msg);
        auto streamId = _msg.payload.streamId;
        stopAudioInput(streamId);
    }),
    StopAudioInputMessage::topic(),
    StopAudioInputMessage::action());    


void startAudioInput(const std::string& streamId, std::shared_ptr<MessageStream> stream) {
    // On another thread, write data to the stream until
    // you receive a StopAudioInput message with the same streamId
    // ...
    // Return quickly to avoid blocking the MessageBroker's outgoing thread!
} 

void stopAudioInput(const std::string& streamId) {
    // Stop writing audio data to the stream
    // ...
    // Return quickly to avoid blocking the MessageBroker's outgoing thread!
}

```

A MessageStream can be read-only, write-only, or support both read and write operations. It is required to specify the operation mode when opening the stream using the `MessageStream::Mode` enumeration. If the MessageBroker cannot open a stream for the specified operation, the `openStream()` call will fail and return a null object.


## Handling synchronous-style messages

Most AASB messages are either fire-and-forget, or they have a separate message that the application or Engine sends as an asynchronous response. However, some messages exchanged between the Engine and the application require a special `reply` message type. Typically these messages retrieve data that the requester requires "synchronously", such as application states retrieved for Alexa events. The Engine may either require a reply in response to a published message, or may send a reply to the application in response to a published message.

### Replying to messages from the Engine

In most cases in which a message requires a reply, the Engine will block sending other messages until it receives the reply (or until a timeout occurs), so it is important to send the reply message right away. 

The following example demonstrates how to subscribe to the GetLocation message from the LocationProvider interface and send a reply back to the Engine:

```c++
#include <AASB/Message/Location/LocationProvider/GetLocationMessage.h>

// subscribe to GetLocation message
m_messageBroker->subscribe([=](const std::string& msg) {
        GetLocationMessageReply _reply;
        // set the reply message "replyToId" to the id of the
        // original message:
        _reply.header.messageDescription.replyToId = _msg.header.id;
        // populate the reply message payload data
        _reply.payload.location.latitude = m_latitude;
        _reply.payload.location.longitude = m_longitude;    
        // publish ther reply
        m_messageBroker->publish(_reply.toString());

    },
    GetLocationMessage::topic(),
    GetLocationMessage::action());

```

### Receiving reply messages from the Engine
For some messages published by the application, the Engine may send a `reply` back to the application. In such cases, your application must subscribe to and handle the reply from the Engine. The "replyToId" in the reply message will contain the message ID for which the reply is sent.

The following example demonstrates how subscribe to `GetPropertyReply` message from the Engine.

```c++
m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetPropertyReplyMessage(message); },
        GetPropertyMessage::topic(),
        GetPropertyMessage::action());


// Publish GetPropertyMessage 
void publishGetProperty(const std::string& name) {
    GetPropertyMessage msg;
    msg.payload.name = name;
    m_messageBroker->publish(msg.toString());
    // Engine sends the GetProperty message reply with the requested property
    // The "replyToId" in the reply message will contain the ID of this published message
}

void handleGetPropertyReplyMessage(const std::string& message) {
    GetPropertyMessageReply msg = json::parse(message);

    // Get the property value from the reply and handle in the implementation
    const std::string& propertyValue = msg.payload.value
}

```

## Migrating existing AASB platform interface handler implementation

Auto SDK 3.3 supports AASB as an optional extension and platform interface. Developers using the AASB platform interface need to migrate the AASB platform interface handler to use the new MessageBroker API instead. This can be accomplished by following a similar pattern as described in the sections above; however, use MessageBroker to subscribe to *ALL* messages in order to provide the same functionality as the existing AASB platform interface.

```c++
class AASBHandler {
public:
    AASBHandler(std::shared_ptr<MessageBroker> messageBroker);
    void messageReceived(const std::string& message);
    // engine interface implementation
    void publish(const std::string& message);
    std::shared_ptr<MessageStream> openStream(
        const std::string& streamId, MessageStream::Mode mode);
private:
    std::shared_ptr<MessageBroker> m_messageBroker;
};

AASBHandler::AASBHandler(
    std::shared_ptr<MessageBroker> messageBroker) : 
    m_messageBroker(messageBroker) {
 
    // subscribe to ALL messages and bind to the messageReceived() function
    // since it has the same method signature as the message handler
    m_messageBroker->subscribe(
        std::bind(&AASBHandler::messageReceived, this, std::placeholders::_1)
    );
}

void AASBHandler::messageReceived(const std::string& message){
    // application logic for handling AASB messages
    std::cout << message << std::endl;
}

void AASBHandler::publish(const std::string& message){
    // invoke the MessageBroker publish method
    m_messageBroker->publish(message);
}

std::shared_ptr<MessageStream> AASBHandler::openStream(
    const std::string& streamId, 
    MessageStream::Mode mode){
    // invoke the MessageBroker openStream method
    return m_messageBroker->openStream(mode);
}

```

## Hybrid or incremental migration

Although Amazon recommends migrating the entire application to MessageBroker when upgrading to Auto SDK 4.0, it is possible to use a hybrid implementation of platform interface handlers and MessageBroker until Auto SDK removes the platform interface API. New interfaces added in Auto SDK 4.0 are enabled to use the MessageBroker API by default. 

The following diagram illustrates the architecture for a hybrid application:

![Migration Hybrid](../diagrams/Migration-Hybrid.png)