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

import com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

public class CheckNaviFavoriteCommand extends Command {
    private static final String TAG = CheckNaviFavoriteCommand.class.getSimpleName();

    public CheckNaviFavoriteCommand(Context context) {
        super(context);

        DaggerSetupComponent.builder()
                .androidModule(new AndroidModule(context))
                .build()
                .injectCheckNaviFavoriteCommand(this);
    }

    @Override
    public void execute() {
        if (NaviFavoritesSettingsProvider.isNavFavoritesEnabled(getContext()))
            publishEvent(new WorkflowMessage(LoginEvent.NAVI_FAVORITES_CONSENT_COMPLETED));
        else
            publishEvent(new WorkflowMessage(LoginEvent.NAVI_FAVORITES_CONSENT_NOT_COMPLETED));
    }
}
