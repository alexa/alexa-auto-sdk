# Notifications Interface

It is the responsibility of the platform implementation to provide a visual indication to the user when notifications (for example, package shipment notifications, notifications from skills, etc.) are available from Alexa. See the [AVS Notifications interface documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/notifications.html) for more information about notifications. The Engine uses the registered Notifications implementation to notify you when a notification indicator should be displayed or removed. It does not give any information about the notifications. Audio playback for the notification is handled by whichever audio channel is assigned to the `NOTIFICATION` type.

To implement a custom handler for Notifications, subscribe to `Notifications` messages:

```cpp
#include <AASB/Message/Alexa/Notifications/SetIndicatorMessage.h>
#include <AASB/Message/Alexa/Notifications/OnNotificationReceivedMessage.h>
using namespace aasb::message::alexa::notifications;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

...

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            OnNotificationReceivedMessage msg = json::parse(message);
            handleOnNotificationReceived(msg);
        },
        OnNotificationReceivedMessage::topic(),
        OnNotificationReceivedMessage::action());

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            SetIndicatorMessage msg = json::parse(message);
            handleSetIndicatorMessage(msg);
        },
        SetIndicatorMessage::topic(),
        SetIndicatorMessage::action());
```