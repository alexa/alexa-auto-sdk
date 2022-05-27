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
package com.amazon.alexaautoclientservice;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSReceiver;
import com.amazon.alexaautoclientservice.AlexaAutoClientService.AACSStateMachine;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONException;
import org.json.JSONObject;

public class ConfigMessageReceivedCallback implements AACSReceiver.MessageReceivedCallback {
    private static final String TAG = AACSConstants.AACS + "-" + ConfigMessageReceivedCallback.class.getSimpleName();
    private Context mContext;
    private AACSStateMachine mStateMachine;

    public ConfigMessageReceivedCallback(@NonNull Context context, @NonNull AACSStateMachine aacsStateMachine) {
        mContext = context;
        mStateMachine = aacsStateMachine;
    }

    @Override
    public void onMessageReceived(String s) {
        Log.i(TAG, "Configuration message received.");
        if (mStateMachine.getState() == AACSConstants.State.STARTED) {
            try {
                JSONObject configMessage = new JSONObject(s);
                boolean configSaved = FileUtil.setConfiguration(mContext, configMessage.getJSONArray("configFilepaths"),
                        configMessage.getJSONArray("configStrings"));

                if (configSaved) {
                    mStateMachine.setState(AACSConstants.State.CONFIGURED);
                } else {
                    Log.e(TAG, "Unable to save configuration after it was received.");
                }

            } catch (JSONException e) {
                Log.d(TAG,
                        String.format(
                                "Error constructing configuration message JSON object. Error: %s", e.getMessage()));
            }
        } else {
            Log.e(TAG, "Configuration message should only be sent alongside the startService intent.");
        }
    }
}
