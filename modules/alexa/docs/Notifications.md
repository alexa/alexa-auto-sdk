# Notifications Interface

It is the responsibility of the platform implementation to provide a visual indication to the user when notifications (for example, package shipment notifications, notifications from skills, etc.) are available from Alexa. See the [AVS Notifications interface documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/notifications.html) for more information about notifications. The Engine uses the registered Notifications implementation to notify you when a notification indicator should be displayed or removed. It does not give any information about the notifications. Audio playback for the notification is handled by whichever audio channel is assigned to the `NOTIFICATION` type.

To implement a custom handler for Notifications extend the `Notifications` class:

```
#include <AACE/Alexa/Notifications.h>
using IndicatorState = aace::alexa::Notifications::IndicatorState;
class MyNotificationsHandler : public aace::alexa::Notifications {

  public:
    void setIndicator( IndicatorState state ) override {
        // set your notifications indicator!
    }

};
...

// Register the platform interface with the Engine
auto m_notificationsHandler = std::make_shared<MyNotificationsHandler>();
engine->registerPlatformInterface(m_notificationsHandler);

```