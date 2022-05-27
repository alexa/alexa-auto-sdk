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
package com.amazon.alexaautoclientservice.util;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.IPCUtils;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;

public class LVCUtil {
    private static final String TAG = AACSConstants.AACS + "-" + LVCUtil.class.getSimpleName();
    private static final String LVC_SERVICE_PACKAGE_NAME = "com.amazon.alexa.auto.lvc";
    private static final String START_LVC_SERVICE_ACTION = LVC_SERVICE_PACKAGE_NAME + ".start";
    private static final String STOP_LVC_SERVICE_ACTION = LVC_SERVICE_PACKAGE_NAME + ".stop";
    private static final String LVC_SERVICE_CLASS = LVC_SERVICE_PACKAGE_NAME + ".LocalVoiceControlService";
    private static final String LVC_CONFIGURATION = "lvc.configuration";
    private static final String PERSISTENT_SYSTEM_SERVICE = "persistentSystemService";
    private static String AACSLVCConfig;

    public static void startLVCService(@NonNull Context context) {
        // aacs.localVoiceControl in AACS config
        if (AACSLVCConfig == null)
            AACSLVCConfig = FileUtil.getLVCConfiguration();

        boolean persistentSystemService = FileUtil.isEnabledInAACSGeneralConfig("persistentSystemService");

        // Start LocalVoiceControlService intent
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(context.getApplicationContext().getPackageName(), LVC_SERVICE_CLASS));
        intent.setAction(START_LVC_SERVICE_ACTION);
        intent.putExtra(LVC_CONFIGURATION, AACSLVCConfig);
        intent.putExtra(PERSISTENT_SYSTEM_SERVICE, persistentSystemService);

        checkAndStartService(context, intent);
    }

    public static void stopLVCService(@NonNull Context context) {
        // Stop LocalVoiceControlService Intent
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(context.getApplicationContext().getPackageName(), LVC_SERVICE_CLASS));
        intent.setAction(STOP_LVC_SERVICE_ACTION);

        checkAndStartService(context, intent);
    }

    public static boolean allowAACSToControlLVC() {
        if (AACSLVCConfig == null)
            AACSLVCConfig = FileUtil.getLVCConfiguration();

        if (AACSLVCConfig == null)
            return true;

        try {
            return new JSONObject(AACSLVCConfig).optBoolean("allowAACSToControlLVC", true);
        } catch (JSONException ex) {
            Log.e(TAG, "Failed to parse AACS LVC config. Error: " + ex.getMessage());
        }

        return true;
    }

    private static void checkAndStartService(@NonNull Context context, @NonNull Intent intent) {
        Log.d(TAG, "checkAndStartService");
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O
                && !IPCUtils.getInstance(context).isSystemApp()) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
}
