# Explore Auto SDK Features

Learn about the Auto SDK features that you can integrate into your application.

## Modules on Github

Auto SDK provides the following modules on Github:

### Address Book module

The `Address Book` module personalizes the communications and navigation capabilities of Alexa by linking the user's phone contacts and favorite navigation locations. When used with the `Phone Control` and `Navigation` modules, `Address Book` enables the user to call contacts by name and navigate to their favorite destinations.

**[>> Address Book module reference](./address-book/index.md)**

### Alexa module

The `Alexa` module provides the core Alexa client implementation to your application. The Engine components of the Alexa module manage the connection to the [Alexa Voice Service (AVS)](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/get-started-with-alexa-voice-service.html) and support the standard AVS capabilities such as speaking to Alexa, streaming media, viewing visual content, setting alerts, and more.

**[>> Alexa module reference](./alexa/index.md)**

### Alexa Presentation Language (APL) module

The `APL` module enables your application to display rich visual experiences when the user interacts with Alexa.

**[>> APL module reference](./apl/index.md)**

### Bluetooth module

The `Bluetooth` module allows the Auto SDK Engine to connect to devices through the Bluetooth Classic or Bluetooth Low Energy (BLE) protocol. Additional Auto SDK modules use the functionality of the `Bluetooth` module to provide bluetooth-based features, such as mobile authorization, to users of Android or iOS smartphones.

**[>> Bluetooth module reference](./bluetooth/index.md)**

### Car Control module

The `Car Control` module enables the user to control vehicle features—turning on seat heaters, adjusting the AC setting, opening windows, and much more—just by asking Alexa.

**[>> Car Control module reference](./car-control/index.md)**

### Code-Based Linking (CBL) module

The `CBL` module implements the code-based linking mechanism of acquiring Login with Amazon (LWA) access tokens, which are required to use Alexa. Your application displays a URL and code to the user to complete the sign-in from a second device, and the Engine takes care of fetching the tokens.

**[>> CBL module reference](./cbl/index.md)**

### Connectivity module

The `Connectivity` module enables a reduced data consumption mode for Alexa. Your application can offer different tiers of Alexa functionality based on the status of the head unit's data connectivity plan.

**[>> Connectivity module reference](./connectivity/index.md)**

### Core module

The `Core` module provides the Auto SDK infrastructure upon which all other modules depend. `Core` includes interfaces for audio input and output, authorization, logging, location reporting, metrics, setting management, network status reporting, and more.

**[>> Core module reference](./core/index.md)**

### Custom Domain module

The `Custom Domain` module enables you to enhance your voice assistant with custom functionality. `Custom Domain` creates a bi-directional communication channel between the head unit and your own cloud skills so your application can exchange custom events and directives with your skills.

**[>> Custom Domain module reference](./custom-domain/index.md)**

### Loopback Detector module

The `Loopback Detector` module suppresses false wake word detections in which Alexa uses her own name in her speech output. The Engine uses a loopback audio stream from the vehicle's own speakers to detect any wake words and prevent false wake-ups during interactions.

**[>> Loopback Detector module reference](./loopback-detector/index.md)**

### Messaging module 

The `Messaging` module adds voice-forward Short Message Service (SMS) features to your application. Users can request Alexa to send or read text messages using the phone connected to the head unit.

**[>> Messaging module reference](./messaging/index.md)**

### Navigation module

The `Navigation` module provides support for Alexa to interface with the head unit's onboard navigation system. Users can ask Alexa to navigate to points of interest and addresses, add stops to the route, answer questions about the route, and more.

**[>> Navigation module reference](./navigation/index.md)**

### Phone Control module

The `Phone Control` module adds voice-forward telephony features to your application. Users can ask Alexa to call numbers or contacts using the vehicle's native telephony system or connected phone.

**[>> Phone Control module reference](./phone-control/index.md)**

### System Audio module

The `System Audio` module provides an out-of-box implementation of the `Core` module's audio input and output interfaces to simplify the audio management your native C++ integration. 

**[>> System Audio module reference](./system-audio/index.md)**

### Text-To-Speech module

The `Text-To-Speech` module enables your application to request synthesis of Alexa speech on demand from text or Speech Synthesis Markup Language (SSML) string. You can use `Text-To-Speech` with `Text-To-Speech-Provider` to provide turn-by-turn navigation instructions in Alexa's voice.

