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
package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;
import android.preference.PreferenceManager;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestriction;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionStatus;
import com.amazon.alexa.auto.apis.uxRestrictions.CarUxRestrictionsController;
import com.amazon.alexa.auto.apps.common.Constants;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

public class CheckDrivingStatusCommand extends Command {
    private static final String TAG = CheckDrivingStatusCommand.class.getSimpleName();

    /**
     * Command to check driving state status and publish the workflow event based on the status.
     */
    public CheckDrivingStatusCommand(Context context) {
        super(context);
    }

    @Override
    public void execute() {
        if (!AlexaApp.from(getContext())
                        .getRootComponent()
                        .getComponent(CarUxRestrictionsController.class)
                        .isPresent()) {
            Log.d(TAG, "Skipping over driving state command, as CarUxRestrictionsController impl was not detected...");
            EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CAR_STATE_PARK_EVENT));
            return;
        }
        AlexaApp.from(getContext())
                .getRootComponent()
                .getComponent(CarUxRestrictionsController.class)
                .ifPresent(carUxRestrictionsController -> {
                    CarUxRestriction carUxRestriction = carUxRestrictionsController.getActiveCarUxRestriction();
                    String uxRestrictionName = carUxRestriction.getUxRestrictionName();
                    switch (uxRestrictionName) {
                        case Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_MOVING:
                            EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CAR_STATE_DRIVE_EVENT));
                            break;
                        case Constants.CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_PARKED:
                            EventBus.getDefault().post(new WorkflowMessage(LoginEvent.CAR_STATE_PARK_EVENT));
                            break;
                        default:
                            Log.e(TAG,
                                    uxRestrictionName
                                            + " is not a supported driving state recognized by the setup module.");
                            break;
                    }
                });
    }
}
