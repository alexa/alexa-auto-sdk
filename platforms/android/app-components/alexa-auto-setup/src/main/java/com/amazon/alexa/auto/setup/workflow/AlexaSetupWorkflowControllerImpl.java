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
