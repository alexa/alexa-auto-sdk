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

package com.amazon.alexa.auto.media;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.amazon.alexa.auto.media.session.MediaSessionManager;

import java.lang.ref.WeakReference;

public class ShutdownActionReceiver extends BroadcastReceiver {
    private static final String TAG = "ShutdownActionReceiver";
    private MediaSessionManager mMediaSessionManager;

    private WeakReference<Context> mContext;

    private IntentFilter mIntentFilter;

    public ShutdownActionReceiver(WeakReference<Context> context) {
        mContext = context;
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_SHUTDOWN);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent != null && intent.getAction().equalsIgnoreCase(Intent.ACTION_SHUTDOWN)) {
            if (mMediaSessionManager != null) {
                mMediaSessionManager.shutdown();
            } else {
                Log.e(TAG, "MediaSessionManager is null");
            }
        }
    }

    public void onCreate(MediaSessionManager manager) {
        mMediaSessionManager = manager;
        mContext.get().registerReceiver(this, mIntentFilter);
    }

    public void onDestroy() {
        mContext.get().unregisterReceiver(this);
    }
}
