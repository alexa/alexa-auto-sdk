# Alexa Auto Voice Interaction

The following list describes the purposes of this library:
* It provides the Alexa Voice Interaction Service (VIS), which extends the Android Voice Interaction Service API. Alexa VIS enables Alexa as a voice assistant on an Android device. To start Alexa VIS, the user goes to `Settings > Apps & notifications > Default apps > Assist & voice input`, and selects Alexa. 
    >**Note:** After Alexa VIS is integrated with the Android Voice Interaction API, it only supports one voice agent at a time. For example, the user cannot use both Alexa and Google Voice Assistant at the same time. 
* It handles Alexa Voice Chrome UI based on the user's request.  Voice Chrome requires that Alexa be selected as the device's voice assistant.
* It is responsible for managing the Alexa Auto Client Service (AACS) lifecycle, based on the Alexa VIS lifecycle.
* It gets the voice interaction session view and vends it out to the app components that inflate onto that view. To access the VIS component, consumers need to get the `SessionViewController` object via Alexa-Auto-Apis. (See the example in alexa-auto-navigation : LocalSearchDirectiveHandler.)
* It provides the Alexa Voice Assist Settings, such as Alexa Privacy Mode and Alexa Locale. To open Alexa Voice Assist Settings, the user goes to `Settings > Apps & notifications > Default apps > Assist & voice input`, makes sure Alexa is selected, and clicks the Settings icon beside `Assist app` to open the Alexa Voice Assist Settings homepage.

When the user selects Alexa as the assistant, AACS is started and connected to Alexa. When the user selects another voice assistant, Alexa VIS shuts down. It also stops AACS.