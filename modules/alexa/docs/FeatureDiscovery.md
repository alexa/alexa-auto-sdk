# FeatureDiscovery Interface

## Overview

The `Feature Discovery` interface enables your Alexa Auto SDK client application to dynamically retrieve the Alexa utterances (hints) from the cloud, helping the customers to discover Alexa features and learn how to use Alexa in the different domains.

To use the Feature Discovery functionality, your product must be onboarded and placed on the allow list by Amazon. Contact your Amazon Solutions Architect (SA) or Partner Manager for details.

Auto SDK provides the [FeatureDiscovery](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/FeatureDiscovery/index.html) AASB message interface for your application to request a list of suggested utterances. In your application, publish the `GetFeatures` message to request a list of utterances associated with the specified `domain` and `eventType`. Subscribe to the `GetFeatures` reply message to receive the response.

> **Note:** The Auto SDK Engine does not cache the suggested utterances returned by Alexa. Therefore, your application is responsible for implementing caching mechanism for the utterances and deciding when to refresh the local cache.

## GetFeatures Request
The `GetFeatures` message requests the suggested utterances from Alexa. The `discoveryRequests` field is a string containing an escaped JSON with the following format:
```
[
    {
        "locale" : {{String}},
        "domain" : {{String}},
        "eventType" : {{String}},
        "limit": {{Integer}}
    },
    ...
]
```

Definition of the DiscoveryRequests JSON array:

| Property | Type | Required | Description |
|-|-|-|-|
| discoveryRequests | List\<DiscoveryRequest\> | Yes | An array of feature discovery requests |  

Definition of each DiscoveryRequest JSON Object:

