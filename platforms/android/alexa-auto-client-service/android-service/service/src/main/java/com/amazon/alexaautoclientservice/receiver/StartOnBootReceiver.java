/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.AlexaAutoClientService;
import com.amazon.alexaautoclientservice.util.FileUtil;

public class StartOnBootReceiver extends BroadcastReceiver {
    private static final String TAG = AACSConstants.AACS + "-" + StartOnBootReceiver.class.getSimpleName();

    public StartOnBootReceiver() {
        super();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) && FileUtil.isStartServiceOnBootEnabled(context)) {
            Log.d(TAG, "Received BOOT_COMPLETED intent.");

            Intent startIntent = new Intent(context, AlexaAutoClientService.class);
            startIntent.putExtra(AACSConstants.NEW_CONFIG, false);
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                context.startForegroundService(startIntent);
            } else {
                context.startService(startIntent);
            }
        }
    }
}
