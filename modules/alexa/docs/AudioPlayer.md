# AudioPlayer Interface

When an audio media stream is received from Alexa, it is the responsibility of the platform implementation to play the stream in a platform-specific media player. The `aace::alexa::AudioPlayer` class informs the platform of the changes in player state being tracked by the Engine. This can be used to update the platform GUI, for example.

To implement a custom handler for audio player output, extend the `AudioPlayer` class:

```
#include <AACE/Alexa/AudioPlayer.h>
class MyAudioPlayer : public aace::alexa::AudioPlayer {
    public:
        void playerActivityChanged( PlayerActivity state ) override {
            // on state change, update playback control UI
        }
};
...
        
// Register the platform interface with the Engine
auto myAudioPlayer = std::make_shared<MyAudioPlayer>();
engine->registerPlatformInterface( myAudioPlayer );
```

## View media metadata on screen with TemplateRuntime

Your application subscribes to the `TemplateRuntime.RenderPlayerInfo` AASB message to receive metadata about the active media playback for you to display. See the [TemplateRuntime AVS documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html) for details about the payload. 