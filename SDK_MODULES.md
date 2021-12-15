# Auto SDK Modules and Extensions
The Auto SDK is organized into logically related groups of functionality called “modules,” which enable you to select only the features you want to include in your integration. Each module includes "AASB", “Platform” and “Engine” libraries. The AASB library includes the AASB messages supported for the module, the Platform library includes the configuration options required for a feature, and the Engine library augments the base functionality of the Engine with the underlying implementation of the feature. 

## Core Module

The Core module (see [README](./modules/core/README.md)) provides the infrastructure for audio input and output, authorization, logging, location reporting, metrics, property management, network monitoring services, local storage, and vehicle information services. The infrastructure is necessary for any module that uses the messaging interfaces (for example, the Alexa module). 

## Alexa Module

The Alexa module (see [README](./modules/alexa/README.md)) supports Alexa features such as speech input and output, authorization, volume control, media playback, equalizer control, template and state rendering, local media sources, alerts, notifications, and do not disturb. 

## Navigation Module

The Navigation module (see [README](./modules/navigation/README.md)) provides support for Alexa to interface with the onboard navigation system.

## Phone Call Controller Module

The Phone Call Controller module (see [README](./modules/phone-control/README.md)) provides support for Alexa to interface with the onboard telephony system.

## Address Book Module

The Address Book module (see [README](./modules/address-book/README.md)) augments the communications and navigation capabilities of Alexa with user data such as phone contacts and navigation favorites ("home", "work", etc.).

## Code-Based Linking (CBL) Module

The CBL module (see [README](./modules/cbl/README.md)) implements the CBL mechanism of acquiring Login with Amazon (LWA) access tokens. For information about the CBL mechanism, see the [Code-Based Linking documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/authorize-cbl.html).

## Alexa Presentation Language (APL) Module
The APL module (see [README](./modules/apl/README.md)) enables devices to support a visual Alexa experience.

>**Note:** The [APL Render module](./aacs/android/app-components/alexa-auto-apl-renderer/modules/apl-render/README.md) is provided to enable APL rendering capabilities in an Android application. 

## Messaging Module 
The Messaging module (see [README](./modules/messaging/README.md)) provides support for Short Message Service (SMS) capabilities of Alexa such as sending and reading text messages.

## Car Control Module
The Car Control module (see [README](./modules/car-control/README.md)) enables your application to build a custom vehicle-control experience that allows the user to voice-control vehicle features using Alexa.

## Connectivity Module
The Connectivity module (see [README](./modules/connectivity/README.md)) creates a lower data consumption mode for Alexa, allowing automakers to offer tiered functionality based on the status of their connectivity plans.

## Text To Speech (TTS) Module
The TTS module (see [README](./modules/text-to-speech/README.md)) enables a platform implementation to request synthesis of Alexa speech on demand from a text or Speech Synthesis Markup Language (SSML) string.

## Text To Speech (TTS) Provider Module
The TTS provider module (see [README](./modules/text-to-speech-provider/README.md)) synthesizes Alexa speech on demand. This module requires Auto SDK to be built with the Local Voice Control extension.
  
## Custom Domain Module
The Custom Domain module (see [README](./modules/custom-domain/README.md)) creates a bi-directional communication channel between the device and your custom cloud skills, allowing customized experience with the voice assistant. 

## AmazonLite Wake Word Extension
Wake Word enables hands-free, voice-initiated interactions with Alexa. The Wake Word extension enables AmazonLite Wake Word support in the Auto SDK.

## Alexa Communications Extension
The Alexa Communications extension enables integration with Alexa-to-Alexa calling, Alexa-to-PSTN calling, and messaging capabilities.

## Alexa Custom Assistant Extension
The Alexa Custom Assistant extension provides the functionality for toggling the settings of Alexa and the automaker's voice assistant, and notifies the IVI system at runtime about updates to the acting assistant for a specific interaction.

## Bluetooth Extension
The Bluetooth extension allows the Auto SDK to connect to devices through the Bluetooth Classic or Bluetooth Low Energy (BLE) protocol. Using these protocols, the Auto SDK can offer Bluetooth-based features to users of Android or iOS smartphones.

## Device Client Metrics (DCM) Extension
The Device Client Metrics (DCM) extension enables logging and uploading Auto SDK metrics to the Amazon cloud. Voice request metrics, for example, include start and end timestamps of user and Alexa speech and user perceived latency (UPL) between the request and Alexa’s response.

## Geolocation Extension
The Geolocation extension adds geolocation consent support to the Auto SDK, enabling the user to grant consent to location sharing with Alexa from your application.

## Local Voice Control (LVC) Extension
The LVC extension provides car control, communication, navigation, local search, and entertainment functionality, without an internet connection. It includes components that run an Alexa endpoint inside the vehicle's head unit.
### Local Voice Control Module
The Local Voice Control module adds core functionality to Auto SDK to enable offline features. The module infrastructure bridges the Auto SDK Engine to the offline Alexa endpoint running in the head unit and is necessary for all other modules in the LVC extension.
### Local Skill Service Module
The Local Skill Service module provides a multipurpose service to the Auto SDK Engine that enables components running alongside the offline Alexa endpoint to communicate with the Auto SDK Engine. The module infrastructure is necessary for other modules in the LVC extension.
### Local Navigation Module
The Local Navigation module enables you to provide customers with offline Alexa local search and navigation to points of interest (i.e., categories, chains, and entities) and addresses.
### Address Book Local Service Module
The Address Book Local Service module works with the Address Book module and the Local Skill Service module to augment the offline communications and navigation capabilities of Alexa with user data such as phone contacts and navigation favorites.
### Car Control Local Service Module
The Car Control Local Service module works with the Car Control module and the Local Skill Service module to enable users to control vehicle features offline with Alexa.

## Mobile Authorization Extension
The Mobile Authorization extension enables applications running on the vehicle's head unit to simplify the login experience. To log in to Alexa, the user uses the Alexa mobile app on a paired smartphone instead of opening a web browser and entering a code.

## Voice Chrome for Android Extension
The Voice Chrome extension adds Voice Chrome support to the Auto SDK for Android x86 64-bit and Android ARM 32/64-bit platforms. Voice Chrome provides a consistent set of visual cues representing Alexa attention state across a range of Alexa-enabled devices. The Voice Chrome extension includes a prebuilt Android AAR library for easy integration with your applications, as well as a patch to the Android Sample App that adds the Voice Chrome functionality.
