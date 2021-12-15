/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.alexaautoclientservice.modules.alexaClient.AlexaClientMessageHandler;
import com.amazon.alexaautoclientservice.util.AACSStateObserver;

// Class replies to a ping intent
public class PingReceiver extends BroadcastReceiver implements AACSStateObserver {
    private final String TAG = AACSConstants.AACS + "-" + PingReceiver.class.getSimpleName();

    private String mState = "";

    public PingReceiver() {
        super();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "Ping received, replying to intent " + intent);
        Intent reply = new Intent();
        String currentState;
        if (AASBConstants.AlexaClient.ALEXA_CLIENT_STATUS_CONNECTED.equals(
                    AlexaClientMessageHandler.getCurrentConnectionState())) {
            currentState = AASBConstants.AlexaClient.ALEXA_CLIENT_STATUS_CONNECTED;
        } else {
            currentState = mState;
        }

        if (intent.hasExtra("replyToPackage") && intent.hasExtra("replyToClass")) {
            String pkg = intent.getStringExtra("replyToPackage");
            String cls = intent.getStringExtra("replyToClass");
            reply.setComponent(new ComponentName(pkg, cls));
        }

        reply.setAction("com.amazon.aacs.pingreply");
        reply.putExtra("state", currentState);

        if (intent.hasExtra("replyType")) {
            String type = intent.getStringExtra("replyType");
            switch (type) {
                case "SERVICE":
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                        context.startForegroundService(reply);
                    } else {
                        context.startService(reply);
                    }
                    break;
                case "ACTIVITY":
                    context.startActivity(reply);
                    break;
                case "RECEIVER":
                    context.sendBroadcast(reply);
                    break;
            }
        }
    }

    @Override
    public void onAACSStateChanged(@NonNull String state) {
        mState = state;
    }
}
