package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;
import android.util.Log;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import java.util.Optional;

import javax.inject.Inject;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.SETUP_ERROR;
import static com.amazon.alexa.auto.setup.workflow.model.LocationConsent.ENABLED;

/**
 * Command to check which setup screens need to be presented
 */
public class CheckLocationConsentCommand extends Command {
    private static final String TAG = CheckLocationConsentCommand.class.getSimpleName();

    public CheckLocationConsentCommand(Context context) {
        super(context);

        DaggerSetupComponent.builder()
                .androidModule(new AndroidModule(context))
                .build()
                .injectCheckLocationConsentCommand(this);
    }

    @Override
    public void execute() {
        String extraModules = ModuleProvider.getModules(getContext());
        if (!extraModules.contains(ModuleProvider.ModuleName.GEOLOCATION.name())) {
            Log.d(TAG, "Geolocation aacs module not present, skipping");
            publishEvent(new WorkflowMessage(LoginEvent.LOCATION_CONSENT_COMPLETED));
            return;
        }
        // As long as the geolocation module is present, show the Location Consent screen every time
        // the user signs in with CBL/activates preview mode
        publishEvent(new WorkflowMessage("Location_Consent"));
    }
}
