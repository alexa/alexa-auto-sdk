# NetworkInfoProvider Interface

Your application should monitor the internet connection and notify the Engine of changes in the status using the [`NetworkInfoProvider`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/NetworkInfoProvider/index.html) interface. The Engine uses this information to adjust its behavior, including tearing down the connection to Alexa cloud when your application reports that it has no connection to the internet. Although using `NetworkInfoProvider` is optional, you should use it so the Engine can avoid undesirable behavior; for instance, attempting to send events to Alexa when the lack of connectivity means the events are bound to fail.

>**Note:** You must use the `NetworkInfoProvider` interface if your application uses the Local Voice Control (LVC) extension.

> **Note:** For Android applications, AACS provides a default implementation of `NetworkInfoProvider`. See the [AACS Default Implementation](https://alexa.github.io/alexa-auto-sdk/aacs/android#default-platform-implementation) documentation for more information.

Various Engine components want the initial network status at startup so they can adapt their initial behavior accordingly. Your application should subscribe to the [`NetworkInfoProvider.GetNetworkStatus`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/NetworkInfoProvider/index.html#getnetworkstatus) and [`NetworkInfoProvider.GetWifiSignalStrength`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/NetworkInfoProvider/index.html#getwifisignalstrength) messages to answer the initial query from the Engine. These messages are synchronous-style and require your application to send the corresponding reply messages right away. 

At runtime, publish the [`NetworkInfoProvider.NetworkStatusChanged`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/NetworkInfoProvider/index.html#networkstatuschanged) message to notify the Engine of any status changes.