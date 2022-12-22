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
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;

import java.lang.ref.WeakReference;

/**
 * Command that checks auth mode and publishes event accordingly which then navigates user
 * to appropriate screen
 */
public class SetupCompleteCommand extends Command {
    private static final String TAG = SetupCompleteCommand.class.getSimpleName();
    AlexaSetupController mAlexaSetupController;
    LoginUIEventListener mUIEventListener;


    public SetupCompleteCommand(@NonNull Context context) {
        super(context);
        AlexaApp app = AlexaApp.from(context);
        mAlexaSetupController = app.getRootComponent().getAlexaSetupController();
        mUIEventListener = app.getRootComponent().getComponent(LoginUIEventListener.class).orElse(null);
    }

    @VisibleForTesting
    SetupCompleteCommand(Context context, AlexaSetupController alexaSetupController, LoginUIEventListener loginUIEventListener) {
        super(context);
        this.mAlexaSetupController = alexaSetupController;
        this.mUIEventListener = loginUIEventListener;
    }

    @Override
    public void execute() {
        Log.d(TAG, "Setup completed, forwarding to landing page...");
        mAlexaSetupController.setSetupCompleteStatus(true);
        new AACSMessageSender(new WeakReference<>(getContext()), new AACSSender())
                .sendMessage(Topic.DEVICE_SETUP, Action.DeviceSetup.SETUP_COMPLETED, "");
        if (mUIEventListener != null) {
            mUIEventListener.loginFinished();
        }
    }
}
