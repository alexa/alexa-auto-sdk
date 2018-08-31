## Phone Call Controller API

### Overview

The Alexa Auto Phone Call Controller API provides the features required by a platform implementation to interface with the phone call controller capabilities of Alexa.

### Handling PhoneCallController

It is the responsibility of the platform implementation to handle dial requests from Alexa. The platform implementation must also inform the Engine of calling events, and the status of the connected calling device on the platform. The platform implementation is responsible for managing the lifecycle and user experience of a call session and connection to a calling device.

To implement a custom phone control handler for handling phone control events from Alexa, the `PhoneCallController` class should be extended:

```
public class PhoneCallControllerHandler extends PhoneCallController
{
	@Override
	public boolean dial( String payload )
	{
		...
		//handle making the call, and keep reference to callId
		...
		callActivated(callId); //inform the engine the call is now active
		...

		if (SOME_ERROR_CASE){
			callFailed( callId, error, message ); //inform the engine that the call failed
	...

	...
		//on some platform handler
		connectionStatusChanged( ConnectionState.CONNECTED ); //phone call device is connected
	...

	...
		// on receiving a dial call or initiating a call outside the scope of Alexa
		String callId = ... // unique identifier given from dial, or UUID generated from the platform for calls outside of Alexa
		callActivated( callId );
	...

	...
		// on an active call was ended or an ongoing phone call setup was cancelled
		String callId = ... // unique identifier for call session
		callTerminated( callId );
	...

	...
		// on error handling for initiating or maintaining a call on the platform calling device
		String callId = ... // unique identifier for call session
		String error = ... // platform error code, ex. 4xx, 500, 503
		String message = ... //platform error message
		callFailed( callId, error, message );
	...

```

Example Dial Directive JSON String payload:

```
{
  "callId": "{{STRING}}",
  "callee": {
    "details" : "{{STRING}}"
    "defaultAddress" : {
      "protocol" : "{{STRING}}",
      "format" : "{{STRING}}",
      "value" : "{{STRING}}"
    }
    "alternativeAddresses" : [{
      "protocol" : "{{STRING}}",
      "format" : "{{STRING}}",
      "value" : "{{STRING}}"
    }]
  }
}
```

### Whitelisting

Please see the [Need Help?](../../NEED_HELP.md) page for how to whitelist your device for using PhoneCallController APIs.
