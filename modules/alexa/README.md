## Alexa API

### Overview

The AAC Alexa API provides the features that are relevant to a platform implementation from the AVS Device SDK. The Engine handles some extra setup and steps to sequence events and directive handling. The platform developer can focus on just using the provided API to interact with AVS. This is done by registering platform interfaces via the Engine object.

### Handling Alexa state changes

The AAC SDK manages internal state information for Alexa and provides an interface for developers to handle state changes in their platform. To implement a custom handler for Alexa state changes, the `aace::alexa::AlexaClient` class should be extended:

    #include <AACE/Alexa/AlexaClient.h>
    class MyAlexaClient : public aace::alexa::AlexaClient {
      public:
        void dialogStateChanged( DialogState state ) override {
            // handle dialog state change!
        }
        void authStateChanged( AuthState state, AuthError error ) override {
            // handle auth state change!
        }
        void connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) override {
            // handle connection status change!
        }
    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyAlexaClient>() );

### Handling speech input

It is the responsibility of the platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to the platform developer. The default platform implementation provides methods for writing raw PCM data to the Engine's audio input processor, which includes handling wakeword recognition and end-of-speech detection.

To implement a custom handler for speech input, the `aace::alexa::SpeechRecognizer` class should be extended:

    #include <AACE/Alexa/SpeechRecognizer.h>
    class MySpeechRecognizer : public aace::alexa::SpeechRecognizer {
      public:
        // possible platform implementation that has an audio input callback
        void startAudioInput(){
          MyAudioInput->start(); //start platform-specific audio input
          ...

        }
        ...

        void MyAudioInputOnWriteCallback( const int16_t* data, const size_t size ) {
            // call inherited write() method to provide audio samples
            // to the Engine for input processing
            write( data, size );  
            ...

        }
    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MySpeechRecognizer>() );

### Handling speech output

