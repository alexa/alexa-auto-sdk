/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Alerts;

import android.app.Activity;
import android.view.View;
import android.widget.TextView;

import com.amazon.aace.alexa.Alerts;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class AlertsHandler extends Alerts {
    private static final String sTag = "Alerts";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private TextView mStateText;

    public AlertsHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        setupGUI();
    }

    @Override
    public void alertStateChanged(final String alertToken, final AlertState state, final String reason) {
        mLogger.postInfo(sTag,
                String.format("Alert State Changed. STATE: %s, REASON: %s, TOKEN: %s", state, reason, alertToken));
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mStateText.setText(state != null ? state.toString() : "");
            }
        });
    }

    @Override
    public void alertCreated(String alertToken, String detailedInfo) {
        mLogger.postInfo(
                sTag, String.format("Alert Created. TOKEN: %s, Detailed Info payload: %s", alertToken, detailedInfo));
    }

    @Override
    public void alertDeleted(String alertToken) {
        mLogger.postInfo(sTag, String.format("Alert Deleted. TOKEN: %s", alertToken));
    }

    private void onLocalStop() {
        mLogger.postInfo(sTag, "Stopping active alert");
        super.localStop();
    }

    private void onRemoveAllAlerts() {
        mLogger.postInfo(sTag, "Removing all pending alerts from storage");
        super.removeAllAlerts();
    }

    private void setupGUI() {
        mStateText = mActivity.findViewById(R.id.alertState);

        mActivity.findViewById(R.id.stopAlertButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLocalStop();
            }
        });
    }
}
