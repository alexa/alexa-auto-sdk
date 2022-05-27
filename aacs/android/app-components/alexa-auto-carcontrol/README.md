# AACS Car Control <!-- omit in toc -->

The AACS Car Control library is an Android library for the AACS Core Service to run car control commands in cars that are based on Android Automotive OS. A car control command is run by the AACS Core Service each time the user tries to voice-control a vehicle component.

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Understanding Library Components](#understanding-library-components)
- [Building the AACS Car Control Library](#building-the-aacs-car-control-library)
- [Before Using the AACS Car Control Library](#before-using-the-aacs-car-control-library)
    - [Including AACS Car Control Library in a System Application](#including-aacs-car-control-library-in-a-system-application)
    - [Providing Permission in Android Manifest](#providing-permission-in-android-manifest)
    - [Ensuring Intent Target Specified in the Library is Used](#ensuring-intent-target-specified-in-the-library-is-used)
- [Sequence Diagrams](#sequence-diagrams)
- [How the AACS Car Control Library Works](#how-the-aacs-car-control-library-works)
  
## Overview
Using the AACS Car Control Library, AACS allows user utterances to be directly applied to Android Automotive OS car control APIs and then to the Hardware Abstraction Layer to complete the car control workflow. The library translates  between Auto SDK Car Control events and Android Automotive `CarPropertyManager` API calls, which controls various car features, such as air conditioning and fan settings. For information about `CarPropertyManager`, see the [Android documentation on CarPropertyManager](https://developer.android.com/reference/android/car/hardware/property/CarPropertyManager).

The library works with Android [Car API](https://developer.android.com/reference/android/car/Car) and [CarPropertyManager API](https://developer.android.com/reference/android/car/hardware/property/CarPropertyManager). These APIs are only available at Android API level 29 and up.

The library is an optional module. You can build it into an Android archive (AAR) to be included in your application.

## Understanding Library Components

The following list describes the purposes of the major components of the library: 

* The AACS Car Control Broadcast Receiver:
    * Receiving AASB `AdjustControllerValue` or `SetControllerValue` messages from the AACS Core Service.
    * Instantiating the Car Control Handler to call specific controller operations. The exact operations supported depend on the controller type, which can be Power, Toggle, Mode, or Range.

* The AACS Car Control platform implementation (`CarControlHandler`):
    * Instantiating the Android Car object to be called in the set and adjust methods for each controller type.
    * Defining the get and set methods for each controller type.
    * Defining the adjust methods for the Range or Power Controller.

* The AACS Car Control Helper/Util:
    * Providing translation between [endpointID, controllerType, controllerID, value] in the AASB Car Control message from the Auto SDK Engine to [propertyId, areaId, value] used in the Android Automotive API call.
    * Getting or saving the current Mode setting for the Mode Controller.
    * Enabling you to parse an external configuration file if you want to use a customized `CarControlEndpointMapping.json` file.

* Car Control Endpoint Mapping configuration file maps [endpointID, controllerType, controllerID, value] from the Auto SDK Car Control Asset to [propertyId, areaId, value] used in the Android Automotive API call. 

    A default `CarControlEndpointMapping.json` file is provided in the assets directory. Be sure to review `CarControlEndpointMapping.json` to verify that it contains values consistent with the ones specified in the [CarControlConfig.json file in the Car Control module](https://github.com/alexa/alexa-auto-sdk/blob/4.0/modules/car-control/assets/CarControlConfig.json). For example, if you have changed an `endpointId` in `CarControlConfig.json` from `"default.light"` to `"default.roof.light"`, the `CarControlEndpointMapping.json` file must contain the same endpoint mapping information.

## Building the AACS Car Control Library
You can build the library locally using the following steps:

  1) Enter the following command to change the directory:
  ~~~
      cd ${AAC_SDK_HOME}/aacs/android/sample-app
  ~~~
  2) Enter the following command to build the Car Control library:
  ~~~
      ./gradlew :alexa-auto-carcontrol:assembleRelease
  ~~~
  Replace `assembleRelease` with `assembleDebug` if you want to build the debug version of the library. The generated AAR is available at `alexa-auto-sdk/aacs/android/app-components/alexa-auto-carcontrol/aacscarcontrol/build/outputs/aar`.
  You must include the `AACSIPC`, `AACSConstants`, `AACSCommonUtils`, `AACS` and `Auto SDK` AARs in your application to use with the AACS Car Control AAR.

To enable car control support in the AACS Sample App, follow these steps:

1) Enter the following command to change the directory:
~~~
    cd ${AAC_SDK_HOME}/aacs/android/sample-app
~~~   
2) Enter the following command to start the local build with car control enabled.
~~~
    ./gradlew assembleLocalRelease -PenabledCarControl
~~~
For more build options, see the [AACS Sample App README](../../sample-app/README.md#optional-arguments).

## Before Using the AACS Car Control Library
Before using the library, follow these major steps:

1. Install your application with the AACS and Car Control AARs as a system privileged app on Android Automotive OS.
2. Provide permission in your app's Android Manifest.
3. Ensure that the intent target specified in the library is used.

### Including AACS Car Control Library in a System Application
For AACS to enable the permission namespace `android.car.permission`, it must run in a system privileged app. To install your application as a system privileged app, place it in the `/system/priv-app/` directory. 

### Providing Permission in Android Manifest
For security reasons, for your application to send intents to or receive intents from the AACS Car Control Library, follow these steps:

1) In `privapp-permissions-com.amazon.alexaautoclientservice.xml`, specify `android.car.permission`. The following example file shows how to specify permissions for using intents for various car control operations.

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <permissions>
	    <privapp-permissions package="com.amazon.alexaautoclientservice">
        <permission name="android.car.permission.CONTROL_CAR_EXTERIOR_LIGHTS" />
        <permission name="android.car.permission.CONTROL_CAR_DOORS" />
        <permission name="android.car.permission.CONTROL_CAR_CLIMATE" />
        <permission name="android.car.permission.CONTROL_CAR_SEATS" />
        <permission name="android.car.permission.CAR_EXTERIOR_ENVIRONMENT" />
        <permission name="android.car.permission.CAR_ENERGY_PORTS" />
        <permission name="android.car.permission.CONTROL_CAR_MIRRORS" />
        <permission name="android.car.permission.READ_CAR_DISPLAY_UNITS" />
        <permission name="android.car.permission.CONTROL_CAR_WINDOWS" />
        <permission name="android.car.permission.CAR_CONTROL_AUDIO_VOLUME" />
        <permission name="android.car.permission.CAR_DISPLAY_IN_CLUSTER" />
        <permission name="android.car.permission.CAR_INSTRUMENT_CLUSTER_CONTROL" />
        <permission name="android.car.permission.CAR_EXTERIOR_LIGHTS" />
        <permission name="android.car.permission.CONTROL_CAR_INTERIOR_LIGHTS" />
        <permission name="android.car.permission.READ_CAR_INTERIOR_LIGHTS" />
        <permission name="android.car.permission.CAR_DYNAMICS_STATE" />
        <permission name="android.car.permission.CONTROL_CAR_DISPLAY_UNITS" />
        <permission name="android.permission.INTERNET" />
        <permission name="android.permission.RECORD_AUDIO" />
        <permission name="android.permission.ACCESS_FINE_LOCATION" />
        <permission name="android.permission.ACCESS_NETWORK_STATE" />
        <permission name="android.permission.ACCESS_WIFI_STATE" />
        <permission name="android.permission.RECEIVE_BOOT_COMPLETED" />
        <permission name="android.permission.CONTROL_INCALL_EXPERIENCE"/>   
        <permission name="android.permission.CAPTURE_AUDIO_OUTPUT"/>
        <permission name="android.permission.INTERACT_ACROSS_USERS"/>
        </privapp-permissions>
    </permissions>
```

2) Include `privapp-permissions-com.amazon.alexaautoclientservice.xml` in the `/etc/permissions/` directory. 

### Ensuring Intent Target Specified in the Library is Used
The AACS Car Control Broadcast Receiver listens to intents from the AACS Core Service with the `CarControl` topic. The intent filter in the AACS Car Control Library already defines the intent target for `CarControl`. For the intent filter in the library to take effect, be sure to clear the intent target defined for `CarControl` in the AACS configuration as follows. Otherwise, the target specification in the AACS configuration overrides the intent filter in the library.

```json
    "CarControl" : {
        "type": [],
        "package": [],   
        "class": []
    }
``` 

## Sequence Diagrams
The following diagram illustrates the flow when an utterance asks Alexa to set fan speed to 3.
![Set Fan Speed](./docs/diagrams/set-fan-speed-to-3.png)

The following diagram illustrates the flow after the set value is finished at the hardware layer.
![Set Reply](./docs/diagrams/set-reply-to-engine.png)

## How the AACS Car Control Library Works
When the user issues an utterance, the Engine receives a car control event from Alexa, which the Engine passes to AACS through an AASB message.

The AASB message received by AACS has the following attributes: 

* Action is `com.amazon.aacs.aasb.AdjustControllerValue` or `com.amazon.aacs.aasb.SetControllerValue`. 
* Category is `com.amazon.aacs.aasb.CarControl`.
* Extras is `payload`.
  
    The `payload` object includes detailed information about the action, which is  specified in the `messageDescription` field of the AASB message. The following list describes the `payload` for each action:
    
    * For `SetControllerValue`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "POWER",
          "endpointId" : "{{String}}",
          "turnOn" : {{Boolean}}
      }
      ```
    * For `AdjustControllerValue`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "TOGGLE",
          "endpointId" : "{{String}}",
          "controllerId" : "{{String}}",
          "turnOn" : {{Boolean}}
      }
      ```
    * For `SetModeController`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "MODE",
          "endpointId" : "{{String}}",
          "controllerId" : "{{String}}",
          "value" : "{{String}}"
      }
      ```
    * For `SetRangeController`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "RANGE",
          "endpointId" : "{{String}}",
          "controllerId" : "{{String}}",
          "value" : {{Double}}
      }
      ```
    * For `AdjustModeController`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "MODE",
          "endpointId" : "{{String}}",
          "controllerId" : "{{String}}",
          "delta" : {{Integer}}
      }
      ```
    * For `AdjustRangeController`, the payload has the following schema:
      ```
      "payload" : {
          "controllerType" : "RANGE",
          "endpointId" : "{{String}}",
          "controllerId" : "{{String}}",
          "delta" : {{Double}}
      }
      ```

After receiving the intent, the AACS Car Control Broadcast Receiver parses the payload and calls for the Car Control Handler to perform specific car control operations.
