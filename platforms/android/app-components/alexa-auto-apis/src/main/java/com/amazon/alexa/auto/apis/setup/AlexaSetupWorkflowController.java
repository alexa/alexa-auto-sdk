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
