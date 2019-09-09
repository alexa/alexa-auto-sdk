# Navigation Module


The Alexa Auto SDK Navigation module provides the features required by a platform implementation to interface with the navigation capabilities of Alexa.

**Table of Contents**

* [Handling Navigation Requests](#handling-navigation-requests)
* [Implementing a Navigation Handler](#implementing-navigation-handler)


## Handling Navigation Requests<a id = "handling-navigation-requests"></a>

It is the responsibility of the platform implementation to set a destination and stop navigation when notified to do so by the Engine. The platform implementation should choose how to handle these requests based on its navigation provider. The platform can also choose to enable some additional voice features by providing the current navigation state in the `getNavigationState` callback. 

## Implementing a Navigation Handler<a id = "implementing-navigation-handler"></a>

To implement a custom navigation handler to handle navigation requests from Alexa, extend the `Navigation` class:

```
#include <AACE/Navigation/Navigation.h>
class MyNavigation : public aace::navigation::Navigation {

    bool setDestination( const std::string &  payload ) override {
        // handle setting destination request from Alexa
    }
    ...

    bool cancelNavigation() override {
        // handle cancel navigation request from Alexa
    }
    ...
    
    std::string getNavigationState() override() {
        // return Navigation State JSON payload string  
    }
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyNavigation>());
```

Following is an example `setDestination` JSON string payload:

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
### NavigationState

Providing the full Navigation state is optional, but will enable the customer to access additional features including :

- Navigate to favorite locations
- Ask for Estimated Time of Arrival (ETA)
- Ask for Distance To Arrival (DTA)

To disable navigation state, the platform can simply return an empty string from the `getNavigationState()` callback. However, this will also disable the ability to cancel navigation using Alexa. 

If you are unable to provide full navigation state information, but would still like to provide the ability for the user to cancel navigation, instead a minimal payload can be passed to inform the engine that the client is currently navigating. 

Example minimal navigation state payload to send from the `getNavigationState()` callback:

```
{
  "state": "NAVIGATING",
  "waypoints": [],
  "shapes": []
}    
``` 


To enable the full navigation state capabilities, the platform must pass a JSON string payload as formatted in the example below. The accepted value for `state` is a string with value `NAVIGATING` or `NOT_NAVIGATING`. The `waypoints` field is an array of waypoint objects. The accepted values for the waypoint `type` is a string called `SOURCE`, `DESTINATION` or `INTERIM`. The time fields in the object should be in ISO 8601 UTC format. The `shapes` field is an array of route shape coordinates. Only the first 100 will be sent to AVS. 

Example navigation state payload to send from the `getNavigationState()` callback:
:

```
{
  "state": "NAVIGATING",
  "waypoints": [
    {
      "type": "SOURCE",
      "estimatedTimeOfArrival": {
        "predicted": "2019-05-20T16:39:57-08:00"
      },
      "address": "2795 Augustine drive, santa clara, CA 95054",
      "coordinate": [37.380946, -121.9794846]
    },
    {
      "type": "DESTINATION",
      "estimatedTimeOfArrival": {
        "ideal": "2019-05-30T17:00:00-08:00",
        "predicted": "2019-05-30T17:10:00-08:00"
      },
      "address": "750 Castro St, Mountain View, CA 94041",
      "coordinate": [37.380946, -121.9794846],
      "pointOfInterest": {
        "id": "POI Lookup Id",
        "name": "Starbucks",
        "phoneNumber": "+12064968523"
      }
    }
  ],
  "shapes": [
    [
      37.380946,
      -121.9794846
    ],
    [
      37.380946,
      -121.9794846
    ],
    ...
	]
}    
```    