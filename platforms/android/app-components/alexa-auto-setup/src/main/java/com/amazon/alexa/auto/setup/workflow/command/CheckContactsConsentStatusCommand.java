package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;

/**
 * Command to check if user has consented to upload contacts to Alexa.
 */
public class CheckContactsConsentStatusCommand extends Command {
    private static final String TAG = CheckContactsConsentStatusCommand.class.getSimpleName();

    private final Context mContext;

    public CheckContactsConsentStatusCommand(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void execute() {
        observeContactsConsentStatus();
    }

    private void observeContactsConsentStatus() {
        AlexaApp app = AlexaApp.from(mContext);
        if (app.getRootComponent().getComponent(ContactsController.class).isPresent()) {
            app.getRootComponent().getComponent(ContactsController.class).ifPresent(contactsController -> {
                contactsController.observeContactsConsent().subscribe(isContactsConsentNeeded -> {
                    if (isContactsConsentNeeded) {
                        Log.d(TAG, "Showing contacts consent screen.");
                        EventBus.getDefault().post(new WorkflowMessage("Contacts_Consent_Setup_Not_Finished"));
                    } else {
                        EventBus.getDefault().post(new WorkflowMessage("Contacts_Consent_Setup_Finished"));
                    }
                });
            });
        } else {
            Log.d(TAG, "Skipping contacts consent step.");
            EventBus.getDefault().post(new WorkflowMessage("Contacts_Consent_Setup_Skipped"));
        }
    }
}
