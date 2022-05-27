# PropertyManager Interface <!-- omit in toc -->

## Overview

Certain modules in the Auto SDK define constants (for example `FIRMWARE_VERSION` and `LOCALE`) that are used to get and set the values of runtime properties in the Engine. Changes to property values may also be initiated from the Alexa Voice Service (AVS). For example, the `TIMEZONE` property may be changed through AVS when the user changes the timezone setting in the Alexa Companion App.

The Auto SDK `Core` module provides the Property Manager service with corresponding AASB interface [`PropertyManager`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/PropertyManager/index.html). Property Manager maintains the runtime properties by storing properties and listeners to the properties and delegating the `SetProperty` and `GetProperty` messages calls from your application to the respective Engine services. The Engine also publishes `PropertyChanged` and `PropertyStateChanged` messages to notify your application about property value changes originating in the Engine.

## Use the Property Manager AASB messages

To change a property value, publish a `SetProperty` message. The Engine publishes a `PropertyStateChanged` message indicating the success or failure of the request.

To retrieve a property value, publish a `GetProperty` message. The Engine publishes synchronous-style a `GetProperty` reply with the value of the property.

When a change in a property value occurs in the Engine that is not initiated by your application, the Engine publishes a `PropertyChanged` message. 

<details><summary>Click to expand or collapse C++ example code</summary>

```cpp
#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/PropertyManager/PropertyManager/GetPropertyMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/PropertyChangedMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/SetPropertyMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class MyPropertyManagerHandler {

    // Subscribe to messages from the engine
    void MyPropertyManagerHandler::subscribeToAASBMessages() {
        m_messageBroker->subscribe(
            [=](const std::string& message) { handlePropertyChangedMessage(message); },
            PropertyChangedMessage::topic(),
            PropertyChangedMessage::action());
        m_messageBroker->subscribe(
            [=](const std::string& message) { handlePropertyStateChangedMessage(message); },
            PropertyStateChangedMessage::topic(),
            PropertyStateChangedMessage::action());
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleGetPropertyReplyMessage(message); },
            GetPropertyMessage::topic(),
            GetPropertyMessage::action());
    }

    void MyPropertyManagerHandler::handlePropertyChangedMessage(const std::string& message) {
        PropertyChangedMessage msg = json::parse(message);

        std::string name = msg.payload.name;
        std::string newValue = msg.payload.newValue;
        
        // ...Handle property changed...
    }

    void MyPropertyManagerHandler::handlePropertyStateChangedMessage(const std::string& message) {
        PropertyStateChangedMessage msg = json::parse(message);

        std::string name = msg.payload.name;
        std::string value = msg.payload.value;
        std::string state = msg.payload.state
        
        // ...Handle property state changed...
    }


    void MyPropertyManagerHandler::handleGetPropertyReplyMessage(const std::string& message) {
        GetPropertyMessageReply msg = json::parse(message);

        std::string messageId = msg.header.messageDescription.replyToId;
        std::string value = msg.payload.value;

        // ...Handle the value for the message...
    }

    // Call to set a property
    void MyPropertyManagerHandler::setProperty(const std::string& name, const std::string& value) {
]        SetPropertyMessage msg;
        msg.payload.name = name;
        msg.payload.value = value;

        m_messageBroker->publish(msg.toString());
    }

    // Call to get a property
    std::string MyPropertyManagerHandler::getProperty(const std::string& name) {
        GetPropertyMessage msg;
        msg.payload.name = name;
        m_messageBroker->publish(msg.toString());
        
        // The Engine will send the GetProperty reply message
        // Return the value from reply message payload
    }
}
```
</details>


## Property Definitions

The definitions of the properties used with the `SetProperty` and `GetProperty` messages are defined by the Auto SDK modules that manage the properties.

### Alexa 

The `Alexa` module provides the following properties:

- **"aace.alexa.wakewordSupported"** This read-only property is used with `GetProperty` to check if wake word support is enabled in the Engine. If wake word is not supported in the Engine, attempts to enable wake word with the `SpeechRecognizer` will fail.

- **"aace.alexa.system.firmwareVersion"** This property is used with `SetProperty` to change the firmware version that is reported to AVS. The value must be a positive, 32-bit signed integer represented as a string.

- **"aace.alexa.setting.locale"** This property is used with `SetProperty` to change the current locale setting for Alexa. The value must be one of the following:
     - A valid locale accepted by AVS as a string. E.g. "en-US"
     - A valid locale pair. The format is a string containing two valid locales separated with a forward slash. E.g. "en-US/es-US"
     
    > **Note:** For a list of the Alexa Voice Service (AVS) supported locales, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/docs/alexa-voice-service/system.html#locales)

- **"aace.alexa.countrySupported"** This read-only property is used with `GetProperty` to check if the vehicle's country is supported.

- **"aace.alexa.wakewordEnabled"** This property is used with `SetProperty` to change the current wake word enabled setting. The value must be a boolean represented as a string, i.e. "true" or "false". Use `GetProperty` with this property to check whether wake word is enabled.
    - > **Note:** The Engine does not persist this setting across device reboots.

- **"aace.alexa.timezone"** This property is used with `SetProperty` to change the current timezone setting of the device. The value must be a valid timezone accepted by AVS. Use `GetProperty`to get the Engine's current timezone setting.

### Core

The `Core` module provides the following properties:

- **"aace.core.version"** This property is used with `GetProperty` to return the Auto SDK version.

- **"aace.vehicle.operatingCountry"** This property is used with `SetProperty` to change the current operating country. The value must be a valid 2-letter ISO country code.

- **"aace.network.networkInterface"** This property is used with `SetProperty` to set the network interface for the network connection. The value must be an IP address or network interface name.

- **"aace.network.httpProxyHeaders"** This property is used with `SetProperty` to set the custom HTTP header to pass in the HTTP request sent to a proxy. 
    The headers should be `\n` separated. For example, 
    `"Proxy-Authorization: Bearer 1234"` (should not be CRLF-terminated)
    
    > **Note:** To apply the custom headers you are required to specify the `CURLOPT_PROXY` in the Engine configuration. The specified headers will be applied to all subsequent requests sent to a proxy.

