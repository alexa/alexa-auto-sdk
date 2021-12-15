# Alexa Auto Voice UI

The following list describes the purposes of this library:
* It handles Alexa Voice Chrome UI based on the user's request. Voice Chrome requires that Alexa be selected as the device's voice assistant.
* It provides the voice interaction session view and vends it out to the app components that inflate onto that view. To access the VIS component, consumers need to get the `SessionViewController` object via `alexa-auto-apis`. (See the example in `alexa-auto-navigation` : [LocalSearchDirectiveHandler](../alexa-auto-navigation/src/main/java/com/amazon/alexa/auto/navigation/handlers/LocalSearchDirectiveHandler.java).)
* It provides the voice interaction session activity and vends it out to the app components that add voice fragment onto that view. Consumers need to get the `SessionActivityController` object via `alexa-auto-apis`. (See the example in `alexa-auto-apl-renderer` : [APLReceiver](../alexa-auto-apl-renderer/src/main/java/com/amazon/alexa/auto/apl/receiver/APLReceiver.java).)
* It provides support for offline network error prompts. With this feature, an offline prompt is played to users when Alexa is unable to respond to utterances in case the internet is not reachable. The prompt is played in the currently active system locale and the error prompt files for the different locales are stored in `res/raw/auto_error_offline_{locale}.mp3`

## Known Issues

* This only applies to Alexa Custom Assistant - If Alexa is disabled, [Brandon] is enabled and network is unavailable, the voice prompt will be played in Alexa's voice, not in [Brandon]'s.