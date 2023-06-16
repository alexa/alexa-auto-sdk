# PlaybackController Interface

The Engine provides a platform interface `aace::alexa::PlaybackController` for the platform implementation to report on-device transport control button presses for media playing through Alexa. For example, if the user presses the on-screen pause button while listening to Amazon Music through Alexa's `AudioPlayer` interface, the platform implementation publishes a `PlaybackController` message to report the button press to the Engine.

>**Note:** `PlaybackController` method calls to manage `AudioPlayer`'s state or playback queue proactively report button presses or the equivalent so that Alexa can react; they do *not* report changes to the playback state that happen locally first. The Alexa cloud manages the playback queue for `AudioPlayer` content, so each `PlaybackController` publishing is a request for Alexa to act on the user's local request. The result of the request will come as one or more messages on the `AudioOutput` associated with the channel used for `AudioPlayer`.

> **Note:** If your implementation needs to stop `AudioPlayer` media in response to system events, such as audio focus transitions to audio playing outside the scope of the Auto SDK, use `PlaybackController` to notify the Engine of such changes. However, keep in mind that the expected usage of the interface does not change when it is used in this use case.

>**Note:** `PlaybackController` only controls media coming from Alexa, i.e. the `AudioPlayer`. `PlaybackController` should not be used with the expectation of controlling playback for non-media Alexa audio sources like `SpeechSynthesizer` or Alexa-aware external media sources integrated with `ExternalMediaAdapter` or `LocalMediaSource`. Additionally, pulishing a `PlaybackController` message while audio is playing through another Alexa-aware external media source will produce unexpected results and is not recommended.

Whenever Alexa plays media through `AudioPlayer`, the Engine sends `TemplateRuntime.RenderPlayerInfo` message to provide visual metadata associated with the media that your implementation should render for the end user. The payload of this message includes descriptions of GUI controls to be displayed and the state in which to display them. When the user interacts with these on-screen controls, your implementation must use the `PlaybackController` interface to report the button presses to the Engine.

The table below maps the controls from the `RenderPlayerInfo` message payload to the corresponding calls in `PlaybackController`.

| RenderPlayerInfo control name | PlaybackController |
|:---|:--- |
| **PlaybackButton** | |
| "PLAY_PAUSE" | PLAY |
| "PLAY_PAUSE" | PAUSE |
| "NEXT" | NEXT |
| "PREVIOUS" | PREVIOUS |
| "SKIP_FORWARD" | SKIP_FORWARD |
| "SKIP_BACKWARD" | SKIP_BACKWARD |
| **PlaybackToggle** |
| "SHUFFLE" | SHUFFLE |
| "LOOP" | LOOP |
| "REPEAT" | REPEAT |
| "THUMBS_UP" | THUMBS_UP |
| "THUMBS_DOWN" | THUMBS_DOWN |

Implement a playback controller by publishing `PlaybackController` messages:

```cpp
#include <AASB/Message/Alexa/PlaybackController/ButtonPressedMessage.h>
#include <AASB/Message/Alexa/PlaybackController/TogglePressedMessage.h>
using namespace aasb::message::alexa::playbackController;

...
    // Publish the ButtonPressedMessage for the button pressed.
    ButtonPressedMessage msg;
    msg.payload.button = PlaybackButton::PLAY;
    m_messageBroker->publish(msg);

    // Publish the TogglePressedMessage for the button pressed.
    TogglePressedMessage msg;
    msg.payload.toggle = PlaybackToggle::SHUFFLE;
    msg.payload.action = true;
    m_messageBroker->publish(msg);
```