package com.amazon.alexa.auto.aacs.common;

import androidx.annotation.NonNull;

/**
 * Data object to represent AACS Message.
 */
public class AACSMessage {
    @NonNull
    public final String messageId;
    @NonNull
    public final String topic;
    @NonNull
    public final String action;
    public final String payload;

    /**
     * Constructs the AACSMessage object.
     *
     * @param messageId Id of the message.
     * @param topic Topic of the message.
     * @param action Action of the message.
     * @param payload Optional payload.
     */
    /* Package */ AACSMessage(
            @NonNull String messageId, @NonNull String topic, @NonNull String action, String payload) {
        this.messageId = messageId;
        this.topic = topic;
        this.action = action;
        this.payload = payload;
    }
}
