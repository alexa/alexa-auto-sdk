package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;

import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

/**
 * Base command class to execute Alexa setup workflow command.
 */
public abstract class Command {
    public static final String CHECK_NETWORK_STATUS_COMMAND = "CheckNetworkStatusCommand";
    public static final String CHECK_LANGUAGE_COMMAND = "CheckLanguageCommand";
    public static final String CHECK_CONTACTS_CONSENT_STATUS_COMMAND = "CheckContactsConsentStatusCommand";
    public static final String CHECK_LOCATION_CONSENT_COMMAND = "CheckLocationConsentCommand";
    public static final String CHECK_LOGIN_REQUIRED_COMMAND = "CheckLoginRequiredCommand";
    public static final String SETUP_COMPLETE_COMMAND = "SetupCompleteCommand";

    private final Context mConext;

    /**
     * Constructor.
     */
    public Command(Context context) {
        mConext = context;
    }

    public Context getContext() {
        return mConext;
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