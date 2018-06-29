## Phone Call Controller API

### Overview

The AAC Phone Call Controller API provides the features required by a platform implementation to interface with the phone call controller capabilities of Alexa.

### Handling Phone Call Controller

The Engine provides callbacks for handling phone call controller directives from Alexa. The platform implementation must also inform the Engine of the status of phone call controller on the platform. The platform implementation is responsible for managing the lifecycle and user experience of a call session and connection to a calling device.

To implement a custom phone call controller handler for handling phone call controller directives and events from Alexa, the `aace::phoneCallController::PhoneCallController` class should be extended:

    #include <AACE/PhoneCallController/PhoneCallController.h>


    class MyPhoneCallController : public aace::phoneCallController::PhoneCallController {

      bool dial( const std::string& payload ) override {
        // handle the call from Alexa to initiate an outgoing call to a destination address
      }
      ...etc...

        // on a connection state change of the platform calling device
        ConnectionState state = ...  // CONNECTED, DISCONNECTED
        connectionStateChanged( state );
        ...

        // on receiving dial call or initiating a call outside of the scope of Alexa
        const std::string& callId = ... // unique identifier given from dial, or UUID generated from the platform for calls outside of Alexa
        callActivated( callId );
        ...

        // on an active call ended or an ongoing phone call setup cancelled
        const std::string& callId = ... // unique identifier for call session
        callTerminated( callId )  
        ...

        // on error handling for initiating or maintaining a call on the platform calling device
        const std::string& callId = ... // unique identifier for call session
        const std::string& error = ... // platform error code, ex. 4xx, 500, 503
        const std::string& message = ... // platform error message
        callFailed( callId, error, message );
        ...

    };

    //engine config
    engine->registerPlatformInterface( std::make_shared<MyPhoneCallController>());

### Whitelisting

Please see the [Need Help?](../../NEED_HELP.md) page for how to whitelist your device for using Phone Call Control APIs.
