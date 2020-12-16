package com.amazon.alexa.auto.media.player;

import android.annotation.TargetApi;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;

import androidx.annotation.NonNull;
import androidx.core.app.NotificationCompat;

import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.R;

/**
 * A helper class to create Android Notifications for the music service.
 * The notification is necessary for Android version >= O, so it would
 * be created as soon as service starts.
 * Once the music starts playing, the notification would be updated to
 * include music metadata and media transport control.
 */
public class NotificationController {
    public static final String ALEXA_MEDIA_CHANNEL_ID = "com.amazon.alexa.auto.AudioPlayer";
    public static final int ALEXA_MEDIA_NOTIFICATION_ID = 0x100;

    @NonNull
    private final NotificationManager mNotificationManager;

    public NotificationController(@NonNull Context context) {
        this.mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        Preconditions.checkNotNull(this.mNotificationManager);

        initializeNotificationChannel(context, mNotificationManager);
    }

    public Notification createServiceStartNotification(@NonNull Context context) {
        NotificationCompat.Builder notificationBuilder =
                new NotificationCompat.Builder(context, ALEXA_MEDIA_CHANNEL_ID)
                        .setChannelId(ALEXA_MEDIA_CHANNEL_ID)
                        .setSmallIcon(R.drawable.alexa_placeholder_logo) // your app icon
                        .setBadgeIconType(NotificationCompat.BADGE_ICON_SMALL) // your app icon
                        .setContentTitle(context.getString(R.string.notification_service_title))
                        //.setAutoCancel(true).setContentIntent(pendingIntent)
                        .setNumber(1)
                        .setColor(255)
                        //.setContentText(extras.get("nm").toString())
                        .setWhen(System.currentTimeMillis());

        return notificationBuilder.build();
    }

    @TargetApi(Build.VERSION_CODES.O)
    private static void initializeNotificationChannel(
            @NonNull Context context, @NonNull NotificationManager notificationManager) {
        NotificationChannel channel = new NotificationChannel(ALEXA_MEDIA_CHANNEL_ID,
                context.getString(R.string.notification_channel), NotificationManager.IMPORTANCE_DEFAULT);

        channel.setDescription(context.getString(R.string.notification_channel_description));

        notificationManager.createNotificationChannel(channel);
    }
}
