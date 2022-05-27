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
package com.amazon.alexa.auto.app.audio;

import android.annotation.TargetApi;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;

import androidx.annotation.NonNull;
import androidx.core.app.NotificationCompat;

import com.amazon.alexa.auto.app.R;

public class AudioServiceNotificationBuilder {
    public static final String AUDIO_NOTIFICATION_CHANNEL_ID = "com.amazon.alexa.auto.app.AudioIO";
    public static final int AUDIO_NOTIFICATION_ID = 0x100;

    public static Notification buildNotification(
            @NonNull Context context, @NonNull NotificationManager notificationManager) {
        createNotificationChannel(context, notificationManager);

        NotificationCompat.Builder notificationBuilder =
                new NotificationCompat.Builder(context, AUDIO_NOTIFICATION_CHANNEL_ID)
                        .setSmallIcon(R.drawable.alexa_bubble_small)
                        .setBadgeIconType(NotificationCompat.BADGE_ICON_SMALL)
                        .setContentTitle(context.getString(R.string.audio_io_notification_service_title))
                        .setNumber(1)
                        .setWhen(System.currentTimeMillis());

        return notificationBuilder.build();
    }

    @TargetApi(Build.VERSION_CODES.O)
    private static void createNotificationChannel(
            @NonNull Context context, @NonNull NotificationManager notificationManager) {
        NotificationChannel channel = new NotificationChannel(AUDIO_NOTIFICATION_CHANNEL_ID,
                context.getString(R.string.audio_io_notification_channel_name), NotificationManager.IMPORTANCE_DEFAULT);

        channel.setDescription(context.getString(R.string.audio_io_notification_channel_description));

        notificationManager.createNotificationChannel(channel);
    }
}
