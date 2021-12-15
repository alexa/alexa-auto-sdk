package com.amazon.alexaautoclientservice.mediaPlayer;

import androidx.annotation.NonNull;

@FunctionalInterface
public interface EventReceiver {
    void onEvent(@NonNull String replyToId, @NonNull String topic, @NonNull String action, @NonNull String payload);
}
