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

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

/**
 * Command that checks auth mode and publishes event accordingly which then navigates user
 * to appropriate screen
 */
public class SetupCompleteCommand extends Command {
    private static final String TAG = SetupCompleteCommand.class.getSimpleName();

    AuthController mAuthController;

    public SetupCompleteCommand(@NonNull Context context) {
        super(context);
        AlexaApp app = AlexaApp.from(context);
        mAuthController = app.getRootComponent().getAuthController();
    }

    @VisibleForTesting
    SetupCompleteCommand(Context context, AuthController mAuthController) {
        super(context);
        this.mAuthController = mAuthController;
    }

    @Override
    public void execute() {
        AuthMode mode = mAuthController.getAuthMode();
        if (mode.equals(AuthMode.CBL_AUTHORIZATION)) {
            publishEvent(new WorkflowMessage(LoginEvent.CBL_FLOW_SETUP_COMPLETED));
        } else if (mode.equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            publishEvent(new WorkflowMessage(LoginEvent.PREVIEW_MODE_FLOW_SETUP_COMPLETED));
        }
    }
}