The speech synthesizer is responsible for handling Alexa's speech. The `SpeechSynthesizer` class extends the abstract `aace::alexa::AudioChannel` class. This is to separate each audio-capable class on the platform while allowing the nEgine to control their interactive behaviors. The platform implementation provides a reference to an instance of a *media player* and a *speaker*. (The term "speaker" is used here in the sense of an "output device") [**Read more about handling media and volume here**](#handling-media-and-volume).

 To implement a custom handler for speech output the `aace::alexa::SpeechSynthesizer` class should be extended:

      #include <AACE/Alexa/SpeechSynthesizer.h>
      class MySpeechSynthesizer : public aace::alexa::SpeechSynthesizer {
        public:
          MySpeechSynthesizer(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker);
          ...
      }
      ...

      MySpeechSynthesizer::MySpeechSynthesizer(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::SpeechSynthesizer(mediaPlayer, mySpeaker) { }
      ...

      // Configure the Engine
      auto mySpeechSynthesizerMediaPlayer = std::make_shared<MyMediaPlayer>(...);
      auto mySpeechSynthesizerSpeaker = std::make_shared<MySpeaker>(...);
      auto mySpeechSynthesizer = std::make_shared<MySpeechSynthesizer>( mySpeechSynthesizerMediaPlayer, mySpeechSynthesizerSpeaker);
      engine->registerPlatformInterface( mySpeechSynthesizer );

### Handling audio player output

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player and to notify the Engine when the audio starts and stops playing.

The class is similar to the SpeechSynthesizer class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for audio player output the `aace::alexa::AudioPlayer` class should be extended:

    #include <AACE/Alexa/AudioPlayer.h>
    class MyAudioPlayer : public aace::alexa::AudioPlayer {
      public:
        MyAudioPlayer(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker);
          ...

    MyAudioPlayer::MyAudioPlayer(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::AudioPlayer(mediaPlayer, mySpeaker) { }
    ...

    // Configure the Engine
    auto myAudioPlayerMediaPlayer = std::make_shared<MyMediaPlayer>(...);
    auto myAudioPlayerSpeaker = std::make_shared<MySpeaker>(...);
    auto myAudioPlayer = std::make_shared<MyAudioPlayer>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
    engine->registerPlatformInterface( myAudioPlayer );

### Handling notifications

It is the responsibility of the platform implementation to provide a visual and audio indication to the user when notifications (package shipment notifications, notifications from skills, etc.) are available from Alexa. [**Read more about Notifications here**](https://alexa.design/AVSDevNotifications).

To implement a custom handler for Notifications the `aace::alexa::Notifications` class should be extended:

```
#include <AACE/Alexa/Notifications.h>
using IndicatorState = aace::alexa::Notifications::IndicatorState;
class MyNotificationsHandler : public aace::alexa::Notifications {
	 MyNotificationsHandler(std::shared_ptr<aace::alexa::MediaPlayer> player, std::shared_ptr<aace::alexa::Speaker> speaker)
	 
  public:
    void setIndicator( IndicatorState state ) override {
        // set your notifcations indicator!
    }
   
};
...

// Configure the Engine
auto notificationsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
auto notificationsSpeaker = std::make_shared<MySpeaker>(...);
auto m_notificationsHandler = std::make_shared<MyNotificationsHandler>(notificationsMediaPlayer, notificationsSpeaker);
engine->registerPlatformInterface(m_notificationsHandler);

```

### Handling alerts

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player and notify the Engine when the audio starts and stops playing. The state of the alert is also made available for the platform to react to. The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for alerts, the `aace::alexa::Alerts` class should be extended:

    #include <AACE/Alexa/Alerts.h>
    class MyAlerts : public aace::alexa::Alerts {
      public:
        MyAlerts(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker)
    ...

    MyAlerts::MyAlerts(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::Alerts(mediaPlayer, mySpeaker) { }

    void MyAlerts::alertStateChanged( const std::string& alertToken, AlertState state, const std::string& reason ) override {
      //handle the alert state change
    }
    ...

    // Configure the Engine
    auto myAlertsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
    auto myAlertsSpeaker = std::make_shared<MySpeaker>(...);
    auto myAlerts = std::make_shared<MyAlerts>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
    engine->registerPlatformInterface( myAlerts );

### Handling media and volume

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific media player and speaker. It is also the responsibility of the platform implementation to notify the Engine when the speech audio actually begins playing in the media player and when it is finished. Alexa will notify the Engine that the platform should play/stop/pause/resume playing the speech or other audio in which case the platform implementation must play/stop/pause/resume the media player playback. The platform should extend both `MediaPlayer` and `Speaker` and implement the platform-specific functionality. These classes will then be instantiated by other platform interfaces (`Alerts`, `SpeechSynthesizer`, `AudioPlayer`) as separate audio channels.

To implement a custom handler for MediaPlayer the `aace::alexa::MediaPlayer` class should be extended:

    #include <AACE/Alexa/MediaPlayer.h>
    class MyMediaPlayer : public aace::alexa::MediaPlayer{
      public:
        bool prepare() override {
          ...

          while(read(data, size) > 0)
             //write readable data to media location
          ...

          if(successful) return true;
          else playbackError(type, error);// if platform mediaplayer fails, notify the engine with type and description
        }
        bool prepare(std::string url) override {
          MyMediaPlayerReference->MySetMediaUrl(url)// set platform media when url is given
          ...

          return true; //if successful
        }
        bool play() override {
          ...

          MyMediaPlayerReference->MyPlay(); //play platform mediaplayer
          ...

          return true; //if successful
        }
        ...

      PlatformMediaPlayer* MyMediaPlayerReference = ...//platform-specific
    }    

The platform implementation's Speaker class will likely need a reference to the media player class.

To implement a custom handler for Speaker, the `aace::alexa::Speaker` class should be extended:

    #include <AACE/Alexa/Speaker.h>
      class MySpeaker : public aace::alexa::Speaker{
        public:
          MySpeaker(std::shared_ptr<MyMediaPlayer> mediaPlayer){
              m_mediaPlayer = mediaPlayer;
          }; //One way to get the media player reference
          ...

          bool setVolume( int8_t volume ) override {
            m_mediaPlayer->MySetVolume(volume);//platform-specific.
            m_myPlatformVolume = volume;
            return true;
            ...

          int8_t getVolume() override {
            return m_myPlatformVolume;//using a member variable
          }
          ...
    }

### Handling GUI templates (display cards)

When template info is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the info that is received from Alexa. There are two template types: [Templates](https://alexa.design/DevDocRenderTemplate) and [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo).

In the case of lists, it is the responsibility of the platform to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of reading the first five elements it will prompt the user to see if they want to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

To implement a custom handler for GUI templates, the `aace::alexa::TemplateRuntime` class should be extended:

    #include <AACE/Alexa/TemplateRuntime.h>
    class MyTemplateRuntime : public aace::alexa::TemplateRuntime {
      public:
        void doRenderTemplate( const std::string& payload ) override {
            // handle rendering the template data specified in payload
        }
        void doRenderPlayerInfo( const std::string& payload ) override {
            // handle rendering the player info data specified in payload
        }
    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyTemplateRuntime>() );

### Handling playback controller events

The Engine provides methods for notifying it of playback controller events. If the platform has playback control features, it must inform the Engine.

To implement a custom handler for the playback controller, the `aace::alexa::PlaybackController` class should be extended:

    #include <AACE/Alexa/PlaybackController.h>
    class MyPlaybackController : public aace::alexa::PlaybackController {
      ...

      void platformPlayButtonPressed(){ //called by some platform event
          playButtonPressed();
          ...

    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyPlaybackController>() );

