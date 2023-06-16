# EqualizerController Interface

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing.

The platform implementation is responsible for the following:

* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

You can programmatically generate Equalizer Controller configuration with details such as supported bands, default state, and decibel adjustment range using the `aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```json
{
    "aace.alexa" {
        "equalizer": {
            "bands": {
                "BASS": true,
                "MIDRANGE": false,
                "TREBLE": true
            },
            "defaultState": {
                "bands": {
                    "BASS": 4,
                    "TREBLE": -1
                }
            },
            "minLevel": -6,
            "maxLevel": 6
        }
    }
}
```

For example, 2 supported bands with amplitude gains ranging from -8dB to +8dB, each with a default of 0dB

```cpp
auto eqConfig = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
    {EqualizerBand::BASS, EqualizerBand::TREBLE},
    -8,
     8,
    { {EqualizerBand::BASS, 0}, {EqualizerBand::TREBLE, 0} } );
engine->configure( { //other config objects..., eqConfig, ... } );

...
```

To implement a custom handler for Equalizer Controller, subscribe to the `EqualizerController` messages:

```cpp
#include "AASB/Message/Alexa/EqualizerController/LocalAdjustBandLevelsMessage.h"
#include "AASB/Message/Alexa/EqualizerController/LocalResetBandsMessage.h"
#include "AASB/Message/Alexa/EqualizerController/LocalSetBandLevelsMessage.h"
using namespace aasb::message::alexa::equalizerController;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

...

    m_messageBroker->subscribe(
        [=](const std::string& message) {
            GetBandLevelsMessage msg = json::parse(message);
            // ...
        },
        GetBandLevelsMessage::topic(),
        GetBandLevelsMessage::action());
    m_messageBroker->subscribe(
        [=](const std::string& message) {
            SetBandLevelsMessage msg = json::parse(message);
            // ...
        },
        SetBandLevelsMessage::topic(),
        SetBandLevelsMessage::action());
...

// If levels are adjusted using local on-device controls, call inherited methods to notify the Engine:

// To set a band to an absolute gain level in decibels
std::vector<EqualizerBandLevel> bandLevels{ {EqualizerBand::BASS, 4} }; // Sets bass amplitude to +4dB
LocalSetBandLevelsMessage msg;
msg.payload.bandLevels = bandLevels;
m_messageBroker->publish(msg);

// To make a relative adjustment to level settings
std::vector<EqualizerBandLevel> bandAdjustments{ {EqualizerBand::BASS, -2} }; // Decreases bass gain by 2dB
LocalAdjustBandLevelsMessage msg;
msg.payload.bandAdjustments = bandAdjustments;
m_messageBroker->publish(msg);

// To reset gain levels to the configured defaults (usually 0dB)
std::vector<EqualizerBand> bands{EqualizerBand::BASS, EqualizerBand::TREBLE}; // Resets bass and treble bands
LocalResetBandsMessage msg;
msg.payload.bands = bands;
m_messageBroker->publish(msg);
```