# TemplateRuntime Interface

Alexa sends visual metadata (display card templates) for your device to display. When template information is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the information that is received from Alexa. There are two display card template types:

* The [Template](https://alexa.design/DevDocRenderTemplate) type provides visuals associated with a user request to accompany Alexa speech.
* The [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo) type provides visuals associated with media playing through the `AudioPlayer` interface. This includes playback control buttons, which must be used with the `PlaybackController` interface.

You can programmatically generate template runtime configuration using the `aace::alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa" {
        "templateRuntimeCapabilityAgent": {
            "displayCardTTSFinishedTimeout": <TIMEOUT_IN_MS>,
            "displayCardAudioPlaybackFinishedTimeout": <TIMEOUT_IN_MS>,
            "displayCardAudioPlaybackStoppedPausedTimeout": <TIMEOUT_IN_MS>
        }
}
```

To implement a custom handler for GUI templates, extend the `TemplateRuntime` class:

```
#include <AACE/Alexa/TemplateRuntime.h>
class MyTemplateRuntime : public aace::alexa::TemplateRuntime {
    public:
        void renderTemplate( const std::string& payload, FocusState focusState ) override {
        // handle rendering the template data specified in payload
        }
        
        void renderPlayerInfo( const std::string& payload, PlayerActivity audioPlayerState, std::chrono::milliseconds offset, FocusState focusState ) override {
        // handle rendering the player info data specified in payload
        }
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyTemplateRuntime>() );
```
>**Note:** In the case of lists, it is the responsibility of the platform implementation to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of the first five elements, Alexa prompts the user whether or not to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.