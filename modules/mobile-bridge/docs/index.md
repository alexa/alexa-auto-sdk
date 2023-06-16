# Mobile Bridge

## Overview

Alexa Auto Mobile Bridge (AAMB) aims to help vehicle drivers use Alexa in the car via their phone's data plan. After provisioning the AAMB feature, the head unit is able to use user's phone as a proxy to forward all network requests made by Alexa or other allow-listed apps.

The Mobile Bridge module provides platform-agnostic implementation of AAMB in Auto SDK and exposes platform interfaces for the app based on Auto SDK to provide platform-specific implementations.

## Prerequisites

- C++ toolchain supporting C++14 standard
- libtins 4.4 and libevent 2.1.12 as Conan dependency

## Implementation

The Mobile Bridge modules provides the following two core components:

- Session Manager: a C++ implementation to parse IP packets and generate the corresponding proxy requests to the Transport Manager.
- Transport Manager: a C++ implementation to serve as a local proxy and forward the network traffic to the active transport.

### Transport Manager

Transport Manager listens on a few local TCP/UDP ports to provide a local TCP/UDP proxy.

### Session Manager

Session Manager requires the platform to provide a file descriptor for [TUN interface](https://docs.kernel.org/networking/tuntap.html) while starting the Mobile Bridge feature. On Android, obtain the file descriptor by creating a [VpnService](https://developer.android.com/reference/android/net/VpnService).

Currently Session Manager ignores IP v6 packets and only IP v4 is supported.

## Configuring the Mobile Bridge Module

The Mobile Bridge module allows customization of the following configuration:

- Transport Manager
	- TCP proxy port number
	- UDP proxy port number

The following JSON object illustrates the list of supported configuration settings and their default values:

```json
{
  "aace.mobileBridge": {
    "tcp-proxy-port": 9876,
    "udp-proxy-port": 9877
  }
}
```

See [Configure the Engine](https://alexa.github.io/alexa-auto-sdk/docs/native/#configure-the-engine) for onfiguring Auto SDK Engine with the preceding JSON configuration.

# Platform Implementation

AAMB expects the platform to provide the following implementations:

- Get the list of available transports
- Connect/Disconnect a transport
- Get notified about a device has complete hand-shaking and authorize it for Internet connectivity

See the following sequence diagram for how mobile bridge engine interacts with platform implementation:

![](./diagrams/mobile-bridge.svg)

App can provide the platform implementation by subscribing message handlers or sending messages with [MessageBroker API](https://alexa.github.io/alexa-auto-sdk/docs/native/#understand-how-to-use-messagebroker).
