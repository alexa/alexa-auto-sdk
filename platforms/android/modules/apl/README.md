# Alexa Presentation Language (APL) Module

The Alexa Auto SDK Alexa Presentation Language (APL) module provides the interfaces required by your platform implementation to interface with the APL capabilities of Alexa.

**Table of Contents:**

* [Overview](#overview)
* [Building the APL Module](#building-the-apl-module)
* [Registering an APL Handler](#registering-an-apl-handler)
* [References](#references)


## Overview <a id ="overview"></a>

You should extend the APL module to handle receiving Alexa Presentation `RenderDocument` and `ExecuteCommands` directives from AVS. These directives contain metadata for rendering or operating on display cards for devices with GUI support. Additionally, your APL handler should send `UserEvent` events to notify AVS of user activity within the rendered document.

>**Note:** The APL module doesn't render APL documents; it provides the conduit to render APL documents and process user events from the graphical user interface (GUI) and/or voice-user interface (VUI). APL rendering is available separately for Android platforms.

## Building the APL Module <a id ="building-the-apl-module"></a>

The APL module depends on [Alexa Smart Screen SDK](https://github.com/alexa/alexa-smart-screen-sdk) which is available on GitHub as open source software from Amazon. The [Alexa Auto SDK Builder](../../../../builder/README.md) automatically builds the APL module for the Alexa Auto SDK, along with necessary components of the Alexa Smart Screen SDK (downloaded as part of the build process), for the specified target.

### APL Rendering for Android

You can implement APL Rendering for Android by extending the Sample App and Web View Host from the Smart Screen SDK. See the [SDK GUI API](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/SDK-GUI-API.md) for details.

Additional native functionality is available with help from your designated Amazon Solutions Architect (SA) or Partner Manager.


## Registering an APL Handler <a id = "registering-an-apl-handler"></a>

To implement a custom handler for APL, extend the `com.amazon.aace.apl.APL` class.

```java
public class APLHandler extends APL {

    public APLHandler() {
        ...
    }

    @Override
    public String getVisualContext() {
        ...
    }

    @Override
    public void renderDocument(String jsonPayload, String token, String windowId) {
        ...
    }

    @Override
    public void clearDocument() {
        ...
    }

    @Override
    public void executeCommands(String payload, String token) {
        ...
    }

    @Override
    public void interruptCommandSequence() {
        ...
    }

}
```

**getVisualContext()**

When `getVisualContext` is called, the platform implementation returns the visual context provided by the APL rendering engine.

**renderDocument( jsonPayload, token, windowId )**

When `renderDocument` is called, the platform implementation renders the document based on the APL specification in the `jsonPayload` argument, then calls `renderDocumentResult` to indicate success or failure (along with an error message). The `token` and `windowId` arguments are used by the rendering engine to manage the APL presentation.

>**Important!:** The payload may contain customer sensitive information and should be used with utmost care. Failure to do so may result in exposing or mishandling of customer data.

**clearDocument()**

When `clearDocument` is called, the platform implementation clears the APL display card, then calls `clearCard` to indicate that the card was cleared.

**executeCommands( jsonPayload, token )**

When `executeCommands` is called, the platform implementation executes the commands based on the APL specification in the payload, then calls `executeCommandsResult` to indicate success or failure (along with an error message). The `token` argument is used by the rendering engine to manage the APL presentation.

**interruptCommandSequence()**

When `interruptCommandSequence()` is called, the platform implementation should interrupt the execution sequence.


## References <a id ="references"></a>

The following resources provide more information about APL:

* [Understand Alexa Presentation Language (APL)](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html)
  * [APL RenderDocument Directive](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-render-document-skill-directive.html)
  * [APL ExecuteCommands Directive](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-execute-command-directive.html)
  * [APL Viewport Characteristics](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-viewport-characteristics.html)
  * [APL UserEvent Event](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-standard-commands.html#userevent)