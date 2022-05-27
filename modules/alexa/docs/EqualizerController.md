# EqualizerController Interface

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing. 

The platform implementation is responsible for the following:

* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

You can programmatically generate Equalizer Controller configuration with details such as supported bands, default state, and decibel adjustment range using the `aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```
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

```
auto eqConfig = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
    {EqualizerBand::BASS, EqualizerBand::TREBLE},
    -8,
     8,
    { {EqualizerBand::BASS, 0}, {EqualizerBand::TREBLE, 0} } );
engine->configure( { //other config objects..., eqConfig, ... } );

...
```

To implement a custom handler for Equalizer Controller, extend the `EqualizerController` class:

```
#include <AACE/Alexa/EqualizerController.h>

using EqualizerBand = aace::alexa::EqualizerController::EqualizerBand;
using EqualizerBandLevel = aace::alexa::EqualizerController::EqualizerBandLevel;

class MyEqualizerControllerHandler : public aace::alexa::EqualizerController {
  public:
    void setBandLevels( std::vector<EqualizerBandLevel> bandLevels ) override {
    // Handle performing audio equalization on the device 
    // according to the provided band dB level settings

    // This invocation may come from "Alexa, reset bass", 
    // "Alexa, reset my equalizer", "Alexa, increase treble", etc.
    }

    std::vector<EqualizerBandLevel> getBandLevels() override {
        // Return the current band level settings on the device
        return m_currentBandLevels;
    }
};
...

// Register the platform interface with the Engine
auto m_equalizerController = std::make_shared<MyEqualizerControllerHandler>();
engine->registerPlatformInterface( m_equalizerController );

...

// If levels are adjusted using local on-device controls, call inherited methods to notify the Engine:

// To set a band to an absolute gain level in decibels
std::vector<EqualizerBandLevel> settings{ {EqualizerBand::BASS, 4} }; // Sets bass amplitude to +4dB
m_equalizerController->localSetBandLevels( settings );

// To make a relative adjustment to level settings
std::vector<EqualizerBandLevel> adjustments{ {EqualizerBand::BASS, -2} }; // Decreases bass gain by 2dB
m_equalizerController->localAdjustBandLevels( adjustments );

// To reset gain levels to the configured defaults (usually 0dB)
std::vector<EqualizerBand> bands{EqualizerBand::BASS, EqualizerBand::TREBLE}; // Resets bass and treble bands
m_equalizerController->localResetBands( bands );

```