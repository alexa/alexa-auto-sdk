package com.amazon.alexa.auto.setup.workflow.command;

import android.content.Context;

import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import javax.inject.Inject;

/**
 * Command to check if device's locale is supported by Alexa.
 */
public class CheckLanguageCommand extends Command {
    private final Context mContext;

    @Inject
    LocalesProvider mLocalesProvider;

    public CheckLanguageCommand(Context context) {
        super(context);
        mContext = context;

        DaggerSetupComponent.builder()
                .androidModule(new AndroidModule(mContext))
                .build()
                .injectCheckLanguageCommand(this);
    }

    @Override
    public void execute() {
        String currentLocale = mLocalesProvider.getCurrentDeviceLocale();
        mLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale).subscribe(localeSupported -> {
            if (localeSupported) {
                publishEvent(new WorkflowMessage(LoginEvent.LANGUAGE_IS_SUPPORTED_EVENT));
            } else {
                publishEvent(new WorkflowMessage(LoginEvent.LANGUAGE_IS_NOT_SUPPORTED_EVENT));
            }
        });
    }
}
