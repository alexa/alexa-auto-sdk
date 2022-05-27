# DoNotDisturb Interface

The DoNotDisturb (DND) interface allows users to block all incoming notifications, announcements, and calls to their devices, and to set daily recurring schedules that turn DND off and on.  For details, see the [DND Interface documentation](https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html). The Engine uses the registered DND implementation to notify the client when DND has been set or unset. A user's voice request to change the DND state triggers audio playback, but no audio playback occurs when a user sets the DND state using the touch screen.

To implement a custom handler for DND extend the `DoNotDisturb` class:

```
#include <AACE/Alexa/DoNotDisturb>

class MyDoNotDisturbHandler : public aace::alexa::DoNotDisturb {

  public:
    void setDoNotDisturb( bool doNotDisturb ) override {
        // set your DoNotDisturb indicator
    }
    // on user GUI setting change
    ...
        bool doNotDisturb = userSetState;
        doNotDisturbChanged(doNotDisturb);
    ...

};
...

// Register the platform interface with the Engine
auto m_doNotDisturbHandler = std::make_shared<MyDoNotDisturbHandler>();
engine->registerPlatformInterface(m_doNotDisturbHandler);

```