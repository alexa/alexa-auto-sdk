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
package com.amazon.alexa.auto.setup.workflow;

import android.content.Context;

import androidx.annotation.Nullable;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;

import javax.inject.Inject;

public class AlexaSetupWorkflowControllerImpl implements AlexaSetupWorkflowController {
    @Inject
    WorkflowNavigator mWorkflowNavigator;

    public AlexaSetupWorkflowControllerImpl(Context context) {
        DaggerSetupComponent.builder().androidModule(new AndroidModule(context)).build().injectWorkflowController(this);
    }

    @Override
    public void startSetupWorkflow(Context context, NavController navController, @Nullable String startStepOverride) {
        String extraModules = ModuleProvider.getModules(context);

        mWorkflowNavigator.startNavigation(navController, extraModules, startStepOverride);
    }

    @Override
    public void stopSetupWorkflow() {
        mWorkflowNavigator.stopNavigation();
    }
}
