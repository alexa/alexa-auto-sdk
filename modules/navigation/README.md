## Navigation API

### Overview

The Navigation module includes a navigation platform interface to handle Alexa navigation events and directives. This means it depends on the [Alexa module](../alexa/README.md).

### Handling Navigation

The Engine provides callbacks for handling navigation directives from Alexa. The platform implementation must also inform the Engine of the status of navigation on the platform. This is optional and dependent on the platform implementation.

To implement a custom navigation handler for handling navigation events from Alexa, the `aace::navigation::Navigation` class should be extended:

    #include <AACE/Navigation/Navigation.h>


    class MyNavigation : public aace::navigation::Navigation {

      bool setDestination( const std::string &  payload ) override {
        // handle the call from Alexa
      }
      ...etc...

      // optional for future use
      void platformNavigationStarted(...){
        navigationStarted();// (future) tell the Engine that navigation has begun
      }
      ..etc...
    };

    //engine config
    engine->registerPlatformInterface( std::make_shared<MyNavigation>());

### Whitelisting

Please see the [Need Help?](../../NEED_HELP.md) page for how to whitelist your device for using Navigation APIs.
