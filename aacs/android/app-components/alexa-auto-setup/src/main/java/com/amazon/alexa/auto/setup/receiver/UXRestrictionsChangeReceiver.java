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
package com.amazon.alexa.auto.setup.receiver;

import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.ModuleName.LVC;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.Constants;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

public class UXRestrictionsChangeReceiver extends BroadcastReceiver {
    private static final String TAG = UXRestrictionsChangeReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        if (AlexaApp.from(context).getRootComponent().getAlexaSetupController().isSetupCompleted()
                || (ModuleProvider.containsModule(context, LVC)
                        && AlexaApp.from(context).getRootComponent().getAuthController().isAuthenticated())) {
            return;
        }

        String drivingStateValue = intent.getStringExtra(Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_ACTION_EXTRA_KEY);
        Log.d(TAG, "Received driving state: " + drivingStateValue);
        switch (drivingStateValue) {
            case Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_MOVING:
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CAR_STATE_DRIVE_EVENT));
                break;
            case Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_PARKED:
                EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CAR_STATE_PARK_EVENT));
                break;
            default:
                Log.e(TAG, drivingStateValue + " is not a supported driving state recognized by the setup module.");
                break;
        }
    }
}