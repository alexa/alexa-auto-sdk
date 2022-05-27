# Alexa Auto SDK System Audio Module

The `System Audio` module provides the default audio capturing and playback functionality for macOS, Linux, and QNX.

## What's Included

The `System Audio` module contains the platform implementations of:

* `aace::audio::AudioOutput` and `aace::audio::AudioOutputProvider` for audio playback capability
* `aace::audio::AudioInput` and `aace::audio::AudioInputProvider` for audio capturing capability

## Supported Audio Backends

Currently the `System Audio` module supports:

* [GStreamer](https://gstreamer.freedesktop.org/) tested on:

    * Ubuntu Linux (18.04 and 20.04)

        > **Note:** Ensure installing at least `gstreamer1.0-plugins-base` and `gstreamer1.0-plugins-good`. It is recommended to install `gstreamer1.0-plugins-bad` for playing content from Amazon Music (which uses HLS) and other Music Service Providers. Refer to https://gstreamer.freedesktop.org/documentation/installing/on-linux.html for instructions.

        **Note:** GStreamer on Poky Linux for iMX8 does not support Audible or Amazon Music playback.

    * [Automotive Grade Linux](https://www.automotivelinux.org/) with 4A framework support (FF or GG)

    * Poky Linux armv7hf and armv8

    * macOS x86_64 with GStreamer installed by Homebrew

* [OpenMAX AL](https://www.khronos.org/openmaxal) *(Encoded audio playback only)* tested with:

    * QNX Multimedia Suite 2.0 on QNX 7.0.0 armv8

* [QNX Sound Architecture (QSA)](http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.audio/topic/architecture_QSA.html) *(Raw audio only)* tested with:

    * QNX 7.0.0 armv8

        > **Note:** You'll need a QNX Multimedia Suite license to use OpenMAX AL, and both OpenMAX AL and QSA are required in order to enable full functionality on QNX.

## Getting Started

### Prerequisites

You'll need GStreamer development packages such as `libgstreamer1.0-dev` and `libgstreamer-plugins-base1.0-dev`.

### Building the Alexa Auto SDK with the System Audio Module

The [Alexa Auto SDK Builder Tool](https://alexa.github.io/alexa-auto-sdk/docs/native/building) automatically includes the `System Audio` module when you build the C++ sample app. You can include it explicitly by specifying `-o aac_modules=aac-module-system-audio,...` to the build command.

### Running the C++ Sample App with the Audio Configuration

When you run the C++ Sample App, a `config-system-audio.json` file with default audio settings for Linux platforms is provided under `shared/system-audio/config`.

#### Changing the Default Audio Settings for Linux (required for Poky 32)

If you need to modify the configuration defined in the `config-system-audio.json` file for your Linux platform, follow these steps:

1. Edit the `config-system-audio.json` file (located in the `aac-sdk/modules/system-audio/configs/linux` directory) as necessary.

    > **Note:** For Poky 32 boards, you must set `"shared"` to `"true"` in the `"default"` node.

2. Save the `config-system-audio.json` file to the same directory other config files reside.

3. Include a  `--config path/to/config-system-audio.json` line when you run the C++ Sample App.

#### Changing the Default Audio Settings for QNX (required)

The default audio settings will not work for QNX. To modify the configuration defined in the `config-system-audio.json` file for your QNX platform, follow these steps:

1. Edit the `config-system-audio.json` file (located in the `aac-sdk/modules/system-audio/configs/neutrino` directory) as necessary. See the [default QNX configuration](#default-qnx-configuration) for guidance.
2. Save the `config-system-audio.json` file to the same directory other config files reside.
3. Include a `--config /opt/AAC/etc/config-system-audio.json` line when you run the C++ Sample App.

> **Note:** You may need to set the `AAL_CAPATH` system environment value to specify which path should OpenMAX AL used for `CURLOPT_CAPATH` internally. If you don't set the `AAL_CAPATH` system environment variable, `/etc/ssl/certs` will be used by default.

## Configuring System Audio

For complex audio setup, you may need to write your own config file. To use this file, save it as `config-system-audio.json` and include a `--config path/to/config-system-audio.json` line when you run the C++ Sample App.

Here is the config file template:

```
{
  "aace.systemAudio": {
    "<provider>": {
      "enabled": {{BOOLEAN}},
      "devices": {
        "<device-name>": {
          "module": "<module-name>",
          "card": "<id>",
          "rate": "<sample-rate>",
          "shared": {{BOOLEAN}}
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
    * `"module"`: Specify a `"<module-name>"` to explicitly define which audio backend to use. By default, `"module"` is set to an empty string, which configures the system audio module to use whatever backend is available.
    * `"card"`: Specify the card id for the specific audio backend you defined with the `"<module-name>"` parameter. By default, `"card"` is set to an empty string since by default `"<module-name>"` is not defined.
    * `"rate"`: Specify the sample rate of audio input. By default the `"rate"` is set to `0`.
    * `"shared"` *(AudioInputProvider only)*: Set to `true` or `false`. Set `"shared"` to `true` for Poky 32 boards or in cases where the device should be shared within the Auto SDK Engine; otherwise, the System Audio module will try to open the device for every audio input type. The `"shared"` option is useful when the underlying backend doesn't support the input splitter. By default `"shared"` is set to `false`.
* `aace.systemAudio.<provider>.types.<type>`: Use the `"type"` option to specify which device should be used for various types of audio. If you do not explicitly specify a device, the `default` type is used. See `aace::audio::AudioInputProvider::AudioInputType` and `aace::audio::AudioOutputProvider::AudioOutputType` for the possible `"<type>"` values.

### Default QNX Configuration <a id = "default-qnx-configuration"></a>

Here is the default configuration for QNX platforms:

```json
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

```json
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

The above example shows how you could provide "Speaker reference" into the engine.

### Specifying Input and Output Device

The `card` field specifies the audio input source and audio output sink.

For devices using module `GStreamer`, the `card` field can be one of the following values:

- If the `card` field is an empty string or not specified at all, the input device will be decided by GStreamer `autoaudiosrc` plugin and the output device will be decided by GStreamer `autoaudiosink` plugin automatically.
- If the the `card` field starts with `bin:`, the input and output device can be specified with a GStreamer bin description. For example, setting `card` of an audio input device to `bin:udpsrc port=5000 caps=\"application/x-rtp,channels=(int)1,format=(string)S16LE,media=(string)audio,payload=(int)96,clock-rate=(int)16000,encoding-name=(string)L16\" ! rtpL16depay` will receive audio from local UDP port 5000. See [Pipeline Description](https://gstreamer.freedesktop.org/documentation/tools/gst-launch.html?gi-language=c#pipeline-description) for details.
- If the `card` field starts with `element:`, the input and output device can be specified with a GStreamer element. For example, setting `card` of an audio output device to `element:pulsesink` will deliver the output audio to the PulseAudio server. See [Plugins](https://gstreamer.freedesktop.org/documentation/plugins_doc.html?gi-language=c) for the list of GStreamer plugins.
- If none of the above matches, the string will be treated as an Advanced Linux Sound Architecture (ALSA) device name. You can use `aplay -L` or `arecord -L` to list available audio output and input devices.

One practical use of the `card` field is to specify virtual audio input/output for the following devices:

- The target device does not have audio input/output hardware.
- The target device is remotely located.

Here is an example to use local microphone and speaker and run C++ sample app on a remote device:

- Local macOS/Linux device
    - Deliver audio from local microphone to remote UDP port 5000 by running `gst-launch-1.0 -v autoaudiosrc ! "audio/x-raw, format=(string)S16LE, channels=(int)1, layout=(string)interleaved" ! audioresample ! audio/x-raw, rate=16000 ! audioconvert ! rtpL16pay ! udpsink host=your.remote.device port=5000`.
    - Build a reverse SSH tunnel by running `ssh -R 24713:localhost:4713 your.remote.device`.
    - Play audio output received from remote device by running `pulseaudio --load=module-native-protocol-tcp --exit-idle-time=-1 --daemon`.
- Remote device
    - Receive audio input from UDP port 5000 by specifying `card` of audio input device to `bin:udpsrc port=5000 caps=\"application/x-rtp,channels=(int)1,format=(string)S16LE,media=(string)audio,payload=(int)96,clock-rate=(int)16000,encoding-name=(string)L16\" ! rtpL16depay`.
    - Send audio output to local device by specifying `card` of audio output device to `element:pulsesink` and export `PULSE_SERVER` environment variable to `tcp:localhost:24713` before running C++ sample app.

## Playlist URL Support

The System Audio module supports playback of playlist URL from media streaming services (such as TuneIn) based on `PlaylistParser` provided by AVS Device SDK. The current supported formats include M3U and PLS. Note that only the first playable entry will be played in the current implementation. Choosing a variant based on stream information or continuing playback of the second or later entry is not supported right now.

After the user asks Alexa to play on TuneIn, if Alexa acknowledges the request but says TuneIn is not available, the parser displays the following error:

```
2021-03-02 05:04:25.745 [AVS] E PlaylistParser:doDepthFirstSearch:url=http\://www.podtrac.com/pts/redirect.mp3/chtbl.com/track/5899E/traffic.megaphone.fm/HSW1953246087.mp3:getHeaderFailed
```

To avoid this error, provide a valid `cacert.pem` to `CURLOPT_CAINFO` in the Auto SDK configuration. Download the `cacert.pem` file from [here](https://curl.se/docs/caextract.html).

```
      "libcurlUtils": {
        "CURLOPT_CAPATH": "/path/to/certs",
        "CURLOPT_CAINFO": "/path/to/cacert.pem"
      }
```
