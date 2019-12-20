# Alexa Auto SDK Migration Guide

This guide outlines the changes you will need to make to migrate from Auto SDK v2.0 to Auto SDK v2.1. 

**Table of Contents**

* [Build Changes](#build-changes)
* [Engine Configuration File Updates](#engine-configuration-file-updates)
* [Navigation Enhancements](#navigation-enhancements)
* [Car Control Source File Relocation](#car-control-source-file-relocation)
* [Code-Based Linking (CBL) Handler in Sample Apps](#code-based-linking-cbl-handler-in-the-sample-apps)

## Build Changes <a id = "build-changes"></a>
The following build changes have been introduced in Auto SDK v2.1:

* The builder script usage has changed for Linux targets. All Linux targets now use the same platform name (`linux`), and `-t <target>` is mandatory. For example, to build for a Linux native target, use:

    `builder/build.sh linux -t native`
    
    to build for Linux native, pokyarm, and pokyarm64 targets, use:
    
    `builder/build.sh linux -t native,pokyarm,pokyarm64`

    See the [Builder README](./builder/README.md#running-builder) for details about supported platforms and targets.

* For QNX targets, you must cross-compile with the QNX multimedia software for the system audio extension (which is built by default for QNX targets). This requires a QNX Multimedia Suite license. See the [System Audio extension README](./extensions/experimental/system-audio/README.md) for details.

## Engine Configuration File Updates <a id = "engine-configuration-file-updates"></a>

The AVS Device SDK portion of the Auto SDK Engine configuration (the `aace.alexa.avsDeviceSDK` node) has been updated. See the [`config.json.in`](./samples/cpp/assets/config.json.in) file for details.

  * The `"deviceInfo"` node includes two new elements: `"manufacturerName"` and `"description"`.
  * A path to the capabilities database is now required. Use the `"capabilitiesDelegate"` element to specify this path.
  * The `"settings"` element has changed to `"deviceSettings"`, and it includes these changes:
    * The default locale setting has been moved from `"defaultAVSClientSettings/locale"` to `"defaultLocale"`.
    * `"deviceSettings"` now requires a `"defaultTimezone"`.


## Navigation Enhancements <a id = "navigation-enhancements"></a>

Auto SDK v2.1 introduces additional navigation features that you can integrate in your application to enrich the user's experience: add/cancel a waypoint, show/navigate to a previous destination, turn and lane guidance, and map display control. Implementing these enhancements required deprecating the `setDestination()` interface in favor of the `startNavigation()` interface and adding several additional interfaces.

To migrate from Auto SDK v2.0 to Auto SDK v2.1, you must update your platform implementation to use the `startNavigation()` method instead of the `setDestination()` method, modify the payload for the `getNavigationState()` method, and implement the new navigation methods. This guide takes you through these steps. Please see the Navigation module README for [C++](./modules/navigation/README.md) or [Android](./platforms/android/modules/navigation/README.md) for additional information and resources.

### What's New

The following abstract methods have been added to the Navigation platform interface:

* `startNavigation()`
* `showPreviousWaypoints()`
* `navigateToPreviousWaypoint()`
* `showAlternativeRoutes()`
* `controlDisplay()`
* `announceManeuver()`
* `announceRoadRegulation()`

The following methods have been added as well:

* `navigationEvent()`
* `navigationError()`
* `showAlternativeRoutesSucceeded()`

The following method has been removed from the Navigation module:

* `setDestination()`

The following method now returns a different payload:

* `getNavigationState()`

### Implementing the New Navigation Features

To implement the new navigation features, follow these steps:

**STEP 1:** Replace the `setDestination()` method with the `startNavigation()` method.

The payload meaning of `startNavigation()` is different than that of deprecated `setDestination()`. `setDestination()` corresponded to adding a destination to the navigation system context. `startNavigation()`, on the other hand, corresponds to using the route information provided in the payload to start navigation, with one or more waypoints. In response to `startNavigation()`, your implementation should also call either the `navigationEvent()` method or the `navigationError()` method.

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
// NavigationHandler.java
@Override
*public void *startNavigation( String payload ) {
    ...
    // success
    navigationEvent( EventName.NAVIGATION_STARTED );
    ...
    // failure
    navigationError( ErrorType.NAVIGATION_START_FAILED, ErrorCode.INTERNAL_SERVICE_ERROR, "" );
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
// NavigationHandler.cpp
void NavigationHandler::startNavigation(const std::string& payload ) {
    ...
    // success
        navigationEvent( aace::navigation::NavigationEngineInterface::EventName::NAVIGATION_STARTED );
    ...
    // failure
        navigationError( aace::navigation::NavigationEngineInterface::ErrorType::NAVIGATION_START_FAILED, aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR, "" );    
```
</p>
</details>
    
**STEP 2:** Modify the payload for the `getNavigationState()` method.

The functionality of the `getNavigationState()` and `cancelNavigationState()` methods is unchanged from Auto SDK v2.0, but the `getNavigationState()` payload has changed. The NavigationState context has been updated to contain more information than in Auto SDK v2.0.

The `address` field has been updated from a string to the following object:

```
...
"address": {
       "addressLine1": "{{STRING}}", //Address line 1
       "addressLine2": "{{STRING}}", //Address line 2
       "addressLine3": "{{STRING}}", //Address line 3
       "city": "{{STRING}}", //City
       "districtOrCounty": "{{STRING}}", //district or county
       "stateOrRegion": "{{STRING}}", //state or region
       "countryCode": "{{STRING}}", //3 letter country code
       "postalCode": "{{STRING}}", //postal code
},
...
```
The `name` field has been added to the waypoint payload:

```
"name": "{{STRING}}", // name of the waypoint such as home or Starbucks
```

<details><summary>Here is an <strong>example of a full NavigationState context payload</strong> - click to expand or collapse</summary>
<p>

```
...,
{
  "header": {
    "namespace": "Navigation",
    "name": "NavigationState"
  },
  "payload": {
    "state": "{{STRING}}", //NAVIGATING or NOT_NAVIGATING
    "waypoints": [
        {
            "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM  
            "estimatedTimeOfArrival": {
                "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
                "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
             },
            "address": {
                   ++ "addressLine1": "{{STRING}}", //Address line 1
                   ++ "addressLine2": "{{STRING}}", //Address line 2
                   ++ "addressLine3": "{{STRING}}", //Address line 3
                   ++ "city": "{{STRING}}", //City
                   ++ "districtOrCounty": "{{STRING}}", //district or county
                   ++ "stateOrRegion": "{{STRING}}", //state or region
                   ++ "countryCode": "{{STRING}}", //3 letter country code
                   ++ "postalCode": "{{STRING}}", //postal code
             },
            ++ "name": "{{STRING}}", // name of the waypoint such as home or Starbucks
             "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
        },
        {
            "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM
            "estimatedTimeOfArrival": {
                "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
                "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
             },
            "address": {
                  ++  "addressLine1": "{{STRING}}", //Address line 1
                  ++  "addressLine2": "{{STRING}}", //Address line 2
                  ++  "addressLine3": "{{STRING}}", //Address line 3
                  ++  "city": "{{STRING}}", //city
                  ++  "districtOrCounty": "{{STRING}}", //district or county
                  ++  "stateOrRegion": "{{STRING}}", // state or region
                  ++  "countryCode": "{{STRING}}", //3 letter country code
                  ++  "postalCode": "{{STRING}}", // postal code
             },
           ++ "name": "{{STRING}}", // name of the waypoint such as home or Starbucks
            "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
            "pointOfInterest": {
                "id": "{{STRING}}", //POI lookup Id vended from Alexa 
                "hoursOfOperation": [
                 {
                     "dayOfWeek": "{{STRING}}",
                     "hours": [
                      {
                         "open": "{{STRING}}", // ISO-8601 time with timezone format
                         "close": "{{STRING}}" // ISO-8601 time with timezone format
                      }
                    ],
                    "type": "{{STRING}}" // Can be: OPEN_DURING_HOURS, OPEN_24_HOURS, etc.
                }
                ],
                "phoneNumber": "{{STRING}}"
             }
             
        },
        ...
      ],
     "shapes": [
            [
              {{LATITUDE_DOUBLE}},
              {{LONGITUDE_DOUBLE}}
            ],
            [
              {{LATITUDE_DOUBLE}},
              {{LONGITUDE_DOUBLE}}
            ],
        ...      
     ]   
  }
}
...,
```
</p>
</details>

**STEP 3:** Implement the new navigation abstract methods.

The new navigation methods are all called in response to navigation-based user utterances such as “show my previous route” or “what’s the speed limit here?”. At a minimum, your implementation should report a `navigationError()` to inform the user when the navigation system does not support that information.

>**Note:** The `navigationEvent()`, `showAlternativeRoutesSucceeded()` and `navigationError()` methods have been implemented in the Auto SDK but are not yet implemented on the cloud side. Sending the events will not affect navigation functionality, but the Alexa cloud will return an `INVALID_REQUEST_EXCEPTION` or `INVALID_SERVICE_EXCEPTION` until these events are implemented on the cloud side.  

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
@Override
public void showPreviousWaypoints() {
    //handle showing information about previous waypoints...
    navigationError( ErrorType.SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode.NOT_SUPPORTED, *""** *);
}
...
    
@Override
public void navigateToPreviousWaypoint() {
    //handle navigation to previous waypoint
    navigationError( ErrorType.PREVIOUS_NAVIGATION_START_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void showAlternativeRoutes( AlternateRouteType alternateRouteType ) {
    //pass AlternateRouteType enum
    navigationError( ErrorType.DEFAULT_ALTERNATE_ROUTES_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void controlDisplay ( ControlDisplay controlDisplay ) {
    //pass ControlDisplay enum
    navigationError( ErrorType.ROUTE_OVERVIEW_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void announceManeuver( String payload  ) {
    //pass the JSON string payload from AnnounceManeuver directive
    navigationError( ErrorType.TURN_GUIDANCE_FAILED, ErrorCode.NOT_SUPPORTED, "" );
}
...
    
@Override
public void announceRoadRegulation( RoadRegulation roadRegulation ) {
    //pass RoadRegulation enum
    navigationError( ErrorType.SPEED_LIMIT_REGULATION_FAILED, ErrorCode.NOT_SUPPORTED, "" );
} 
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
void NavigationHandler::showPreviousWaypoints() {
        //handle showing information about previous waypoints...
        navigationError( aace::navigation::Navigation::ErrorType.SHOW_PREVIOUS_WAYPOINTS_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, ""* *);
   }
    ...
    
    void NavigationHandler::navigateToPreviousWaypoint() {
        //handle navigation to previous waypoint
        navigationError( aace::navigation::Navigation::ErrorType.PREVIOUS_NAVIGATION_START_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::showAlternativeRoutes( aace::navigation::Navigation::AlternateRouteType alternateRouteType ) {
        //pass AlternateRouteType enum
        navigationError( aace::navigation::Navigation::ErrorType.DEFAULT_ALTERNATE_ROUTES_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::controlDisplay ( aace::navigation::Navigation::ControlDisplay controlDisplay ) {
        //pass ControlDisplay enum
        navigationError( aace::navigation::Navigation::ErrorType.ROUTE_OVERVIEW_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::announceManeuver( String payload  ) {
        //pass the JSON string payload from AnnounceManeuver directive
        navigationError( aace::navigation::Navigation::ErrorType.TURN_GUIDANCE_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    }
    ...
    
    void NavigationHandler::announceRoadRegulation( aace::navigation::Navigation::RoadRegulation roadRegulation ) {
        //pass RoadRegulation enum
        navigationError( aace::navigation::Navigation::ErrorType.SPEED_LIMIT_REGULATION_FAILED, 
            aace::navigation::Navigation::ErrorCode.NOT_SUPPORTED, "" );
    } 
```
</p>
</details>

### New TemplateRuntime Interface Version

The Auto SDK now implements version 1.2 of the TemplateRuntime interface to handle display card templates. If you support TemplateRuntime in your implementation, you must update your implementation to support the new card types.

The TemplateRuntime interface remains the same, but the `LocalSearchListTemplate1` template has been deprecated in favor of the new `LocalSearchListTemplate2` template. In addition, two new templates (`TrafficDetailsTemplate` and `LocalSearchDetailTemplate1`), are now supported. The `TrafficDetailsTemplate` includes commute information to favorite destinations such as home or work. The `LocalSearchDetailTemplate1` template includes information about specific locations or information in response to users asking for details about locations presented in the `LocalSearchListTemplate2` template. For details about the TemplateRuntime interface, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/templateruntime.html). For details about implementing TemplateRuntime in your Auto SDK implementation see the Alexa module README for [C++](./modules/alexa/README.md#handling-display-card-templates) or [Android](./platforms/android/modules/alexa/README.md#handling-display-card-templates).


## Car Control Source File Relocation <a id = "car-control-source-file-relocation"></a>

The Car Control module platform interface files and documentation are now located in `aac-sdk/modules/car-control` for C++ and `aac-sdk/platforms/android/modules/car-control` for Android, rather than in the Local Voice Control (LVC) extension directory structure.

## Code-Based-Linking (CBL) Handler in the Sample Apps <a id = "code-based-linking-cbl-handler-in-the-sample-apps"></a>
Both of the Auto SDK Sample Apps now include the Code-Based Linking (CBL) handler implementation (in favor of the `AuthProvider` handler implementation ) to handle obtaining access tokens from Login with Amazon (LWA). Changing from the `AuthProvider` handler to the CBL handler is *not a required change*, but we recommend that you use the Auto SDK CBL interface for ease of implementation. For details about the CBL handler, please see the CBL module README [for C++](./modules/cbl/README.md) or [for Android](./platforms/android/modules/cbl/README.md).

If you want to continue using the `AuthProvider` interface, we recommend that you implement the new `onAuthFailure()` method that exposes 403 "unauthorized request" exceptions from Alexa Voice Service (AVS). This method may be invoked, for example, when your product makes a request to AVS using an access token obtained for a device which has been deregistered from the Alexa companion app. In the Sample Apps, you can override the  interface and unset your login credentials as if the user had done so with your GUI interface: 

<details><summary><strong>Java (Android)</strong> - click to expand or collapse</summary>
<p>

```
@Override
    public void authFailure( String token ) {
        // handle user de-authorize scenario 
```
</p>
</details>

<details><summary><strong>C++</strong> - click to expand or collapse</summary>
<p>

```
void AuthProviderHandler::authFailure( const std::string& token ) {
        // handle user de-authorize scenario ```
</p>
</details>