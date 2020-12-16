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

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.AlexaAutoClientService.AACSStateMachine;
import com.amazon.alexaautoclientservice.AlexaAutoClientService.State;
import com.amazon.alexaautoclientservice.aacs_extra.AACSContext;
import com.amazon.alexaautoclientservice.constants.LVCServiceConstants;

import java.util.HashMap;

public class LVCReceiver extends BroadcastReceiver {
    private final String TAG = AACSConstants.AACS + "-" + LVCReceiver.class.getSimpleName();

    private AACSStateMachine mStateMachine;
    private HashMap<String, String> mAACSContextMap;

    public LVCReceiver(@NonNull AACSStateMachine aacsStateMachine, @NonNull HashMap<String, String> contextMap) {
        super();
        mStateMachine = aacsStateMachine;
        mAACSContextMap = contextMap;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (LVCServiceConstants.LVC_RECEIVER_INTENT.equals(intent.getAction())) {
            Log.i(TAG, "onReceive LVC Config message.");
            if (intent.hasExtra(LVCServiceConstants.LVC_RECEIVER_FAILURE_REASON)) {
                // LVCInteractionService was unable to provide config from LVC
                String reason = intent.getStringExtra(LVCServiceConstants.LVC_RECEIVER_FAILURE_REASON);
                Log.e(TAG, "Failed to init LVC: " + reason);
            } else if (intent.hasExtra(LVCServiceConstants.LVC_RECEIVER_CONFIGURATION)) {
                // LVCInteractionService received config from LVC
                Log.i(TAG, "Received LVC config. Adding to AACSContext.");
                String config = intent.getStringExtra(LVCServiceConstants.LVC_RECEIVER_CONFIGURATION);
                mAACSContextMap.put(AACSContext.JSON, config);
                mStateMachine.setState(State.CONFIGURED);
            }
        }
    }
}
