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
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.navigation.NavController;
import androidx.navigation.NavDestination;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.workflow.command.CheckContactsConsentStatusCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckDefaultAssistAppCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckDrivingStatusCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLanguageCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLocationConsentCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLoginRequiredCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckNaviFavoriteCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckNetworkStatusCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckSeparateAddressBookConsentCommand;
import com.amazon.alexa.auto.setup.workflow.command.Command;
import com.amazon.alexa.auto.setup.workflow.command.SetupCompleteCommand;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;
import java.util.Optional;

/**
 * Alexa setup workflow navigator, it helps starting Alexa setup navigation
 * and navigating to next workflow step with workflow event.
 */
public class WorkflowNavigator {
    private static final String TAG = WorkflowNavigator.class.getSimpleName();

    private final WorkflowProvider mWorkflowProvider;
    private final WeakReference<Context> mContext;

    private NavController mNavController;
    private String mExtraModules;

    @VisibleForTesting
    Bundle mSupportedFeatures;

    /**
     * Constructor.
     * @param context Android Context.
     * @param workflowProvider Setup workflow provider.
     */
    public WorkflowNavigator(WeakReference<Context> context, WorkflowProvider workflowProvider) {
        mWorkflowProvider = workflowProvider;
        mContext = context;
    }

    /**
     * Start Alexa setup workflow navigation.
     * @param navController Android navigation controller.
     * @param extraModules Extra modules that are supported in Auto SDK.
     */
    public void startNavigation(NavController navController, String extraModules, @Nullable String startStepOverride) {
        Log.d(TAG, "Start Alexa setup workflow navigation.");
        mNavController = navController;
        mExtraModules = extraModules;
        mSupportedFeatures = new Bundle();
        mSupportedFeatures.putString(ModuleProvider.MODULES, mExtraModules);

        mWorkflowProvider.readWorkflowSpecificationAsync(extraModules)
                .filter(Optional::isPresent)
                .map(Optional::get)
                .subscribe((workflow) -> {
                    if (!EventBus.getDefault().isRegistered(this)) {
                        EventBus.getDefault().register(this);
                    }

                    if (startStepOverride != null) {
                        mWorkflowProvider.nextWorkflowStep(startStepOverride).ifPresent(this::navigateToNext);
                    } else {
                        mWorkflowProvider.getWorkflowStartingStep().ifPresent(this::navigateToNext);
                    }
                });
    }

    /**
     * Stop Alexa setup workflow navigation.
     */
    public void stopNavigation() {
        EventBus.getDefault().unregister(this);
    }

    /**
     * Navigate to next workflow step.
     *
     * @param workflowStep Setup workflow step.
     */
    private void navigateToNext(WorkflowStep workflowStep) {
        switch (workflowStep.getType()) {
            case "view":
                Log.d(TAG, "Navigate to " + workflowStep.getResource());
                int navId = mWorkflowProvider.getResourceIdFromWorkflowStep(workflowStep);

                // TODO: Temporary fix  to execute cancelLogin in the auth controller whenever we get interrupted
                NavDestination lastNav = mNavController.getCurrentDestination();
                if (lastNav != null) {
                    if (lastNav.getId() == R.id.navigation_fragment_network
                            || lastNav.getId() == R.id.navigation_fragment_blockSetupDrive) {
                        AuthController authController =
                                AlexaApp.from(mContext.get()).getRootComponent().getAuthController();
                        if (!authController.isAuthenticated())
                            authController.cancelLogin(null);
                    }
                }

                mNavController.navigate(navId, mSupportedFeatures);
                break;
            case "action":
                Log.d(TAG, "Executing command: " + workflowStep.getResource());
                executeCommand(workflowStep.getResource());
                break;
            default:
                Log.d(TAG, "The workflow type is not valid");
        }
    }

    @Subscribe
    public void onSetupWorkflowChange(WorkflowMessage message) {
        Log.d(TAG, "onSetupWorkflowChange: " + message.getWorkflowEvent());
        mWorkflowProvider.nextWorkflowStep(message.getWorkflowEvent()).ifPresent(this::navigateToNext);
    }

    /**
     * Execute the command based on the workflow step's action item.
     * @param action workflow step's action.
     */
    private void executeCommand(String action) {
        Command command = null;
        switch (action) {
            case Command.CHECK_DEFAULT_ASSIST_APP_COMMAND:
                command = new CheckDefaultAssistAppCommand(mContext.get());
                break;
            case Command.CHECK_DRIVING_STATUS_COMMAND:
                command = new CheckDrivingStatusCommand(mContext.get());
                break;
            case Command.CHECK_NETWORK_STATUS_COMMAND:
                command = new CheckNetworkStatusCommand(mContext.get());
                break;
            case Command.CHECK_LANGUAGE_COMMAND:
                command = new CheckLanguageCommand(mContext.get());
                break;
            case Command.CHECK_LOGIN_REQUIRED_COMMAND:
                command = new CheckLoginRequiredCommand(mContext.get());
                break;
            case Command.CHECK_CONTACTS_CONSENT_STATUS_COMMAND:
                command = new CheckContactsConsentStatusCommand(mContext.get());
                break;
            case Command.CHECK_LOCATION_CONSENT_COMMAND:
                command = new CheckLocationConsentCommand(mContext.get());
                break;
            case Command.SETUP_COMPLETE_COMMAND:
                command = new SetupCompleteCommand(mContext.get());
                break;
            case Command.NAVIGATION_FAVORITE_COMMAND:
                command = new CheckNaviFavoriteCommand(mContext.get());
                break;
            case Command.CHECK_SEPARATE_ADDRESSBOOK_CONSENT_COMMAND:
                command = new CheckSeparateAddressBookConsentCommand(mContext.get());
                break;
            default:
                break;
        }
        if (command != null) {
            command.execute();
        }
    }
}
