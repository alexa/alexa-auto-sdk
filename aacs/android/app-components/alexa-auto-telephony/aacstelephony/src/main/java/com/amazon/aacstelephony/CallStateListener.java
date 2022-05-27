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

package com.amazon.aacstelephony;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.telecom.Call;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.TelephonyConstants;
import com.amazon.aacsipc.IPCUtils;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

public class CallStateListener extends Call.Callback {
    private static final String TAG = AACSConstants.AACS + "-" + CallStateListener.class.getSimpleName();
    private Context mContext;
    private String mCallId;
    private String mCallerId;
    private AACSMessageSender mAACSMessageSender;
    private String mPackageName;
    private ComponentName mTelephonyServiceComponent;

    public CallStateListener(@NonNull Context context, @NonNull String callId, @NonNull String callerId,
            @NonNull AACSMessageSender messageSender) {
        mContext = context;
        mCallId = callId;
        mCallerId = callerId;
        mAACSMessageSender = messageSender;
        mPackageName = mContext.getApplicationContext().getPackageName();
        Log.d(TAG, "AACS Telephony lib is in " + mPackageName);
        mTelephonyServiceComponent = new ComponentName(mPackageName, Constants.TELEPHONY_SERVICE_CLASS);
    }

    @Override
    public void onStateChanged(Call call, int state) {
        super.onStateChanged(call, state);

        Log.v(TAG, String.format("onStateChanged: callId=%s, state=%s", mCallId, Util.getAACSCallState(state)));
        Util.publishCallStateToAACS(state, mAACSMessageSender, mCallId, mCallerId);

        if (state == Call.STATE_ACTIVE) {
            notifyService(Constants.ACTION_CANCEL_IDLE_TIMER);
        } else if (state == Call.STATE_DISCONNECTED) {
            notifyService(Constants.ACTION_RESET_IDLE_TIMER);
        }
    }

    private void notifyService(String action) {
        // notify AACS Telephony Service to mark itself busy on call state ACTIVE
        // mark itself idle on call state IDLE
        Intent intent = new Intent(action);
        intent.addCategory(TelephonyConstants.CATEGORY_AACS_TELEPHONY);

        intent.setComponent(mTelephonyServiceComponent);
        intent.setPackage(mPackageName);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && !IPCUtils.getInstance(mContext).isSystemApp()) {
            mContext.startForegroundService(intent);
        } else {
            mContext.startService(intent);
        }
    }
}
