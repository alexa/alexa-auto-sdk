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

package com.amazon.sampleapp.impl.Notifications;

import android.app.Activity;
import android.widget.TextView;

import com.amazon.aace.alexa.Notifications;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class NotificationsHandler extends Notifications {
    private static final String sTag = "Notifications";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private TextView mStateText;

    public NotificationsHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mStateText = mActivity.findViewById(R.id.indicatorState);
    }

    @Override
    public void setIndicator(final IndicatorState state) {
        mLogger.postInfo(sTag, "Indicator State: " + state);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mStateText.setText(state != null ? state.toString() : "");
            }
        });
    }
}
