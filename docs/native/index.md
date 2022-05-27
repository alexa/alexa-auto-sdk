# Auto SDK Native C++ Developer Guide

## Overview

This guide outlines how to set up, build, and integrate Auto SDK into your native C++ application. Use this guide if you develop for a head unit running a Linux or QNX operating system.

## Prerequisites

1. Follow the steps in the general [Get Started with Auto SDK](../get-started.md) guide to set up an Amazon developer account, access to the Auto SDK source code, and understand the core API and features.

2. Read the [Build Alexa Auto SDK](./building.md) guide to learn about how the Auto SDK build system works and understand how to build the SDK for your host and target hardware combination.

## (Optional) Try the sample app

Auto SDK provides a [C++ sample app](https://github.com/alexa/alexa-auto-sdk/tree/master/samples/cpp) that you can run on a Linux, QNX, or macOS machine to try sample utterances that exercise the Auto SDK APIs. See the [C++ sample app documentation](./sample-app/README.md) for more information about building and using the sample app.

## Build Auto SDK libraries

Follow the instructions in [Build Alexa Auto SDK](./building.md) to build the Auto SDK for your target platform. The output of your build command will be an archive in the `${AUTO_SDK_HOME}/builder/deploy/` directory. Extract the archive contents to find the Auto SDK libraries and headers. For example,
```
aac-dev-macos_x86_64-release-220111103523/
  ├─ docs/
  ├─ include/
  │   ├── AACE
  │   └── AASB
  ├─ lib/
  |  ├─ libAACECore.so
  |  └─ ...
  ├─ share/
  └─ aac-buildinfo.txt
```
Link the libraries from `lib` to your application, and include the headers from `include`.

## Manage the Engine lifecycle in your application

To use Auto SDK features, your application must instantiate and manage the lifecycle of the Engine.

### Create the Engine

During the launch sequence of your application, create an instance of the [Engine](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1core_1_1_engine.html) using the static function `Engine::create()`:

```cpp
#include <AACE/Core/Engine.h>

std::shared_ptr<aace::core::Engine> engine = aace::core::Engine::create();
```
A typical application creates the Engine once when the user turns on the vehicle ignition and uses the instance until the user turns off the vehicle ignition.

### Configure the Engine

After creating the Engine instance, configure the Engine with the required Engine configurations for every module that you use. Engine configuration uses serialized JSON strings, but you pass the configurations to the Engine in one or more [`aace::core::config::EngineConfiguration`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1core_1_1config_1_1_engine_configuration.html) wrapper objects. Auto SDK provides two options to generate `EngineConfiguration` objects:

- Specify your Engine configuration in a [JSON file](#specify-configuration-in-a-file) and construct an `EngineConfiguration` from a path to the file
- Build the configuration [programmatically](#specify-configuration-programmatically) using one of the configuration factory functions.

You can choose either option or a combination of both. I.e., you can generate a single `EngineConfiguration` object that includes all configuration data for the Engine components you use, or you can break up the configuration data into logical sections and generate multiple `EngineConfiguration` objects. For example, you might generate one `EngineConfiguration` object for each module.

To configure the Engine, call the `Engine::configure()` function, passing in the `EngineConfiguration` object(s): 

* For a single `EngineConfiguration` object:

    ~~~
    engine->configure( config );
    ~~~
  
* For multiple `EngineConfiguration` objects:
    ~~~
    engine->configure( { xConfig, yConfig, zConfig } );
    ~~~
    replacing `xConfig`, `yConfig`, `zConfig` with logical names to identify the `EngineConfiguration` objects you generated.

See the documentation for [individual module features](../explore/features/index.md) to see the format of each module's respective JSON configuration.

> **Note**: For one Engine instance, you can call the `configure()` function only once, and you must call it before you [subscribe to AASB messages](#subscribe-to-aasb-messages) or [start the Engine](#start-the-engine).

#### Specify configuration in a file

Auto SDK provides the [`ConfigurationFile`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1core_1_1config_1_1_configuration_file.html#a8ee151fa389256ca8e2df8626e76407e) class that reads the JSON configuration from a specified file path and creates an `EngineConfiguration` object from the configuration:

```cpp
#include <AACE/Core/EngineConfiguration.h>

aace::core::config::ConfigurationFile::create( "</path/to/filename.json>" )
```
 
You can include all the configuration data in a single JSON file to create a single `EngineConfiguration` object. For example,

```cpp
auto config = aace::core::config::ConfigurationFile::create( "/opt/AAC/config/config.json" );

engine->configure(config);
```

Alternatively, you can break the configuration data into multiple JSON files to create multiple `EngineConfiguration` objects. For example,

```cpp
auto coreConfig = aace::core::config::ConfigurationFile::create( "/opt/AAC/data/core-config.json" );
auto alexaConfig = aace::core::config::ConfigurationFile::create( "/opt/AAC/data/alexa-config.json" );
auto navigationConfig = aace::core::config::ConfigurationFile::create( "/opt/AAC/data/navigation-config.json" );

engine->configure({coreConfig, alexaConfig, navigationConfig});
```

#### Specify configuration programmatically

Each Auto SDK module that defines configuration provides a factory class with functions that return `EngineConfiguration` objects. The values a function puts in the configuration it creates correspond to the function parameters. For example, you can configure the `Alexa` module's `alertsCapabilityAgent` settings by using the [`AlexaConfiguration::createAlertsConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html#af7a3007198c8d8e47ce33edbf5c902a7) function:

```cpp
auto alertsConfig = aace::alexa::config::AlexaConfiguration::createAlertsConfig("</some/directory/path/for/databases/alerts.db>" );
```

### Register for AASB Messages

After you configure the Engine, get the `MessageBroker` instance from the `Engine` instance:

```cpp
std::shared_ptr<aace::core::MessageBroker> messageBroker = engine->getMessageBroker();
```

Use `MessageBroker` to subscribe to any AASB messages that your application will handle. See [Understand how to use MessageBroker](#understand-how-to-use-messagebroker) for further information. 

> **Note**: For one Engine instance, you must subscribe to messages after [configuring the Engine](#configure-the-engine) and before [starting the Engine](#start-the-engine).

### Start the Engine

After configuring the Engine and subscribing to AASB messages, start the Engine by calling the `Engine::start()` function:

```cpp
engine->start();
```
Engine start initializes the internal Engine components for each Engine service your application uses. With respect to Alexa, `start` initiates the connection to Alexa if there is an internet connection and an Alexa access token. Wait to publish AASB messages to the Engine until after `start()` returns.

Your application can start the Engine more than once in its lifetime, if needed, by stopping the Engine and starting it again. However, you cannot start the Engine again after shutting it down.

### Stop the Engine

When your application needs to halt the operations of the Engine, stop the Engine by calling the `Engine::stop()` function:

```cpp
engine->stop();
```
With respect to Alexa, stopping the Engine tears down the Alexa connection.
Typically, Engine stop is a cleanup step before Engine shutdown. However, if you stopped the Engine at runtime and need to start it again, calling `start()` resumes Engine operations. With respect to Alexa, this includes reestablishing the Alexa connection.

### Shut down the Engine

When your application is ready to exit, shut down the Engine by calling the Engine's `shutdown()` function.

```cpp
engine->shutdown();
```

Make sure you also stop the Engine prior to shutting it down. After shutdown completes, you can safely dispose of the pointer to your Engine instance. You cannot use this instance of the Engine again.

## Understand how to use MessageBroker

As outlined in [Auto SDK Core API Overview](../explore/concepts/core-api-overview.md), your application will use `MessageBroker` to interface with the Engine by exchanging AASB messages. The Message Broker uses AASB messages as serialized JSON strings; however, Auto SDK provides C++ wrapper classes for each message that help with the serialization and de-serialization. The Auto SDK build system generates these wrapper classes as part of the build. For example, if the build output archive file is `aac-dev-macos_x86_64-release-220111103523.tgz`, the extracted archive contains the AASB messages for each interface in a directory structure like the following example:

```
aac-dev-macos_x86_64-release-220111103523
    ├── aac-buildinfo.txt
    ├── bin
    ├── docs
    ├── include
    │   ├── AACE
    │   └── AASB
    │       └── Message
    │           ├── Alexa
    │           │   ├── ...directory for other interface in Alexa module...
    │           │   │   ├── ...message header 1 for this other interface...
    │           │   │   └── ...message header 2 for this other interface...
    │           │   ├── SpeechRecognizer
    │           │   │   ├── EndOfSpeechDetectedMessage.h
    │           │   │   ├── Initiator.h
    │           │   │   ├── StartCaptureMessage.h
    │           │   │   ├── StopCaptureMessage.h
    │           │   │   └── WakewordDetectedMessage.h
    │           │   └── ...directory for other interface in Alexa module...
    │           ├── ...directory for other module...
    │           │   ├── ...directory for other interface in this module...
    │           └── ...directory for other module...
    ├── lib
    └── share
```
The header file `StartCaptureMessage.h`, for example, represents the incoming `SpeechRecognizer.StartCapture` AASB message, and the header contains a class your application can use to build the message in the correct format and then convert it to a string to publish with `MessageBroker`. Similarly, the `WakewordDetectedMessage.h` header file contains a class your application can use to easily subscribe to the `SpeechRecognizer.WakewordDetectedMessage` message and de-serialize the message when you receive it from the Engine.

The following example code uses the AASB message wrapper classes for the `SpeechRecognizer` interface to subscribe to messages from the Engine with `SpeechRecognizer` topic:

```cpp
#include <AASB/Message/Alexa/SpeechRecognizer/EndOfSpeechDetectedMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/WakewordDetectedMessage.h>

// call this function before starting the Engine
void SpeechRecognizerHandler::subscribeToAASBMessages() {
    messageBroker->subscribe(
        [=](const std::string& message) { handleEndOfSpeechDetectedMessage(message); },
        EndOfSpeechDetectedMessage::topic(),   // equivalent to "SpeechRecognizer"
        EndOfSpeechDetectedMessage::action()); // equivalent to "EndOfSpeechDetected"

    messageBroker->subscribe(
        [=](const std::string& message) { handleWakewordDetectedMessage(message); },
        WakewordDetectedMessage::topic(),      // equivalent to "SpeechRecognizer"
        WakewordDetectedMessage::action());    // equivalent to "WakewordDetected"
}

void SpeechRecognizerHandler::handleEndOfSpeechDetectedMessage(const std::string& message) {
    // Your application defines this handling function.
    // MessageBroker invokes this function when the Engine publishes a SpeechRecognizer.EndOfSpeechDetected message.

    // Per the AASB message documentation, this message has no payload.

    // Do something here, and return quickly to avoid blocking MessageBroker's outgoing thread.
}

void SpeechRecognizerHandler::handleWakewordDetectedMessage(const std::string& message) {
    // Your application defines this handling function.
    // MessageBroker invokes this function when the Engine publishes a SpeechRecognizer.WakewordDetected message.

    // You can use the WakewordDetectedMessage class to deserialize the message.
    // The payload of this message is simple, but other messages may contain more complex payloads.
    WakewordDetectedMessage msg = json::parse(message);
    std::string ww = msg.payload.wakeword; // This string contains the wake word that the Engine detected

    // Do something here, and return quickly to avoid blocking MessageBroker's outgoing thread.
}

```

For every AASB message interface that you wish to handle, your application will define code like the above example in which you define a function to handle each "outgoing" message (or one function to handle all outgoing messages) and subscribe the function to `MessageBroker` by specifying the topic and action combination that the function will handle. In the implementation of the handling functions, you must return quickly and delegate any time-consuming handling to a worker thread.

When you need to publish an "incoming" message to the Engine, you can construct the message JSON string directly or, preferably because it's simpler, use the AASB message helper class to construct the message and convert it to a string. Then use `MessageBroker` to publish the message to the Engine. For example,

```cpp
#include <AASB/Message/Alexa/SpeechRecognizer/Initiator.h>
#include <AASB/Message/Alexa/SpeechRecognizer/StartCaptureMessage.h>


// call this function when the user tapped the Alexa invocation button to initiate an interaction
void SpeechRecognizerHandler::tapToTalk(Initiator initiator) {
    StartCaptureMessage msg;
    msg.payload.initiator = Initiator::TAP_TO_TALK;
    m_messageBroker->publish(msg.toString());
}
```

Only publish messages to the Engine after the Engine was started successfully (i.e., after `Engine::start()` returned `true`).

## Implement features

The [Auto SDK feature documentation](../explore/features/index.md) provides detailed documentation for each individual Auto SDK module and explains the Engine configuration and AASB messages you must implement in order to use the features the module provides. Follow the feature documentation as you integrate features into your application.

### Tip to get started quickly

To get your application up and running as quickly as possible if you wish to develop features incrementally, you can start by integrating only the `Core`, `Alexa`, `CBL`, and `System Audio` modules. In particular, follow these high level guidelines:

  * Implement the Engine lifecycle management described above.
  * Read the documentation for each of these four modules.
  * Provide the required Engine configuration for each of these modules.
  * Integrate the AASB messages for the following interfaces:
    * `Authorization`
        * Note: `CBL` completes most of the implementation for you.
    * `AudioInput` and `AudioOutput`
        * Note: `System Audio` completes the implementation for you.
    * `PropertyManager`
        * Note: You can start by handling only the properties defined by the modules you initially use
    * `SpeechRecognizer` with `TAP_TO_TALK` initiation.
  
After you have completed this bare minimum integration, you should be able to start the Engine in your application and invoke Alexa by button press to ask a simple question such as "What's the weather?"