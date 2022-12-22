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
package com.amazon.alexaautoclientservice.util

import android.annotation.TargetApi
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.core.app.NotificationCompat
import com.amazon.alexaautoclientservice.R


class GeneralNotificationBuilder {
    fun buildNotification(
            context: Context, notificationManager: NotificationManager): Notification {
        createNotificationChannel(context, notificationManager)
        val notificationBuilder: NotificationCompat.Builder = NotificationCompat.Builder(context, NOTIFICATION_CHANNEL_ID)
                .setSmallIcon(R.drawable.alexa_bubble_small)
                .setBadgeIconType(NotificationCompat.BADGE_ICON_SMALL)
                .setContentTitle(context.getString(R.string.general_notification_title))
                .setContentText(context.getString(R.string.general_notification_text))
                .setNumber(1)
                .setShowWhen(true)
                .setWhen(System.currentTimeMillis())
                .setStyle(NotificationCompat.BigTextStyle());
        return notificationBuilder.build()
    }

    @TargetApi(Build.VERSION_CODES.O)
    private fun createNotificationChannel(
            context: Context, notificationManager: NotificationManager) {
        val channel = NotificationChannel(NOTIFICATION_CHANNEL_ID,
                context.getString(R.string.general_notification_title), NotificationManager.IMPORTANCE_DEFAULT)
        channel.description = context.getString(R.string.general_notification_title)
        notificationManager.createNotificationChannel(channel)
    }

    companion object {
        const val NOTIFICATION_CHANNEL_ID = "com.amazon.alexaautoclientservice.util.GeneralNotificationBuilder.kt"
        const val NOTIFICATION_ID = 0x101
    }
}
