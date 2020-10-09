package com.amazon.alexa.auto.aacs.common;

import androidx.annotation.NonNull;

/**
 * Data object to represent AACS Reply Message.
 */
public class AACSReplyMessage {
    @NonNull
    public final String messageId;
    @NonNull
    public final String replyToId;
    public final String payload;

    /**
     * Constructs the AACSReplyMessage object.
     *
     * @param messageId Id of the message.
     * @param replyToId Reply id (Id of request for this reply).
     * @param payload Optional payload.
     */
    /* Package */ AACSReplyMessage(@NonNull String messageId, @NonNull String replyToId, String payload) {
        this.messageId = messageId;
        this.replyToId = replyToId;
        this.payload = payload;
    }
}