| Property | Type | Required| Description | Example |
|-|-|-|-|-|
|domain | String | Yes | The category of the utterances to be returned. See the [Domain and EventType](#domain-and-eventtype) section for the accepted values. | "ENTERTAINMENT" |
|eventType | String | Yes | The event type of the utterances to be returned. The event type specifies what action or state change happened. See the [Domain and EventType](#domain-and-eventtype) section for the accepted values. | "SETUP" |
|locale	| String | No | The locale of the utterances to be returned. If omitted, the Alexa locale retrieved by `PropertyManager` will be used in the request. For a list of the Alexa Voice Service (AVS) supported locales, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/docs/alexa-voice-service/system.html#locales).| "en-US"|
|limit|Integer| No |The maximum number of utterances to return. The default value is 1. | 5 |

> **Note:** When requesting the utterances, you can combine multiple discovery requests in one `GetFeatures` message by specifying multiple discovery request objects in the `discoveryRequests` JSON array. The Auto SDK Engine will reply with a single `GetFeatures` message that contains a merged response of the multiple requests. If you specify multiple request objects in the `GetFeatures` message, expect more latency to receive the message reply.


### Domain and EventType
Each utterance configured in the Alexa cloud is associated with a scenario, which is a combination of `domain` and `eventType`. The combination determines where the utterance should be displayed. 
When requesting Alexa utterances, your application must specify the `domain` and `eventType` values in the `GetFeatures` message payload.

| Property | Type| Accepted Values |
|-|-|-|
|domain|String|"GETTING_STARTED", "TALENTS", "ENTERTAINMENT", "COMMS", "WEATHER", "SMART_HOME", "NEWS", "NAVIGATION", "TRAFFIC", "SKILLS", "LISTS", "SHOPPING", "QUESTIONS_ANSWERS", "SPORTS" or "CALENDAR". |
|eventType|String|"THINGS_TO_TRY" or "SETUP".|

The valid combinations of `domain` and `eventType` are as follows:

| Domain | EventType | Scenario | Example Utterance|
|-|-|-|-|
| "GETTING_STARTED" | "SETUP"  | Hints displayed on the success page of the Alexa setup flow. | "Alexa, play music."<br /> "Alexa, find a nearby gas station." |
| "GETTING_STARTED" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Getting Started" category. | "Alexa, what’s the weather?" <br /> "Alexa, what's my Flash Briefing?"|
| "TALENTS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Alexa's Talents" category. | "Alexa, tell me a story."|
| "ENTERTAINMENT" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Entertainment" category. | "Alexa, play rock music from the 70s."|
| "COMMS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Communication" category. | "Alexa, send a message to John."|
| "WEATHER" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Weather" category. | "Alexa, will it rain today?"|
| "SMART_HOME" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Smart Home" category. | "Alexa, lock the front door."|
| "NEWS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "News and Information" category. | "Alexa, what’s the news?"|
| "NAVIGATION" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Navigation" category. | "Alexa, take me to the airport."|
| "TRAFFIC" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Traffic Information" category. | "Alexa, how’s my commute to work?"|
| "SKILLS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Skills" category. | "Alexa, what automotive skills do you have?" |
| "LISTS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Lists" category. | "Alexa, what's on my to-do list?" |
| "SHOPPING" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Shopping" category. | "Alexa, reorder toothpaste."|
| "QUESTIONS_ANSWERS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Questions and Answers" category. | "Alexa, how far away is the moon?"|
| "SPORTS" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Sports" category. | "Alexa, what's my sports update?"| 
| "CALENDAR" | "THINGS_TO_TRY"  | Hints displayed in the Things-to-Try settings menu under the "Calendar" category. | "Alexa, add a 2:00 PM coffee chat to my calendar."|
</details>

### GetFeatures Reply
The `GetFeaturesReply` message returns the suggested utterances from the `GetFeatures` message request. The `discoveryResponses` field is a string containing an escaped JSON with the following format:
```
[
  {
    "domain" : {{String}},
    "eventType" : {{String}},
    "locale" : {{String}},
    "localizedContent": [
       {
            "utteranceText": {{String}},
            "descriptionText": {{String}}
       }
    ]
  }, ...
 
]
```

Definition of DiscoveryResponses JSON array:

| Property | Type | Required | Description |
|-|-|-|-|
| discoveryResponses | List\<DiscoveryResponse\> | Yes | An array of feature discovery responses |  

Definition of each DiscoveryResponse JSON Object:

| Property | Type| Description | Example |
|-|-|-|-|
|domain | String | The category of the utterances received. See the [Domain and EventType](#domain-and-eventtype) section for the accepted values. | "ENTERTAINMENT" |
|eventType | String  | The event type of the utterances received. The event type specifies what action or state change happened. See the [Domain and EventType](#domain-and-eventtype) section for the accepted values. | "SETUP" |
|locale	| String  | The locale of the utterances received. | "en-US"|
|localizedContent| List\<LocalizedHint\> | An array of LocalizedHint objects.|-|

Definition of the LocalizedHint JSON Object:

| Property | Type| Description | Example |
|-|-|-|-|
|utteranceText | String | The exact utterance for the feature. The utterance is represented in plain text. | "Alexa, what time is it?"|
|descriptionText | String  | The description of the utterance. It can be an empty string if no description is found. | "You can ask Alexa about the time." |

## Integrating the FeatureDiscovery messages Into Your Application

### C++ MessageBroker Integration

Use the Engine's `MessageBroker` to publish `FeatureDiscovery` AASB messages and subscribe to their replies.

<details markdown="1"><summary>Click to expand or collapse C++ sample code</summary>
<br></br>

```cpp
#include <AACE/Core/MessageBroker.h>
#include <AASB/Message/Alexa/FeatureDiscovery/GetFeaturesMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class MyFeatureDiscoveryHandler {

    // Subscribe to reply messages from the Engine
    void MyFeatureDiscoveryHandler::subscribeToAASBMessages() {
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleGetFeaturesReplyMessage(message); },
            GetFeaturesMessageReply::topic(),
            GetFeaturesMessageReply::action());
    }


    // Handle the GetFeaturesReply message from the Engine
    void MyFeatureDiscoveryHandler::handleGetFeaturesReplyMessage(const std::string& message) {
        GetFeaturesMessageReply msg = json::parse(message);
        parseDiscoveryResponses(msg.payload.discoveryResponses);
    }
    void MyFeatureDiscoveryHandler::parseDiscoveryResponses(const std::string& discoveryResponses) {
        const auto& responseArray = json::parse(discoveryResponses);
        if (responseArray.empty()) {
            // No feature was discovered, returning
        } else {
            for (const auto& response : responseArray) {
                if (!response.contains("localizedContent") || !response["localizedContent"].is_array()) {
                    continue;
                }
                for (const auto& feature : response["localizedContent"]) {
                    if (feature.contains("utteranceText")) {
                        // On another thread, do something with the utterances received...
                    }
                }
            }
        }
    }
    
    // Construct and send the FeatureDiscovery requests.
    void MyFeatureDiscoveryHandler::getFeatures(const std::string& domain) {
        json requestsArray = json::array();
        requestsArray.push_back({{"domain", domain}, {"eventType", "THINGS_TO_TRY"}, {"limit", 5}});
        GetFeaturesMessage msg;
        msg.payload.discoveryRequests = requestsArray.dump();
        m_messageBroker->publish(msg.toString());
    }
};

```

</details>

### Android Integration

The Alexa Auto Client Service (AACS) sample app provides a sample implementation to integrate the Auto SDK `FeatureDiscovery` messages on Android. See the [Alexa Auto Settings App Component](https://alexa.github.io/alexa-auto-sdk/docs/android/aacs/app-components/alexa-auto-settings/) for the reference implementation.
