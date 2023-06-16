# AudioPlayer Interface

When an audio media stream is received from Alexa, it is the responsibility of the platform implementation to play the stream in a platform-specific media player. The `aace::alexa::AudioPlayer` class informs the platform of the changes in player state being tracked by the Engine. This can be used to update the platform GUI, for example.

To implement a custom handler for audio player output, subscribe to `AudioPlayer` messages:

```cpp
// Include necessary message header files
#include "AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessage.h"
#include "AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessage.h"
#include "AASB/Message/Alexa/AudioPlayer/PlayerActivityChangedMessage.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

...

    // Subscribe to corresponding messages with handlers
    m_messageBroker->subscribe(
        [=](const std::string& message) {
            PlayerActivityChangedMessage msg = json::parse(message);
            // Handle player activity change
        },
        PlayerActivityChangedMessage::topic(),
        PlayerActivityChangedMessage::action());

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            GetPlayerDurationMessageReply msg = json::parse(message);
            // Handle received duration
        },
        GetPlayerDurationMessageReply::topic(),
        GetPlayerDurationMessageReply::action());

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            GetPlayerPositionMessageReply msg = json::parse(message);
            // Handle received position
        },
        GetPlayerPositionMessageReply::topic(),
        GetPlayerPositionMessageReply::action());
```

## View media metadata on screen with TemplateRuntime

Your application subscribes to the `TemplateRuntime.RenderPlayerInfo` AASB message to receive metadata about the active media playback for you to display. See the [TemplateRuntime AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html) for details about the payload.

## Manage multiple audio activities

Alexa can manage multiple audio activities at the same time. For example, an `AudioPlayer` activity can be paused in the background while another `ExternalMediaPlayer` activity is playing on the foreground. User can control these audio activities with their voice or by interacting with GUI on the vehicle. When a user makes an ambiguous request or switches activity on the GUI locally, it's necessary to let Alexa cloud know which activity is active (or in foucs) so that Alexa can respond correctly.

`SetAsForegroundActivity` message enables the app to set the Alexa `AudioPlayer` interface as the foreground player the user sees on screen. This message is useful for scenarios in which the user played an `AudioPlayer` media source, then played a different Alexa-aware `ExternalMediaPlayer` media source, such as a deep-linked media app or a local media source, and then manually returned visual activity to the Alexa `AudioPlayer` GUI. Publishing `SetAsForegroundActivity` message ensures the next VUI command or GUI interaction with the playback control buttons acts on the `AudioPlayer` source rather than the more recently played `ExternalMediaPlayer` source:

```cpp
#include "AASB/Message/Alexa/AudioPlayer/SetAsForegroundActivityMessage.h"
using namespace aasb::message::alexa::audioPlayer;
...
    SetAsForegroundActivityMessage msg;
    m_messageBroker->publish(msg);
```

Note that the `AudioPlayer` had to be previously playing at least once during this Engine cycle in order for this message to make Alexa act on `AudioPlayer`.
