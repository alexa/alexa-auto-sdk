package com.amazon.alexa.auto.aacs.common;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * Data object to represent AACS Reply Message.
 */
public class AACSReplyMessage {
    @NonNull
    public final String messageId;
    @NonNull
    public final String replyToId;
    @NonNull
    public final String topic;
    @NonNull
    public final String action;
    @Nullable
    public final String payload;

    /**
     * Constructs the AACSReplyMessage object.
     *
     * @param messageId Id of the message.
     * @param replyToId Reply id (Id of request for this reply).
     * @param payload Optional payload.
     */
    /* Package */ AACSReplyMessage(@NonNull String messageId, @NonNull String replyToId, @NonNull String topic,
            @NonNull String action, @Nullable String payload) {
        this.messageId = messageId;
        this.replyToId = replyToId;
        this.topic = topic;
        this.action = action;
        this.payload = payload;
    }
}
