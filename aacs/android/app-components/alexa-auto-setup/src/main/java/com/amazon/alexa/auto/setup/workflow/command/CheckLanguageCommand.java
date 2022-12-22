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

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.os.LocaleListCompat;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
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
    private static final String TAG = CheckLanguageCommand.class.getCanonicalName();

    @Inject
    AlexaLocalesProvider mAlexaLocalesProvider;

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

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
        if (AppCompatDelegate.getApplicationLocales().isEmpty()) {
            Log.d(TAG, "AppCompatDelegate locale is empty");
            AppCompatDelegate.setApplicationLocales(LocaleListCompat.wrap(Resources.getSystem().getConfiguration().getLocales()));
        }
        checkLocaleSupported();

    }

    private void checkLocaleSupported() {
        String currentLocale = LocaleUtil.INSTANCE.parseAndroidLocaleListToAlexaLocaleString(
                AppCompatDelegate.getApplicationLocales());
        mAlexaLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale).subscribe(localeSupported -> {
            if (localeSupported) {
                mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, currentLocale).subscribe(successStatus -> {
                    if (successStatus) {
                        publishEvent(new WorkflowMessage(LoginEvent.LANGUAGE_IS_SUPPORTED_EVENT));
                        Log.d(TAG, "publishEvent: " + LoginEvent.LANGUAGE_IS_SUPPORTED_EVENT);
                    } else {
                        Log.e(TAG, "error updating alexa locale");
                    }
                });
            } else if (!currentLocale.equals("null")){
                publishEvent(new WorkflowMessage(LoginEvent.LANGUAGE_IS_NOT_SUPPORTED_EVENT));
                Log.d(TAG, "publishEvent: " + LoginEvent.LANGUAGE_IS_NOT_SUPPORTED_EVENT);
            }
        });
    }
}
