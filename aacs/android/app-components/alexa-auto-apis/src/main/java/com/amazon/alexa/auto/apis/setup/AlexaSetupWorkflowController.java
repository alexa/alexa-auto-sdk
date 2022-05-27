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
package com.amazon.alexa.auto.apis.setup;

import android.content.Context;

import androidx.annotation.Nullable;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to make progress in Alexa setup.
 */
public interface AlexaSetupWorkflowController extends ScopedComponent {
    /**
     * Start Alexa setup workflow with Android navigation controller
     * If startStepOverride is null, then use default start step from the workflow specification file.
     */
    void startSetupWorkflow(Context context, NavController navController, @Nullable String startStepOverride);

    /**
     * Stop Alexa setup workflow with Android navigation controller
     */
    void stopSetupWorkflow();
}
