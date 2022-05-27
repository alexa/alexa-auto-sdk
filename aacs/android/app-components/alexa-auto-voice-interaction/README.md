# Alexa Auto Voice Interaction

The following list describes the purposes of this library:

* It provides the Alexa Voice Interaction Service (VIS), which extends the Android Voice Interaction Service API. Alexa VIS enables Alexa as a voice assistant on an Android device. To start Alexa VIS, the user goes to `Settings > Apps & notifications > Default apps > Assist & voice input`, and selects Alexa.

    >**Note:** After Alexa VIS is integrated with the Android Voice Interaction API, infotainment system only supports one voice agent at a time. For example, the user cannot use both Alexa and Google Voice Assistant at the same time.

* It is responsible for managing the Alexa Auto Client Service (AACS) lifecycle, based on the Alexa VIS lifecycle.
* When there is new voice session created, it starts an assistant activity displayed on top of other activities in the system.
* It provides the Alexa Voice Assist Settings, such as Alexa Privacy Mode and Alexa Locale. To open Alexa Voice Assist Settings, the user goes to `Settings > Apps & notifications > Default apps > Assist & voice input`, makes sure Alexa is selected, and clicks the Settings icon beside `Assist app` to open the Alexa Voice Assist Settings homepage.

When the user selects Alexa as the assistant, AACS is started and connected to Alexa. When the user selects another voice assistant, Alexa VIS shuts down. It also stops AACS.