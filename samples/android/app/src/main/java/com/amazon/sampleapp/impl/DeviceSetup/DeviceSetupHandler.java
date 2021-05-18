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

package com.amazon.sampleapp.impl.DeviceSetup;

import android.app.Activity;
import android.view.View;
import android.widget.Toast;

import com.amazon.aace.alexa.DeviceSetup;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class DeviceSetupHandler extends DeviceSetup {
    private static final String sTag = DeviceSetupHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;

    public DeviceSetupHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        setupGUI();
    }

    @Override
    public void setupCompletedResponse(final StatusCode statusCode) {
        mLogger.postInfo(sTag, "setupCompletedResponse " + statusCode.toString());
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(mActivity, statusCode.toString(), Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void setupGUI() {
        View logoutView = mActivity.findViewById(R.id.cblLogout);
        logoutView.findViewById(R.id.sendSetupCompleted).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mLogger.postInfo(sTag, "Sending setupCompleted");
                setupCompleted();
            }
        });
    }
}
