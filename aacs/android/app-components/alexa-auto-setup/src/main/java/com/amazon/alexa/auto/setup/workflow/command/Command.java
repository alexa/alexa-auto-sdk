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

import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

/**
 * Base command class to execute Alexa setup workflow command.
 */
public abstract class Command {
    public static final String CHECK_DEFAULT_ASSIST_APP_COMMAND = "CheckDefaultAssistAppCommand";
    public static final String CHECK_DRIVING_STATUS_COMMAND = "CheckDrivingStatusCommand";
    public static final String CHECK_NETWORK_STATUS_COMMAND = "CheckNetworkStatusCommand";
    public static final String CHECK_LANGUAGE_COMMAND = "CheckLanguageCommand";
    public static final String CHECK_CONTACTS_CONSENT_STATUS_COMMAND = "CheckContactsConsentStatusCommand";
    public static final String CHECK_LOCATION_CONSENT_COMMAND = "CheckLocationConsentCommand";
    public static final String CHECK_LOGIN_REQUIRED_COMMAND = "CheckLoginRequiredCommand";
    public static final String SETUP_COMPLETE_COMMAND = "SetupCompleteCommand";
    public static final String NAVIGATION_FAVORITE_COMMAND = "CheckNaviFavoriteCommand";
    public static final String CHECK_SEPARATE_ADDRESSBOOK_CONSENT_COMMAND = "CheckSeparateAddressBookConsentCommand";

    private final Context mContext;

    /**
     * Constructor.
     */
    public Command(Context context) {
        mContext = context;
    }

    public Context getContext() {
        return mContext;
    }

    /**
     * Publish event based on the workflow message.
     * @param message workflow message.
     */
    public void publishEvent(WorkflowMessage message) {
        EventBus.getDefault().post(message);
    }

    /**
     * Perform the action of the command.
     */
    public abstract void execute();
}