package com.amazon.alexa.auto.voice.ui.common;

import androidx.annotation.Nullable;

/**
 * Alexa Auto Voice UI Message
 */
public class AutoVoiceUIMessage {
    private String topic;
    private String action;
    private String payload;

    public AutoVoiceUIMessage(String topic, String action, @Nullable String payload) {
        this.topic = topic;
        this.action = action;
        this.payload = payload;
    }

    public String getTopic() {
        return this.topic;
    }

    public String getAction() {
        return this.action;
    }

    public String getPayload() {
        return this.payload;
    }
}
