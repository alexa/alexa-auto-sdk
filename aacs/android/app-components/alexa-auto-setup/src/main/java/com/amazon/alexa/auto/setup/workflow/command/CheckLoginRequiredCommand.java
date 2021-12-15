package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

/**
 * Command to check if login is required. This way the user doesn't need to go through login again
 * if there is an error in another step
 */
public class CheckLoginRequiredCommand extends Command {

    private final AuthController authController;

    public CheckLoginRequiredCommand(Context context) {
        super(context);
        AlexaApp app = AlexaApp.from(context);
        authController = app.getRootComponent().getAuthController();
    }

    @Override
    public void execute() {
        if (authController.isAuthenticated()) {
            if (authController.getAuthMode() == AuthMode.AUTH_PROVIDER_AUTHORIZATION) {
                publishEvent(new WorkflowMessage(LoginEvent.PREVIEW_MODE_ENABLED));
            } else if (authController.getAuthMode() == AuthMode.CBL_AUTHORIZATION) {
                publishEvent(new WorkflowMessage(LoginEvent.CBL_AUTH_FINISHED));
            }
        } else {
            publishEvent(new WorkflowMessage(LoginEvent.LOGIN));
        }
    }
}
