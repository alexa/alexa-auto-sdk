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
package com.amazon.alexaautoclientservice.modules.notification

import android.app.Notification
import android.app.NotificationManager
import android.content.Context
import android.util.Log
import com.amazon.aacsconstants.AACSConstants
import com.amazon.aacsconstants.Action
import com.amazon.alexaautoclientservice.util.GeneralNotificationBuilder
import org.json.JSONObject

class NotificationsMessageHandler(val mContext: Context) {

    private var doNotDisturbFlag: Boolean? = null

    private val mNotificationBuilder = GeneralNotificationBuilder()
    fun handleNotificationMessage(
        messageId: String,
        topic: String,
        action: String,
        payload: String
    ) {
        Log.d(TAG, "handleNotificationMessage - Action: $action, Payload: $payload")
        val notificationManager =
            mContext.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val notification: Notification =
            mNotificationBuilder.buildNotification(mContext, notificationManager)

        if (Action.DoNotDisturb.SET_DO_NOT_DISTURB == action && !parseStateDNDIndicator(payload)) {
            doNotDisturbFlag = false
        }
        if (Action.Notifications.ON_NOTIFICATION_RECEIVED != action && offStateIndicator(
                action,
                payload
            )
        ) {
            notificationManager.cancel(GeneralNotificationBuilder.NOTIFICATION_ID)
        } else if (onStateIndicator(action, payload)) {
            notificationManager.notify(GeneralNotificationBuilder.NOTIFICATION_ID, notification)
        } else if (Action.Notifications.ON_NOTIFICATION_RECEIVED == action) {
            if (doNotDisturbFlag != null && doNotDisturbFlag == false) {
                doNotDisturbFlag = null
            } else {
                notificationManager.cancel(GeneralNotificationBuilder.NOTIFICATION_ID)
                notificationManager.notify(GeneralNotificationBuilder.NOTIFICATION_ID, notification)
            }
        }
    }

    private fun onStateIndicator(action: String, payload: String): Boolean {
        return (Action.Notifications.SET_INDICATOR == action) &&
            (parseStateNotificationIndicator(payload) == INDICATOR_ON)
    }

    private fun offStateIndicator(action: String, payload: String): Boolean {
        return (Action.Notifications.SET_INDICATOR == action) &&
            (parseStateNotificationIndicator(payload) == INDICATOR_OFF)
    }

    private fun parseStateNotificationIndicator(json: String): String {
        return try {
            val obj = JSONObject(json)
            val indicatorState = obj[INDICATOR_STATE].toString()
            indicatorState
        } catch (exception: Exception) {
            Log.e(
                TAG,
                "Failed to parse notification state from aacs message: " +
                    "$json error: $exception"
            )
            INDICATOR_UNKNOWN
        }
    }

    private fun parseStateDNDIndicator(json: String): Boolean {
        return try {
            val obj = JSONObject(json)
            val dndState = obj[DND_INDICATOR] as Boolean
            dndState
        } catch (exception: Exception) {
            Log.e(
                TAG,
                "Failed to parse do not disturb state from aacs message: " +
                    "$json error: $exception"
            )
            false
        }
    }

    companion object {
        private const val INDICATOR_STATE = "state"
        private const val INDICATOR_ON = "ON"
        private const val INDICATOR_OFF = "OFF"
        private const val INDICATOR_UNKNOWN = "UNKNOWN"

        private const val DND_INDICATOR = "doNotDisturb"
        private val TAG =
            AACSConstants.AACS + "-" + NotificationsMessageHandler::class.java.simpleName
    }
}
