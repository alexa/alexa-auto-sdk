package com.amazon.alexa.auto.voiceinteraction.common;

public class Constants {
    // Voice Interaction Topic
    public static final String TOPIC_VOICE_ANIMATION = "VOICE_ANIMATION";
    public static final String TOPIC_ALEXA_CONNECTION = "ALEXA_CONNECTION";

    // Voice Interaction Action
    public static final String ACTION_ALEXA_NOT_CONNECTED = "ALEXA_NOT_CONNECTED";
    public static final String ACTION_CUSTOM_PAYLOAD = "ACTION_CUSTOM_PAYLOAD";

    // AACS components
    public static final String AACS_PACKAGE = "com.amazon.alexaautoclientservice";
    public static final String AACS_CLASS = "com.amazon.alexaautoclientservice.AlexaAutoClientService";

    // AACS intents
    public static final String DIALOG_STATE_CHANGE = "com.amazon.aacs.aasb.DialogStateChanged";
    public static final String CONNECTION_STATUS_CONNECTED = "CONNECTED";
    public static final String CONNECTION_STATUS_DISCONNECTED = "DISCONNECTED";
}
