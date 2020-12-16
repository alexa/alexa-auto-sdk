package com.amazon.alexa.auto.voiceinteraction.common;

public class Constants {
    // AACS Configs
    public static final String AACS_CONFIG_FILE_PATH = "configFilepaths";
    public static final String AACS_CONFIG_STRINGS = "configStrings";

    // Voice Interaction Topic
    public static final String TOPIC_VOICE_CHROME = "VOICE_CHROME";
    public static final String TOPIC_ALEXA_CONNECTION = "ALEXA_CONNECTION";

    // Voice Interaction Action
    public static final String ACTION_ALEXA_NOT_CONNECTED = "ALEXA_NOT_CONNECTED";

    // AACS components
    public static final String AACS_PACKAGE = "com.amazon.alexaautoclientservice";
    public static final String AACS_CLASS = "com.amazon.alexaautoclientservice.AlexaAutoClientService";

    // AACS intents
    public static final String DIALOG_STATE_CHANGE = "com.amazon.aacs.aasb.DialogStateChanged";
    public static final String CONNECTION_STATUS_CONNECTED = "CONNECTED";
    public static final String CONNECTION_STATUS_DISCONNECTED = "DISCONNECTED";

    public static final String EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN = "exitAfterLogin";
}
