# MediaPlaybackRequestor Interface

Alexa Media-Resume is a feature that helps Alexa play customers’ favorite content when they start their Alexa-enabled vehicles. Media-resume simplifies the content selection and playing process for customers, removing the need for them to use dash touch buttons or to ask Alexa.
To resume the media, Alexa Auto SDK needs to send `RequestMediaPlayback` event with the invocation reason set to `AUTOMOTIVE_STARTUP`. To implement a handler, subscribe to `MediaPlaybackResponse` messages.

`RequestMediaPlayback` is the message to send the event to the cloud. This message needs `invocationReason` and `elapsedBootTime` as input parameters.

`MediaPlaybackResponse` message receives the status of the `RequestMediaPlayback` message asynchronously.

`InvocationReason` enum indicates the invocation reason for calling the event. `AUTOMOTIVE_STARTUP` represents a situation where platform automatically sends `RequestMediaPlayback` message to automatically resume the media after infotainment system reboot. `EXPLICIT_USER_ACTION` represents resuming the media after explicit driver action by pressing the button or switch. Music resuming on `EXPLICIT_USER_ACTION` is not yet supported and this will be enabled in the future. Please check with your partner manager before using this action.

`MediaPlaybackRequestStatus` enum indicate the status of the `RequestMediaPlayback` message delivery. `SUCCESS` means `RequestMediaPlayback` event is successfully reported to the cloud. `FAILED_CAN_RETRY` means `RequestMediaPlayback` message can not be processed because Alexa Auto SDK is not in the connected state but platform implementation can retry after some time. `FAILED_TIMEOUT` means threshold time is crossed and media can not be resumed now. Driver can play media by making a voice request. `ERROR` means API could not be called successfully and media can not be resumed.

This feature needs following configuration. Please contact to your partner manager for finalizing the threshold numbers.

```json
"aace.alexa": {
    "requestMediaPlayback": {
      "mediaResumeThreshold": 30000
    }
}
```

`mediaResumeThreshold` is the maximum time in milliseconds to receive the `RequestMediaPlayback` message from the platform implementation. Platform implementation should consider their boot time, time to initialize Alexa and get connected to send the `RequestMediaPlayback` event successfully. Platform team and partner manager should try to keep this time minimum for the better user experience. Delayed media resume can surprise driver and result in driver distraction.
