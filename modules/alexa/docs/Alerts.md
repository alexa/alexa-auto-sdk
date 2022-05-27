# Alerts Interface

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player. See the [AVS Alerts interface documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alerts.html) for more information about alerts. The state of the alert is also made available for the platform to react to. The playback is handled by whichever audio channel is assigned to the `ALERT` type.

To implement a custom handler for alerts, extend the `Alerts` class:

```
#include <AACE/Alexa/Alerts.h>
class MyAlerts : public aace::alexa::Alerts {
    public:
        void MyAlerts::alertStateChanged( const std::string& alertToken, AlertState state, const std::string& reason ) override {
        //handle the alert state change
        }

        void MyAlerts::alertCreated( const std::string& alertToken, const std::string& detailedInfo ) override {
        //handle the alert detailed info when alert is created (optional)
            /*
    		  * JSON string detailedInfo :
    		  * {	
    		  *	 "time" : <String>
    		  *	 "type" : <String>
    		  *	 "label" : <String>	
    		  * }
    		  */
        }
        
        void MyAlerts::alertDeleted( const std::string& alertToken ) override {
            //handle the alert when alert is deleted (optional)
        }
};
...

// Register the platform interface with the Engine
auto myAlertsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
auto myAlertsSpeaker = std::make_shared<MySpeaker>(...);
auto myAlerts = std::make_shared<MyAlerts>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
engine->registerPlatformInterface( myAlerts );
```