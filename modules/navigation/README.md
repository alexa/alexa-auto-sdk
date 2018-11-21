## Navigation API

### Overview

The Alexa Auto SDK Navigation API provides the features required by a platform implementation to interface with the navigation capabilities of Alexa.

### Handling Navigation

It is the responsibility of the platform implementation to set a destination and stop navigation when notified to do so by the Engine. The platform implementation should choose how to handle these requests based on its navigation provider.

To implement a custom navigation handler for handling navigation requests from Alexa, the `aace::navigation::Navigation` class should be extended:

```
    #include <AACE/Navigation/Navigation.h>


    class MyNavigation : public aace::navigation::Navigation {

      bool setDestination( const std::string &  payload ) override {
        // handle setting destination request from Alexa
      }
      ...etc...

      bool cancelNavigation() override {
        // handle cancel navigation request from Alexa
      }
      ..etc...
    };

    //engine config
    engine->registerPlatformInterface( std::make_shared<MyNavigation>());
```

Example setDestination JSON string payload:

```
{
  "destination": {
    "coordinate": {
      "latitudeInDegrees": XXX.XXXXXXX,
      "longitudeInDegrees": -XXX.XXXXXXX
    },
    "singleLineDisplayAddress": "Number StreetName City State Zipcode",
    "multipleLineDisplayAddress": "Number StreetName \n City State Zipcode",
    "name": "Place Name"
  },
  "transportationMode": "DRIVING",
  "metadata": {
  "hoursOfOperation": [
    {
      "dayOfWeek": "MONDAY",
      "hours": [{
        "open": "12:30:00-8:00"
        "close": "8:00:00-12:30"
      }],
      "status": "OPEN_DURING_HOURS"
    },
    {
      "dayOfWeek": "TUESDAY",
      "hours": [{
        "open": "12:30:00-8:00"
        "close": "8:00:00-12:30"
      }],
      "status": "OPEN_DURING_HOURS"
    },
    {
      "dayOfWeek": "WEDNESDAY",
      "hours": [{
        "open": "12:30:00-8:00"
        "close": "8:00:00-12:30"
      }],
      "status": "OPEN_DURING_HOURS"
    },
    {
      "dayOfWeek": "THURSDAY",
      "hours": [{
        "open": "12:30:00-8:00"
        "close": "8:00:00-12:30"
      }],
      "status": "OPEN_DURING_HOURS"
    },
    {
      "dayOfWeek": "FRIDAY",
      "hours": [{
        "open": "12:30:00-8:00"
        "close": "8:00:00-12:30"
      }],
      "status": "OPEN_DURING_HOURS"
    },
    {
      "dayOfWeek": "SATURDAY",
      "status": "CLOSED"
    },
    {
      "dayOfWeek": "SUNDAY",
      "status": "CLOSED"
    },
  ],
  "phoneNumber": "+12221113333"
  }
}

```
