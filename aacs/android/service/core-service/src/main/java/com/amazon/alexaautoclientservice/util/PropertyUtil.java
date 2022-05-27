/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.util;

import android.app.AlarmManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aacsconstants.AACSConstants;

import java.util.TimeZone;

/**
 * Util class for Alexa/System properties.
 */
public class PropertyUtil {
    private static final String TAG = AACSConstants.AACS + "-" + PropertyUtil.class.getSimpleName();

    public static void updateSystemProperty(@NonNull Context context, @NonNull String key, @NonNull String newValue) {
        Log.v(TAG, "updateSystemProperty");
        if (AlexaProperties.TIMEZONE.equals(key)) {
            updateSystemTimeZone(context, newValue);
        }
    }

    private static void updateSystemTimeZone(@NonNull Context context, @NonNull String newTimeZone) {
        String systemTimeZone = TimeZone.getDefault().getID();
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        if (!systemTimeZone.equals(newTimeZone)) {
            PackageManager pm = context.getPackageManager();
            ApplicationInfo ai;
            try {
                ai = pm.getApplicationInfo(context.getPackageName(), 0);
            } catch (PackageManager.NameNotFoundException e) {
                Log.e(TAG,
                        String.format("Package with name %s not found by Package Manager", context.getPackageName()));
                return;
            }

            if (ai == null) {
                Log.e(TAG, "Couldn't get application info!");
                return;
            }

            if ((ai.flags & ApplicationInfo.FLAG_SYSTEM) != 0) {
                alarmManager.setTimeZone(newTimeZone);
                Log.v(TAG, "Successfully updated system time zone to " + newTimeZone);
            } else {
                Log.w(TAG, "System time zone is not updated as the app does not have system privilege.");
            }
        }
    }
}
