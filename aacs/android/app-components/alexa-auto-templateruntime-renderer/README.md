# Alexa Auto Template Runtime Renderer

This library houses template runtime implementations. This library is one of the subscribers for the `TemplateRuntime::RenderTemplate` directive. It parses the incoming directives and renders the display cards in the current voice session provided by the Alexa Voice Interaction Service (VIS).

The template is automatically dismissed 8 seconds after the voice response from Alexa is completed unless the card is interacted with, in which case the card dismissal gets extended by 8 seconds from the last interaction.

The following template runtime directives are currently supported

* WeatherTemplate

* BodyTemplate1

* BodyTemplate2

* ListTemplate1

For more details refer the [automotive HMI guidelines](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/display-cards.html).
    
## Known Issues
* The weather card is comprised of 2 screens - current weather and weather forecast. The card currently scrolls continuously which is not the correct behavior. It should be paginated.
