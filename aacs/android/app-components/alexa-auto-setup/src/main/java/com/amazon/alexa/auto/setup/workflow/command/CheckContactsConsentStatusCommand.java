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

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

/**
 * Command to check if user has consented to upload contacts to Alexa.
 */
public class CheckContactsConsentStatusCommand extends Command {
    private static final String TAG = CheckContactsConsentStatusCommand.class.getSimpleName();

    private final Context mContext;
    private final AuthController authController;

    public CheckContactsConsentStatusCommand(Context context) {
        super(context);
        AlexaApp app = AlexaApp.from(context);
        mContext = context;
        authController = app.getRootComponent().getAuthController();
    }

    @Override
    public void execute() {
        observeContactsConsentStatus();
    }

    private void observeContactsConsentStatus() {
        if (!authController.getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)
                && !ModuleProvider.isAlexaCustomAssistantEnabled(mContext)) {
            Log.d(TAG,
                    "Auth mode is not CBL and Alexa Custom Assistant is not enabled, skipping contacts consent step.");
            publishEvent(new WorkflowMessage("Contacts_Consent_Setup_Skipped"));
            return;
        }

        AlexaApp app = AlexaApp.from(mContext);
        if (app.getRootComponent().getComponent(ContactsController.class).isPresent()) {
            app.getRootComponent().getComponent(ContactsController.class).ifPresent(contactsController -> {
                contactsController.observeContactsConsent().subscribe(isContactsConsentNeeded -> {
                    if (isContactsConsentNeeded) {
                        Log.d(TAG, "Showing contacts consent screen.");
                        publishEvent(new WorkflowMessage("Contacts_Consent_Setup_Not_Finished"));
                    } else {
                        publishEvent(new WorkflowMessage("Contacts_Consent_Setup_Finished"));
                    }
                });
            });
        } else {
            Log.d(TAG, "Skipping contacts consent step.");
            EventBus.getDefault().post(new WorkflowMessage("Contacts_Consent_Setup_Skipped"));
        }
    }
}
