package com.amazon.alexa.auto.voiceinteraction.common;

import androidx.annotation.Nullable;

/**
 * Alexa Auto Voice Interaction Message
 */
public class AutoVoiceInteractionMessage {
    private String topic;
    private String action;
    private String payload;

    public AutoVoiceInteractionMessage(String topic, String action, @Nullable String payload) {
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
