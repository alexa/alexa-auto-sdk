# LocationProvider Interface

Sometimes the user asks Alexa a question that requires she know the location in order to answer properly. For example, a user in San Francisco, California might say *"Alexa, what's the weather?"*. This user probably wants to hear Alexa say something like *"The weather in San Francisco is sixty-five degrees and overcast..."* rather than something like *"I can't find your exact location right now..."*. Similarly, the user might say *"Alexa, take me to the nearest Whole Foods"* and wants Alexa to start navigation to a Whole Foods that is actually nearby.

To provide the user with accurate responses to local search commands, weather questions, and more, obtain the user's consent to share their location with Alexa and use the [`LocationProvider`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/LocationProvider/index.html) interface.

> **Note:** For Android applications, AACS provides a default implementation of `LocationProvider`. See the [AACS Default Implementation](https://alexa.github.io/alexa-auto-sdk/aacs/android#default-platform-implementation) documentation for more information.

Your application should subscribe to the [`LocationProvider.GetLocation`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/LocationProvider/index.html#getlocation) and [`LocationProvider.GetCountry`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/LocationProvider/index.html#getcountry) messages to provide location data, such as geographic coordinates and vehicle operating country, when the Engine requests it. These messages are synchronous-style and require your application to send the corresponding reply messages right away. To avoid blocking the MessageBroker outgoing thread and delaying user requests to Alexa, your application should keep the location data in a cache that you update frequently. Pull the location from the cache when the Engine requests it.

The Engine won't publish the `GetLocation` message if it knows your application has lost access to the location data. Keep the Engine in sync with the state of your application's location provider availability by proactively publishing the [`LocationServiceAccessChanged`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/LocationProvider/index.html#locationserviceaccesschanged) message at startup and each time the state changes. For example, your application might publish this message with `access` set to `DISABLED` if the system revokes your application's access to location or if GPS turns off.

> **Note:** The Engine does not persist this state across device reboots. To ensure the Engine always knows the initial state of location availability, publish a `LocationServiceAccessChanged` message each time you start the Engine. This includes notifying the Engine that `access` is `ENABLED`.

<details markdown="1">
<summary>Click to expand or collapse C++ example code</summary>

```cpp
#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Location/LocationProvider/GetCountryMessage.h>
#include <AASB/Message/Location/LocationProvider/GetLocationMessage.h>
#include <AASB/Message/Location/LocationProvider/LocationServiceAccessChangedMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class MyLocationProviderHandler {

    // Call before you start the Engine
    void MyLocationProviderHandler::subscribeToAASBMessages() {
        messageBroker->subscribe(
            [=](const std::string& message) { handleGetCountryMessage(message); },
            GetCountryMessage::topic(),
            GetCountryMessage::action());
        messageBroker->subscribe(
            [=](const std::string& message) { handleGetLocationMessage(message); },
            GetLocationMessage::topic(),
            GetLocationMessage::action());
    }

    void MyLocationProviderHandler::handleGetCountryMessage(const std::string& message) {
        GetCountryMessage msg = json::parse(message);

        // Quickly publish the GetCountry reply message
        auto country = getCountryFromCache(); // implement this stub
        GetCountryMessageReply replyMsg;
        replyMsg.header.messageDescription.replyToId = msg.header.id;
        replyMsg.payload.country = country;
        messageBroker->publish(replyMsg.toString());
    }

    void MyLocationProviderHandler::handleGetLocationMessage(const std::string& message) {
        GetLocationMessage msg = json::parse(message);

        // Quickly publish the GetCountry reply message
        auto location = getLocationFromCache(); // implement this stub
        GetLocationMessageReply replyMsg;
        replyMsg.header.messageDescription.replyToId = msg.header.id;

        // parse "location" and populate the fields of the reply message
        aasb::message::location::locationProvider::Location replyLocation;
        replyLocation.latitude = ... ; // the latitude from "location";
        replyLocation.longitude =  ... ; // the longitude from "location";
        replyMsg.payload.location = replyLocation;
        messageBroker->publish(replyMsg.toString());
    }

    // Call when the application access to location data changes
    // and after starting the Engine
    void MyLocationProviderHandler::locationServiceAccessChanged(bool hasAccess) {
        LocationServiceAccessChangedMessage msg;
        if (hasAccess) {
             msg.payload.access = aasb::message::location::locationProvider::LocationServiceAccess::ENABLED;
        } else {
            msg.payload.access = aasb::message::location::locationProvider::LocationServiceAccess::DISABLED;
        }
        messageBroker->publish(msg.toString());
    }
}

```
</details>