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

package com.amazon.sampleapp.impl.DoNotDisturb;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.alexa.AlexaClient.AuthError;
import com.amazon.aace.alexa.AlexaClient.AuthState;
import com.amazon.aace.alexa.DoNotDisturb;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.AlexaClient.AuthStateObserver;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class DoNotDisturbHandler extends DoNotDisturb implements AuthStateObserver {
    private static final String sTag = "DoNotDisturb";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private SwitchCompat mDoNotDisturbSwitch;
    private final SharedPreferences mPreferences;
    private final CompoundButton.OnCheckedChangeListener mCheckedChangeListener;

    public DoNotDisturbHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mPreferences =
                activity.getSharedPreferences(activity.getString(R.string.preference_file_key), Context.MODE_PRIVATE);

        mCheckedChangeListener = new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (doNotDisturbChanged(isChecked)) {
                    SharedPreferences.Editor editor = mPreferences.edit();
                    editor.putBoolean(mActivity.getString(R.string.preference_do_not_disturb), isChecked);
                    editor.apply();
                    mLogger.postInfo(sTag, String.format("doNotDisturbChanged ACTIVE: %s", isChecked));
                } else {
                    mDoNotDisturbSwitch.setChecked(!isChecked);
                    mLogger.postError(sTag, String.format("doNotDisturbChanged Failed"));
                }
            }
        };

        setupGUI();
    }

    @Override
    public void onAuthStateChanged(AuthState authState, AuthError authError) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (authState == AuthState.REFRESHED) {
                    // enable listener and toggle
                    mDoNotDisturbSwitch.setOnCheckedChangeListener(mCheckedChangeListener);
                    mDoNotDisturbSwitch.setEnabled(true);
                } else {
                    // disable toggle when not connected
                    mDoNotDisturbSwitch.setEnabled(false);
                }
            }
        });
    }

    @Override
    public void setDoNotDisturb(final boolean doNotDisturb) {
        mLogger.postInfo(sTag, String.format("setDoNotDisturb ACTIVE: %s", doNotDisturb));
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // do not trigger check change listener
                mDoNotDisturbSwitch.setOnCheckedChangeListener(null);
                mDoNotDisturbSwitch.setChecked(doNotDisturb);
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.putBoolean(mActivity.getString(R.string.preference_do_not_disturb), doNotDisturb);
                editor.apply();
                mDoNotDisturbSwitch.setOnCheckedChangeListener(mCheckedChangeListener);
            }
        });
    }

    private void setupGUI() {
        View switchItem = mActivity.findViewById(R.id.toggle_do_not_disturb);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.do_not_disturb_switch);
        mDoNotDisturbSwitch = switchItem.findViewById(R.id.drawerSwitch);
        // init value
        mDoNotDisturbSwitch.setChecked(
                mPreferences.getBoolean(mActivity.getString(R.string.preference_do_not_disturb), false));
    }
}