**[>> Text-To-Speech module reference](./text-to-speech/index.md)**

### Text-To-Speech Provider module

The `Text-To-Speech Provider` module synthesizes Alexa speech on demand. The `Text-To-Speech Provider` module requires the `Local Voice Control` Auto SDK extension.

**[>> Text-To-Speech Provider module reference](./text-to-speech-provider/index.md)**

## Extension modules

Auto SDK provides the following modules in private extensions on the Alexa developer console. Contact your Amazon Solutions Architect (SA) or Partner Manager for access.

### Alexa Communication module

The `Alexa Communication` (or `Alexa Comms`) module enables users to place two-way Alexa-to-Alexa calls to a remote Alexa device, drop in on a remote Alexa device, or send an announcement to a remote Alexa device.

> The `Alexa Communication` module is part of the [`Alexa Communication` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Alexa%20Comms%20Extension).

### Alexa Custom Assistant module

The `Alexa Custom Assistant` module enables you to develop in-vehicle infotainment (IVI) software in which the user can easily interact with both Alexa and your own branded voice assistant.

> The `Alexa Custom Assistant` module is part of the [`Alexa Custom Assistant` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Custom%20Assistant%20Extension). `Alexa Custom Assistant` requires the `Local Voice Control` extension.

### Amazonlite module

The `Amazonlite` module enables hands-free voice-initiated interactions with Alexa powered by the Amazonlite wake word detection engine.

> The `Amazonlite` module is part of the [`Amazonlite` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Amazonlite%20Extension).

### Device Client Metrics (DCM) module

The `DCM` module enables the Engine to upload Auto SDK performance metrics, such as user-perceived latency, to the Amazon cloud.

> The `DCM` module is part of the [`Device Client Metrics` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Metric%20Upload%20Service%20Extension).

### Geolocation module

The `Geolocation` module adds location consent features to Auto SDK. Without the `Geolocation` module, the user consents to share location with Alexa while registering the head unit to their Amazon account (i.e., during the sign in flow). With `Geolocation`, the user can provide or revoke consent directly from your application.

> The `Geolocation` module is part of the [`Geolocation` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/df3a5c0f-8a81-486e-803b-f4e168afb24e).

### Local Voice Control (LVC) extension

The [`Local Voice Control` (`LVC`) extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Local%20Voice%20Control%20Extension) provides several modules that work together to enable features—car control, calling, navigation, local search, entertainment, and more—without an internet connection. In addition to Auto SDK modules, the LVC extension provides separate components that run a local Alexa endpoint inside the vehicle head unit.

#### Local Voice Control module
The` Local Voice Control` module adds core functionality to Auto SDK to enable offline features. The module infrastructure bridges the Auto SDK Engine to the offline Alexa endpoint running in the head unit and is necessary for all other modules in the LVC extension.

#### Local Skill Service module
The `Local Skill Service` module provides a multipurpose service to the Auto SDK Engine that enables components running alongside the offline Alexa endpoint to communicate with the Auto SDK Engine. The `Local Skill Service` infrastructure is necessary for other modules in the LVC extension.

#### Local Navigation module
The `Local Navigation` module enables you to provide customers with offline Alexa local search and navigation to points of interest and addresses.

#### Address Book Local Service module
The `Address Book Local Service` module works with the `Address Book` module and the `Local Skill Service` module to augment the offline communications and navigation capabilities of Alexa with the user's phone contacts and favorite navigation locations.

#### Car Control Local Service module
The `Car Control Local Service` module works with the `Car Control` module and the `Local Skill Service` module to enable users to control vehicle features offline with Alexa.

### Mobile Authorization module

The `Mobile Authorization` module simplifies your user's sign-in experience. Instead of opening a web browser and entering a code, the user signs in to Alexa with the Alexa app on their bluetooth-paired smartphone.

> The `Mobile Authorization` module is part of the [`Mobile Authorization` Auto SDK extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/069fc380-64dc-4943-85a2-37ada266736f).

### Voice Chrome extension for Android

The [`Voice Chrome` extension](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Voice%20Chrome%20Extension) provides an Android library that you can use to add the Alexa visual attention state "voice chrome" to the UI of your application.

> **Note:** The Voice Chrome library is an Android app component you can use with Alexa Auto Client Service for Android, not a standard Auto SDK module that has a native C++ Engine implementation.