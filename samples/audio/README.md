# Alexa Auto Sample Audio Implementation

## Overview

In a typical Alexa Auto SDK application, users are required to implement the following interfaces to provide audio input/output functionality:

* `aace::alexa::MediaPlayer` for audio playback capability
* `aace::alexa::Speaker` for audio playback volume control capability
* `aace::alexa::SpeechRecognizer` for audio capturing capability

This project shows an example of how you can implement these interfaces. It includes an output channel implementation of `MediaPlayer` and `Speaker` that you may use instead of directly implementing these interfaces yourself and an input channel implementation to support your custom `SpeechRecognizer`.

## Supported Platforms

Currently this project supports:

* [GStreamer](https://gstreamer.freedesktop.org/)
  * [Automotive Grade Linux](https://www.automotivelinux.org/) with 4A framework support (FF or GG)
  * Generic Linux
  * macOS

>**IMPORTANT:** You'll need extra GStreamer plugins to play content from Amazon Music (which uses HLS) and other Music Service Providers. Installing plugins such as `gst-plugins-good1.0`, `gst-plugins-bad1.0`, and `gst-plugins-ugly1.0` is recommended.

## Getting Started

### Build Alexa Auto SDK with Audio Implementation

Run Alexa Auto Builder with an extra argument (where `AAC_SDK_HOME` is the location you've installed the Alexa Auto SDK.):

```
$ cd ${AAC_SDK_HOME}
$ builder/build.sh oe samples/audio <args>
```

>**Note:** You'll need GStreamer devel packages such as `libgstreamer1.0-dev` and `libgstreamer-plugins-base1.0-dev`.

The generated image file will include an extra library `libAACEGStreamerAudio`, which you can utilize in your application.

### How to use in your application

Use the `aace::audio::AudioManager` class to initialize various audio channels. Note that you should only create one instance within your application.

#### Audio output channels

In the example snippet below, we open the audio channel for `Content`. `contentChannel` has the implementation of `aace::alexa::MediaPlayer` and `aace::alexa::Speaker`, so you can use them directly within your `aace::alexa::SpeechSynthesizer` implementation.

```
#include <AACE/Audio/AudioManager.h>

....

auto audioManager = aace::audio::AudioManager::create();
auto contentChannel = audioManager->openOutputChannel("Content");
```

#### Audio input channel

For the input channel, the `aace::audio::AudioCapture` interface is provided for your application. The snippet below shows how you can use it within your `aace::alexa::SpeechRecognizer` implementation.

```

#include <AACE/Audio/AudioManager.h>

...

// Initialize input channel, and keep it within our class.
auto dialogChannel = audioManager->openInputChannel("Dialog");
m_audioCapture = dialogChannel.audioCapture;

...

bool SpeechRecognizerHandler::startAudioInput() {
    return m_audioCapture->startAudioInput([this](const int16_t* data, const size_t size) {
        return this->write(data, size);
    });
}

bool SpeechRecognizerHandler::stopAudioInput() {
    return m_audioCapture->stopAudioInput();
}


```

#### Tips for AGL

1. When creating `aace::audio::AudioManager`, you need to pass `afb_api_t` structure as an argument.

   ```
   int your_binding_init(afb_api_t api)
   {
       ...
       auto audioManager = aace::audio::AudioManager::create(api);
       ...
   }
   ```

2. When opening output channels, you need to specify 4A roles for each channel. The snippet below shows an example of opening the `Content` channel with `multimedia` 4A role.

   ```
   auto contentChannel = audioManager->openOutputChannel("Content", "multimedia");
   ```

3. When opening an input channel, you need to specify an ALSA device name.

   ```
   auto dialogChannel = audioManager->openInputChannel("Dialog", "hw:2,0");
   ```

## Known Issues

* TuneIn (including streams from many Flash Briefing and Sports Update skills) uses the .ashx playlist URL format for playback. This implementation does not include a parser for ashx audio playlists, so streams from TuneIn may not be able to play.
* Lowering the volume by an amount that is larger than the current volume will result in the volume being set to its maximum.