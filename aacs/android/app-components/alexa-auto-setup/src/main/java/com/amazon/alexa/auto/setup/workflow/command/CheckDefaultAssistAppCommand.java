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

import static com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil.isAlexaAppDefaultAssist;
import static com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil.isDefaultAssistantNone;
import static com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil.setAlexaAppAsDefault;
import static com.amazon.alexa.auto.apps.common.util.DefaultAssistantUtil.shouldSkipAssistAppSelectionScreen;

import android.content.Context;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

/**
 * Command to check whether Alexa is set as the default assist app and publish the workflow event based on the status.
 */
public class CheckDefaultAssistAppCommand extends Command {
    public CheckDefaultAssistAppCommand(Context context) {
        super(context);
    }

    @Override
    public void execute() {
        if (shouldSkipAssistAppSelectionScreen(getContext())) {
            publishEvent(new WorkflowMessage(LoginEvent.ALEXA_IS_SELECTED_EVENT));
        } else {
            publishEvent(new WorkflowMessage(LoginEvent.ALEXA_NOT_SELECTED_EVENT));
        }
    }
}
