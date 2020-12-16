package com.amazon.alexa.auto.aacs.common;

import android.content.ComponentName;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

/**
 * Class for sending messages to AACS service.
 */
public class AACSMessageSender {
    private static final String TAG = AACSMessageSender.class.getSimpleName();

    @NonNull
    private WeakReference<Context> mContextWk;
    @NonNull
    private AACSSender mAACSSender;
    @NonNull
    private TargetComponent mAACSTarget;

    /**
     * Constructs an instance of @c AACSMessageSender.
     *
     * @param contextWk Android Context which will be used for sending intents
     *                  to AACS.
     * @param sender AACS Sender.
     */
    public AACSMessageSender(@NonNull WeakReference<Context> contextWk, @NonNull AACSSender sender) {
        this.mContextWk = contextWk;
        this.mAACSSender = sender;
        this.mAACSTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.AACS_PACKAGE_NAME, AACSConstants.AACS_CLASS_NAME),
                TargetComponent.Type.SERVICE);
    }

    /**
     * Send message to AACS.
     *
     * @param topic Topic of the message (typically capability name).
     * @param action Action of the message.
     * @param payload Optional payload of the message.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendMessage(@NonNull String topic, @NonNull String action, @Nullable String payload) {
        Context context = mContextWk.get();
        if (context == null) {
            Log.w(TAG, "Invalid context found while sending message. Topic: " + topic + " Action: " + action);
            return CompletableFuture.completedFuture(false);
        }

        Optional<String> message = AACSMessageBuilder.buildMessage(topic, action, payload);
        if (!message.isPresent()) {
            Log.w(TAG,
                    "Failed to build AACS payload for message. Topic: " + topic + " Action: " + action
                            + " Message payload: " + payload);
            return CompletableFuture.completedFuture(false);
        }

        return this.mAACSSender.sendAASBMessageAnySize(message.get(), action, topic, this.mAACSTarget, context);
    }

    /**
     * Send reply message to AACS.
     *
     * @param replyToId Message Id that is sent with AACS Request.
     * @param topic Topic of the message (typically capability name).
     * @param action Action of the message.
     * @param payload Optional payload of the message.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendReplyMessage(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, @Nullable String payload) {
        Context context = mContextWk.get();
        if (context == null) {
            Log.w(TAG,
                    "Invalid context found while sending reply. ReplyId: " + replyToId + " Topic: " + topic
                            + " Action: " + action);
            return CompletableFuture.completedFuture(false);
        }

        Optional<String> message = AACSMessageBuilder.buildReplyMessage(replyToId, topic, action, payload);
        if (!message.isPresent()) {
            Log.w(TAG,
                    "Failed to build AACS payload for reply. ReplyId: " + replyToId + " Topic: " + topic
                            + " Action: " + action + " Message payload: " + payload);
            return CompletableFuture.completedFuture(false);
        }

        return this.mAACSSender.sendAASBMessageAnySize(message.get(), action, topic, this.mAACSTarget, context);
    }
}
