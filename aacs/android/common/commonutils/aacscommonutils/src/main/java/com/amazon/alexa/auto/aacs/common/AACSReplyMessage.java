/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
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
