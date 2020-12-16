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
                        .setSmallIcon(R.drawable.alexa_placeholder_logo) // your app icon
                        .setBadgeIconType(NotificationCompat.BADGE_ICON_SMALL) // your app icon
                        .setContentTitle(context.getString(R.string.audio_io_notification_service_title))
                        .setNumber(1)
                        .setColor(255)
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
