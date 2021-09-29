package com.amazon.alexa.auto.setup.workflow;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.navigation.NavController;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.workflow.command.CheckContactsConsentStatusCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLanguageCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLoginRequiredCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckNetworkStatusCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLocationConsentCommand;
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

                mNavController.navigate(navId, mSupportedFeatures);
                break;
            case "action":
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
            default:
                break;
        }
        if (command != null) {
            command.execute();
        }
    }
}
