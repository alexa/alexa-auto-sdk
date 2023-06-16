# DoNotDisturb Interface

The DoNotDisturb (DND) interface allows users to block all incoming notifications, announcements, and calls to their devices, and to set daily recurring schedules that turn DND off and on.  For details, see the [DND Interface documentation](https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html). The Engine uses the registered DND implementation to notify the client when DND has been set or unset. A user's voice request to change the DND state triggers audio playback, but no audio playback occurs when a user sets the DND state using the touch screen.

To implement a custom handler for DND extend the `DoNotDisturb` class:

```cpp
#include "AASB/Message/Alexa/DoNotDisturb/SetDoNotDisturbMessage.h"
#include "AASB/Message/Alexa/DoNotDisturb/DoNotDisturbChangedMessage.h"
using namespace aasb::message::alexa::doNotDisturb;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

...

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            SetDoNotDisturbMessage msg = json::parse(message);

        },
        SetDoNotDisturbMessage::topic(),
        SetDoNotDisturbMessage::action());

    // Publish the "DoNotDisturbChanged" message
    DoNotDisturbChangedMessage msg;
    msg.payload.doNotDisturb = true;
    m_messageBroker->publish(msg);
```
