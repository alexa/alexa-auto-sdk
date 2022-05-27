# AlexaSpeaker Interface

The Alexa service keeps track of two device volume types: `ALEXA_VOLUME` and `ALERTS_VOLUME`. The `aace::alexa::AlexaSpeaker` class should be implemented by the platform to both set the volume and mute state of these two speaker types and allow the user to set the volume and mute state of these two speaker types locally via GUI if applicable. 

`SpeakerManager` is a configurable option, enabled by default. When not enabled, user requests to change the volume or mute now have an appropriate Alexa response, e.g. "Sorry, I can't control the volume on your device".

You can programmatically generate speaker manager configuration using the `aace::alexa::config::AlexaConfiguration::createSpeakerManagerConfig()` factory method, or provide the equivalent JSON values in a configuration file.
```
{
    "aace.alexa": {
        "speakerManager": {
            "enabled": false
        }
    }
}
```

## Set a custom volume range

You can use a custom volume control to support an Alexa device's native input volume range. By default, Alexa supports voice utterances that specify volume values between 0 and 10, but some devices may support a different range (i.e. 0 to 100). By placing on Amazon's allow list your Alexa device's volume range for your target platform, you can specify input volume levels per your device's range. Your device's input volume range is then mapped appropriately to the Alexa volume range.

Contact your Alexa Auto Solution Architect (SA) for help with allow lists. Placing a device on the allow list requires the following parameters:

* DeviceTypeID: `<YOUR_DEVICE_TYPE_ID>`
* Min: `<YOUR_MIN_VOLUME_VALUE>`
* Max: `<YOUR_MAX_VOLUME_VALUE>`

This does not impact the range used in the directives to the device. You must continue to use the SDK 0-100 volume range used by `AudioOutput` and `AlexaSpeaker` and map these values to the correct range in your implementation.
