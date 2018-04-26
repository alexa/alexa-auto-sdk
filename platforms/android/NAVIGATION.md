## Navigation API

### Overview

The Navigation module includes a navigation platform interface to handle Alexa navigation events and directives. This means it depends on the [Alexa module](./ALEXA.md).

### Handling Navigation

The Engine provides callbacks for handling navigation directives from Alexa. The platform implementation must also inform the Engine of the status of navigation on the platform. This is optional and dependent on the platform implementation.

To implement a custom navigation handler for handling navigation events from Alexa, the `Navigation` class should be extended:

```
public class NavigationHandler extends Navigation
{
	@Override
	public boolean setDestination( String payload )
	{
		//handle setting the destination payload
```

Example JSON payload:

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
	}
}

```

### Whitelisting

Please see the [Need Help?](../../NEED_HELP.md) page for how to whitelist your device for using Navigation APIs.
