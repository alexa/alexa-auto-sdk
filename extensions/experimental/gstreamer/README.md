# GStreamer Extension

The Alexa Auto SDK GStreamer extension allows you to implement interfaces to provide audio input/output functionality.

**Table of Contents**

* [Overview](#overview)
* [Supported Platforms](#supported-platforms)
* [Getting Started](#getting-started)
* [Known Problems](#known-problems)


## Overview <a id ="overview"></a>

In a typical Alexa Auto SDK application, you must implement the following interfaces to provide audio input/output functionality:

* `aace::audio::AudioOutput` and `aace::audio::AudioOutputProvider` for audio playback capability
* `aace::audio::AudioInput` and `aace::audio::AudioInputProvider` for audio capturing capability

The Alexa Auto GStreamer extension provides an example of how you can implement these interfaces.

## Supported Platforms

Currently this extension supports:

* [GStreamer](https://gstreamer.freedesktop.org/)
  * [Automotive Grade Linux](https://www.automotivelinux.org/) with 4A framework support (FF or GG)
  * Generic Linux
  * macOS

>**IMPORTANT:** You'll need extra GStreamer plugins to play content from Amazon Music (which uses HLS) and other Music Service Providers. Installing plugins such as `gst-plugins-good1.0`, `gst-plugins-bad1.0`, and `gst-plugins-ugly1.0` is recommended.

## Getting Started

### Build Alexa Auto SDK with Audio Implementation

Run Alexa Auto Builder with an extra argument (where `AAC_SDK_HOME` is the location into which you've installed the Alexa Auto SDK.):

```
$ cd ${AAC_SDK_HOME}
$ builder/build.sh oe extensions/experimental/gstreamer <args>
```

>**Note:** You'll need GStreamer devel packages such as `libgstreamer1.0-dev` and `libgstreamer-plugins-base1.0-dev`.

### Providing loopback audio

To provide `aace::audio::AudioInputProvider::AudioInputType::LOOPBACK` type of audio, you need to add this config.

```
"aace.gstreamer": {
  "AudioInputProvider": {
    "deviceNames": {
      "loopback": "hw:Loopback,1,0"
    }
  }
}
```

In above example, `hw:Loopback,1,0` is specified as loopback audio.

>**Note** The GStreamer extension expects PulseAudio for the audio inputs and outputs by default. If you use ALSA directly, your platform implementation must handle splitting/mixing audio.

## Known Issues<a id ="known-issues"></a>
* The GStreamer Audio Library implementation can get into a bad state when handling new content before existing content has finished. This issue is due to the serialization required for a common resource the Audio Library uses.
* TuneIn (including streams from many Flash Briefing and Sports Update skills) uses the .ashx playlist URL format for playback. This implementation does not include a parser for ashx audio playlists, so streams from TuneIn may not be able to play.

