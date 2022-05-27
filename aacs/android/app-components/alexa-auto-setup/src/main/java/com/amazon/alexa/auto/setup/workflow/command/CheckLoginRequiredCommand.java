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

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

/**
 * Command to check if login is required. This way the user doesn't need to go through login again
 * if there is an error in another step
 */
public class CheckLoginRequiredCommand extends Command {
    private final AuthController authController;

    public CheckLoginRequiredCommand(Context context) {
        super(context);
        AlexaApp app = AlexaApp.from(context);
        authController = app.getRootComponent().getAuthController();
    }

    @Override
    public void execute() {
        if (authController.isAuthenticated()) {
            if (authController.getAuthMode() == AuthMode.AUTH_PROVIDER_AUTHORIZATION) {
                publishEvent(new WorkflowMessage(LoginEvent.PREVIEW_MODE_ENABLED));
            } else if (authController.getAuthMode() == AuthMode.CBL_AUTHORIZATION) {
                publishEvent(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
            }
        } else {
            publishEvent(new WorkflowMessage(LoginEvent.LOGIN));
        }
    }
}
