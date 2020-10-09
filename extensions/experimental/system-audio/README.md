# Alexa Auto SDK System Audio Extension

The Alexa Auto SDK System Audio extension provides the default audio capturing and playback functionality for various platforms.

**Table of Contents**

* [What's Included](#whats-included)
* [Supported Audio Backends](#supported-audio-backends)
* [Getting Started](#getting-started)

## What's Included <a id = "whats-included"></a>
The System Audio extension contains the platform implementations of:

* `aace::audio::AudioOutput` and `aace::audio::AudioOutputProvider` for audio playback capability
* `aace::audio::AudioInput` and `aace::audio::AudioInputProvider` for audio capturing capability

## Supported Audio Backends <a id = "supported-audio-backends"></a>

Currently the System Audio extension supports:

* [GStreamer](https://gstreamer.freedesktop.org/) tested with:
  * Generic Linux (Ubuntu 18.04)
      >**Note:** You'll need extra GStreamer plugins to play content from Amazon Music (which uses HLS) and other Music Service Providers. Installing plugins such as `gst-plugins-good1.0`, `gst-plugins-bad1.0`, and `gst-plugins-ugly1.0` is recommended.
  
    **Note:** GStreamer on Poky Linux for iMX8 does not support Audible or Amazon Music playback.

  * [Automotive Grade Linux](https://www.automotivelinux.org/) with 4A framework support (FF or GG)
* [OpenMAX AL](https://www.khronos.org/openmaxal) *(Encoded audio playback only)* tested with:
  * QNX Multimedia Suite
* [QNX Sound Architecture (QSA)](http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.audio/topic/architecture_QSA.html) *(Raw audio only)* tested with:
  * QNX 7.0.0

    >**Note:** You'll need a QNX Multimedia Suite license to use OpenMAX AL, and both OpenMAX AL and QSA are required in order to enable full funcitonality on QNX.

## Getting Started

### Prerequisites

You'll need GStreamer development packages such as `libgstreamer1.0-dev` and `libgstreamer-plugins-base1.0-dev`.

### Building the Alexa Auto SDK with the System Audio Extension

The [Alexa Auto SDK Builder](../../../builder/README.md) automatically includes the System Audio extension when you build the Auto SDK. For example, assuming `AAC_SDK_HOME` is the location where you've installed the Alexa Auto SDK, use this command to build the Auto SDK for generic Linux:

```
$ cd ${AAC_SDK_HOME}
$ builder/build.sh linux -t native [options]
```

### Running the C++ Sample App with the Audio Configuration

When you run the C++ Sample App, a `config-system-audio.json` file with default audio settings for Linux platforms is installed under `/opt/AAC/etc`.

#### Changing the Default Audio Settings for Linux (required for Poky 32)

If you need to modify the configuration defined in the `config-system-audio.json` file for your Linux platform, follow these steps:

1. Open the `config.json.linux` file (located in the `aac-sdk/extensions/experimental/system-audio/modules/audio/assets` directory).
2. Edit the `config.json.linux` file as necessary.

  >**Note:** For Poky 32 boards, you must set `"shared"` to `"true"` in the `"default"` node.
3. Save the `config.json.linux` file as `config-system-audio.json` under `/opt/AAC/etc`.
4. Include  a  `--config /opt/AAC/etc/config-system-audio.json \` line when you run the C++ Sample App.

#### Changing the Default Audio Settings for QNX (required)

The default audio settings will not work for QNX. To modify the configuration defined in the `config-system-audio.json` file for your QNX platform, follow these steps: 

1. Open the `config.json.qnx` file (located in the `aac-sdk/extensions/experimental/system-audio/modules/audio/assets` directory).
2. Edit the `config.json.qnx` file as necessary. See the [default QNX configuration](#default-qnx-configuration) for guidance.
3. Save the `config.json.qnx` file as `config-system-audio.json` under `/opt/AAC/etc`.
4. Include  a  `--config /opt/AAC/etc/config-system-audio.json \` line when you run the C++ Sample App.

>**Note:** You may need to set the `AAL_CAPATH` system environment value to specify which path should OpenMAX AL used for `CURLOPT_CAPATH` internally. If you don't set the `AAL_CAPATH` system environment variable, `/etc/ssl/certs` will be used by default.

## Advanced Usage

For complex audio setup, you may need to write your own config file. To use this file, save it as `config-system-audio.json` under `/opt/AAC/etc` and include  a  `--config /opt/AAC/etc/config-system-audio.json \` line when you run the C++ Sample App.

Here is the config file template:

```
{
  "aace.systemAudio": {
    "<provider>": {
      "enabled": <boolean>,
      "devices": {
        "<device-name>": {
          "module": "<module-name>",
          "card": "<id>",
          "rate": "<sample-rate>",
          "shared": <boolean>
        }
      },
      "types": {
        "<type>": "<device-name>"
      }
    }
  }
}
```

* `aace.systemAudio.<provider>`: Set to `AudioInputProvider` or `AudioOutputProvider`. You can write a configuration for each `<provider>`, depending on the direction (input or output).
* `aace.systemAudio.<provider>.enabled`: Set to `true` or `false`. Setting this parameter to `false` disables the registration of the `<provider>` platform implementation. The default setting is `true`.
* `aace.systemAudio.<provider>.devices.<device-name>`: Set to any `"<device-name>"` or to `default`. If you set the `"<device-name>"` to `"default"` audio will be routed by default if there is no explicit `"<device-name>`" available for the configured `"<type>"`. You can configure multiple devices, depending on your platform.
  * `"module"`: Specify a `"<module-name>"` to explicitly define which audio backend to use. By default, `"module"` is set to an empty string, which configures the system audio extension to use whatever backend is available.
  * `"card"`: Specify the card id for the specific audio backend you defined with the `"<module-name>"` parameter. By default, `"card"` is set to an empty string since by default `"<module-name>"` is not defined.
  * `"rate"`: Specify the sample rate of audio input. By default the `"rate"` is set to `0`.
  * `"shared"` *(AudioInputProvider only)*: Set to `true` or `false`. Set `"shared:` to `true` for Poky 32 boards or in cases where the device should be shared within the Auto SDK Engine; otherwise, the System Audio extension will try to open the device for every audio input type. The `"shared"` option is useful when the underlying backend doesn't support the input splitter. By default `"shared"` is set to `false`.
* `aace.systemAudio.<provider>.types.<type>`: Use the `"type"` option to specify which device should be used for various types of audio. If you do not explicitly specify a device, the `default` type is used. See `aace::audio::AudioInputProvider::AudioInputType` and `aace::audio::AudioOutputProvider::AudioOutputType` for the possible `"<type>"` values.

### Default QNX Configuration <a id = "default-qnx-configuration"></a>

Here is the default configuration for QNX platforms:

```
{
  "aace.systemAudio": {
    "AudioInputProvider": {
      "devices": {
        "default": {
          "module": "QSA",
          "shared": true
        }
      }
    },
    "AudioOutputProvider": {
      "devices": {
        "default": {
          "module": "OpenMAX AL"
        },
        "raw": {
          "module": "QSA"
        }
      },
      "types": {
        "COMMUNICATION": "raw"
      }
    }
  }
}
```

If you use this configuration:
* The audio capturing for all types will use `QSA`, but it will be shared. This means that only a single PCM channel will be opened by the Engine.
* The audio playback for all types except `COMMUNICATION` will use `OpenMAX AL`. `COMMUNICATION` audio will use `QSA` instead. Note that the multiple PCM channels will be opened for each types.

### Linux Configuration Example

Here is a configuration example for Linux platforms:

```
{
  "aace.systemAudio": {
    "AudioInputProvider": {
      "devices": {
        "default": {
          "module": "GStreamer"
        },
        "loopback": {
          "module": "GStreamer",
          "card": "hw:Loopback,1,0",
          "shared": true
        }
      },
      "types": {
        "LOOPBACK": "loopback"
      }
    },
    "AudioOutputProvider": {
      "devices": {
        "default": {
          "module": "GStreamer"
        }
      }
    }
  }
}
```

The above example shows how you could provide "Speaker reference" into the engine. Note that the `card` parameter corresponds to the ALSA device name.

## Playlist URL Support

The System Audio extension supports playback of playlist URL from media streaming services (such as TuneIn) based on `PlaylistParser` provided by AVS Device SDK. The current supported formats include M3U and PLS. Note that only the first playable entry will be played in the current implementation. Choosing a variant based on stream information or continuing playback of the second or later entry is not supported right now.

## Debugging System Audio Extension

If you would like to set breakpoints in AAL, it will be good to extend the state timeout by modifying `SYSTEM_AUDIO_EXTEND_STATE_TIMEOUT` to 1 in `modules/audio/engine/CMakeLists.txt`. Extending the timeout duration (to 300 seconds) allows setting breakpoints in AAL without causing state timeout.