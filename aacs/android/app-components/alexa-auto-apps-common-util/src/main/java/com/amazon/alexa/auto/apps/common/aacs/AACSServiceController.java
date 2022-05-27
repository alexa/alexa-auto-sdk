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
package com.amazon.alexa.auto.apps.common.aacs;

import static com.amazon.alexa.auto.apps.common.Constants.AACS_SAMPLE_APP_FILE_PROVIDER;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.content.FileProvider;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.IPCUtils;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Service Controller for AACS.
 */
public class AACSServiceController {
    private static final String TAG = AACSServiceController.class.getSimpleName();

    /**
     * Start Alexa Auto Client Service.
     *
     * @param context Android Context to start the service.
     * @param waitForNewConfig Signal AACS to use new configuration, which will
     *                         be sent after service start.
     */
    public static void startAACS(@NonNull Context context, boolean waitForNewConfig) {
        Log.i(TAG, "Sending start signal to Alexa Client Service.");
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(
                AACSConstants.getAACSPackageName(new WeakReference<Context>(context)), AACSConstants.AACS_CLASS_NAME));
        intent.setAction(Action.LAUNCH_SERVICE);
        intent.putExtra(AACSConstants.NEW_CONFIG, waitForNewConfig);
        checkAndroidVersionAndStartService(context, intent);
    }

    /**
     * Stop the Alexa Auto Client Service.
     *
     * @param context Android Context to stop the service.
     */
    public static void stopAACS(@NonNull Context context) {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(
                AACSConstants.getAACSPackageName(new WeakReference<Context>(context)), AACSConstants.AACS_CLASS_NAME));
        context.stopService(intent);
    }

    public static void shareFilePermissionsOfSameType(
            @NonNull Context context, File parent, String[] filenames, String module) {
        Log.i(TAG, "shareFilePermissionsOfSameType");
        ArrayList<Uri> fileUris = new ArrayList<>();
        for (String name : filenames) {
            File file = new File(parent, name);
            Uri fileUri = FileProvider.getUriForFile(context, AACS_SAMPLE_APP_FILE_PROVIDER, file);
            context.grantUriPermission(AACSConstants.getAACSPackageName(new WeakReference<Context>(context)), fileUri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION);
            fileUris.add(fileUri);
        }

        Intent shareFileIntent = new Intent();
        shareFileIntent.setComponent(new ComponentName(
                AACSConstants.getAACSPackageName(new WeakReference<Context>(context)), AACSConstants.AACS_CLASS_NAME));
        shareFileIntent.setAction(Intent.ACTION_SEND_MULTIPLE);
        shareFileIntent.setType(context.getContentResolver().getType(fileUris.get(0)));
        shareFileIntent.putExtra(AACSConstants.CONFIG_MODULE, module);
        shareFileIntent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, fileUris);
        checkAndroidVersionAndStartService(context, shareFileIntent);
    }

    public static void checkAndroidVersionAndStartService(@NonNull Context context, Intent intent) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O
                && !IPCUtils.getInstance(context).isSystemApp()) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
}
