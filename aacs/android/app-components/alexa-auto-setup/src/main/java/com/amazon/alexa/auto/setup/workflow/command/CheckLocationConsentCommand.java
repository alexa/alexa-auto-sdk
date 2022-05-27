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
package com.amazon.alexa.auto.setup.workflow.command;

import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.DISABLED;
import static com.amazon.alexa.auto.setup.workflow.model.UserConsent.ENABLED;

import android.content.Context;
import android.util.Log;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import java.util.Optional;

import javax.inject.Inject;

/**
 * Command to check which setup screens need to be presented
 */
public class CheckLocationConsentCommand extends Command {
    private static final String TAG = CheckLocationConsentCommand.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    public CheckLocationConsentCommand(Context context) {
        super(context);

        if (mAlexaPropertyManager == null)
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

        if (mAlexaPropertyManager != null) {
            mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.GEOLOCATION_ENABLED)
                    .filter(Optional::isPresent)
                    .map(Optional::get)
                    .subscribe(consent_value -> {
                        if (ENABLED.getValue().equals(consent_value)) {
                            publishEvent(new WorkflowMessage(LoginEvent.LOCATION_CONSENT_COMPLETED));
                        } else if (DISABLED.getValue().equals(consent_value)) {
                            publishEvent(new WorkflowMessage(LoginEvent.LOCATION_CONSENT_NOT_COMPLETED));
                        }
                    });
        } else {
            publishEvent(new WorkflowMessage(LoginEvent.LOCATION_CONSENT_NOT_COMPLETED));
        }
    }
}
