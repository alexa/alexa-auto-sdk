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
package com.amazon.alexa.auto.apps.common.util;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.provider.Settings;
import android.service.voice.VoiceInteractionService;
import android.util.Log;

import androidx.annotation.NonNull;

import java.util.List;

/**
 * Util class for default assistant setting.
 */
public class DefaultAssistantUtil {
    private static final String TAG = DefaultAssistantUtil.class.getSimpleName();
    private static final String KEY_ASSISTANT = "assistant";
    private static final String KEY_VIS = "voice_interaction_service";
    private static final String KEY_VRS = "voice_recognition_service";

    /**
     * Get the name of the Voice Interaction Service implemented in the Alexa app.
     *
     * @param context Android Context.
     * @return the name of the Voice Interaction Service.
     */
    public static String getAlexaVoiceInteractionServiceName(@NonNull Context context) {
        final List<ResolveInfo> services = context.getPackageManager().queryIntentServices(
                new Intent(VoiceInteractionService.SERVICE_INTERFACE), 0);
        String packageName = "";
        String serviceName = "";
        for (final ResolveInfo info : services) {
            packageName = info.serviceInfo.packageName;
            serviceName = info.serviceInfo.name;
            if (context.getPackageName().equals(packageName)) {
                return serviceName;
            }
        }
        return "";
    }

    /**
     * Set the Alexa app as the Android default assist app.
     *
     * @param context Android Context.
     * @return true if Alexa is set to the default assistant successfully,
     *         or false if the operation fails.
     */
    public static boolean setAlexaAppAsDefault(@NonNull Context context) {
        try {
            String alexaVoiceInteractionServiceName = getAlexaVoiceInteractionServiceName(context);
            int isPermissionGranted = context.checkSelfPermission(Manifest.permission.WRITE_SECURE_SETTINGS);
            if (PackageManager.PERMISSION_GRANTED != isPermissionGranted) {
                Log.e(TAG, "WRITE_SECURE_SETTINGS permission is not granted");
                return false;
            }
            String component = context.getPackageName() + "/" + alexaVoiceInteractionServiceName;
            Settings.Secure.putString(context.getContentResolver(), KEY_ASSISTANT, component);
            Settings.Secure.putString(context.getContentResolver(), KEY_VIS, component);
            Settings.Secure.putString(context.getContentResolver(), KEY_VRS, component);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "failed to set Alexa as default Assist App. Exception: " + e.getMessage());
            return false;
        }
    }

    /**
     * Check whether the Alexa app is set as the Android default assist app.
     *
     * @param context Android Context.
     * @return true if Alexa is selected as the default assistant,
     *         or false if Alexa is not.
     */
    public static boolean isAlexaAppDefaultAssist(@NonNull Context context) {
        String component = Settings.Secure.getString(context.getContentResolver(), KEY_ASSISTANT);
        return component != null && component.contains(context.getPackageName());
    }

    /**
     * Check whether the default assistant is not selected.
     *
     * @param context Android Context.
     * @return true if no default assistant is set, or false if there is an assistant set as default.
     */
    public static boolean isDefaultAssistantNone(@NonNull Context context) {
        String component = Settings.Secure.getString(context.getContentResolver(), KEY_ASSISTANT);
        return component == null || component.isEmpty();
    }

    /**
     * Check whether the assist app selection screen should be skipped.
     * The assist app selection screen is skipped in the following scenarios:
     *  1. The Alexa app is already selected as the default assist app.
     *  2. if no assist app is selected, the Alexa app successfully sets itself
     *     as the default assist app.
     *
     * @param context Android Context.
     * @return true if the screen should be skipped, or false if the screen should be displayed.
     */
    public static boolean shouldSkipAssistAppSelectionScreen(@NonNull Context context) {
        if (isAlexaAppDefaultAssist(context)) {
            Log.i(TAG, "Alexa is already selected as default.");
            return true;
        }
        if (isDefaultAssistantNone(context) && setAlexaAppAsDefault(context)) {
            Log.i(TAG, "No assist app is selected, thus setting Alexa as default directly.");
            return true;
        }
        Log.i(TAG, "the assist app selection screen should be displayed.");
        return false;
    }
}