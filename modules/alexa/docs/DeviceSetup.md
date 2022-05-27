# DeviceSetup Interface

>**Note:** This feature requires Amazon to allowlist your device. For help, contact your Amazon Solutions Architect or partner manager.

After the user signs in to your application during or after the out-of-box experience, your application starts the Engine and publishes the [`DeviceSetup.SetupCompleted` message](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/DeviceSetup/index.html#setupcompleted) to notify Alexa that the setup is complete. The Engine publishes the [`DeviceSetup.SetupCompletedResponse`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/alexa/DeviceSetup/index.html#setupcompletedresponse) to your application to indicate Alexa was notified successfully. In response to the `SetupCompleted` event, Alexa starts an onboarding experience including a short first-time conversation with the user. 

Because `SetupCompleted` triggers an onboarding experience, do not publish the message if the signed-in user has already seen the experience. The onboarding experience is for first-time users only and might differ for returning users.

>**Note:** Do not publish the `SetupCompleted` message if user is in [Connectivity mode](https://alexa.github.io/alexa-auto-sdk/docs/explore/features/connectivity/) or Preview Mode or if the user has disabled hands-free listening. Publishing `SetupCompleted` in these conditions causes undesirable user experience.
