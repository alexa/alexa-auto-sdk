package com.amazon.alexa.auto.apps.common.message;

import androidx.annotation.Nullable;

/**
 * Assistant Message
 */
public class AssistantMessage {
    private String assistantName;
    private String action;
    private String payload;

    public AssistantMessage(String assistantName, String action, @Nullable String payload) {
        this.assistantName = assistantName;
        this.action = action;
        this.payload = payload;
    }

    public String getAssistantName() {
        return this.assistantName;
    }

    public String getAction() {
        return this.action;
    }

    public String getPayload() {
        return this.payload;
    }
}
