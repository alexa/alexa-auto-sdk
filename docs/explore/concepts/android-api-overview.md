# Auto SDK Android API Overview

Auto SDK provides the Alexa Auto Client Service (AACS) to simplify integrating Auto SDK into an Android environment. AACS is a suite of Android libraries including a core service, called `AlexaAutoClientService`, and supporting feature and utility libraries. The core service sets up the Auto SDK Engine and related infrastructure for you so your application can interact with the AASB message API through standard Android intents. The [Auto SDK Android developer documentation](../../android/index.md) explains AACS in detail and guides you through integrating AACS into your Android system. 

At a high level, AACS takes care of the following:

- **Owning the Engine lifecycle—** AACS initializes the Engine and maps the Engine lifecycle to the lifecycle of `AlexaAutoClientService`. Instead of creating, starting, and stopping the Engine, your application starts and stops `AlexaAutoClientService` as a standard Android service.

- **Mapping AASB messages to intents—** Instead of directly using `MessageBroker` to register Java methods to handle particular AASB messages, AACS uses `MessageBroker` for you. Your application specifies intent targets for standard Android intents that represent AASB messages. When the Engine publishes an AASB message, AACS converts the message to an intent that Android delivers to your registered target broadcast receiver, activity, or service. Similarly, when you send an intent to the Engine, the AACS core service receives the intent and uses `MessageBroker` to publish the corresponding AASB message to the Engine.

- **Simplifying IPC—** AACS provides a utility library to simplify how your application parses and sends AACS intents.

- **Providing out-of-box functionality—** AACS comes with prebuilt implementations of many Auto SDK features. You can use the optional "default handlers" that run in the core service and the separate "app component" libraries to get complete or almost-complete deep integrations into the Android system. Using the default handlers and app components means fewer AASB message interfaces to integrate with, so your application gets up and running faster.

- **Simplifying Engine configuration—** AACS provides some of the basic required Engine configuration so your application doesn't have to. In general, the Engine configuration in your AACS configuration file is simpler than it would be without AACS.

## AACS intents

Because the AACS core service takes care of using the Message Broker and converting AASB messages to intents, when your application uses AACS, "subscribing to an AASB message" means "registering for an AACS intent", typically as an intent filter in your application manifest. Similarly, "publishing an AASB message" means "sending an explicit intent to AACS core service". 

An AACS intent maps to an AASB message as follows:

* The AACS intent `category` contains the AASB message `topic`. If the AASB message has `topic` "ExampleTopic", the corresponding AACS intent has `category` "com.amazon.aacs.aasb.ExampleTopic".

* The AACS intent `action` contains the AASB message `action`. If the AASB message has `action` "ExampleAction", the corresponding AACS intent has `action` "com.amazon.aacs.aasb.ExampleAction".

* For a standard AASB message, the AACS intent `extras` contains the full AASB message. The AACS intent `extras` has a `Bundle` with name "payload". The "payload" bundle contains a key "message", and the value of "message" is the full AASB message as a serialized JSON string.

    > **Note:** Some AASB messages represent larger data, such as audio streams or large JSON payloads. See the [Auto SDK Android developer documentation](../../android/index.md) for details about how AACS transmits large data.
