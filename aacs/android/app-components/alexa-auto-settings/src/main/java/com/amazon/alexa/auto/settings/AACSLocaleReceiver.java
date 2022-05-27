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
package com.amazon.alexa.auto.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import javax.inject.Inject;

/**
 * AACSLocaleReceiver receives and handles Android locale changed intent. When system locale is changed, we want to
 * check if
 * 1. system locale is supported by Alexa, Alexa locale will be set to new system locale, and persist Alexa locale with
 * new system locale.
 * 2. system locale is not supported by Alexa. If user is signed in, we display language mismatch popup screen
 * and let user select one Alexa supported locale if he wants to. If user is not signed in, we update application's
 * locale configuration and reset Alexa locale.
 */
public class AACSLocaleReceiver extends BroadcastReceiver {
    private static final String TAG = AACSLocaleReceiver.class.getCanonicalName();
    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    AlexaLocalesProvider mLocalesProvider;

    @Override
    public void onReceive(Context context, Intent intent) {
        DaggerSettingsComponent.builder().androidModule(new AndroidModule(context)).build().inject(this);
        Log.d(TAG, intent.getAction());
        if ("android.intent.action.LOCALE_CHANGED".equals(intent.getAction())) {
            String currentLocale = LocaleUtil.getCurrentDeviceLocale(context);
            String currentDeviceLocaleDisplayLanguage = LocaleUtil.getCurrentDeviceLocaleDisplayLanguage(context);

            Log.d(TAG, "System locale is changed, persist the system locale information.");
            LocaleUtil.persistSystemLocaleData(context, currentLocale, currentDeviceLocaleDisplayLanguage);

            mLocalesProvider.isCurrentLocaleSupportedByAlexa(currentLocale).subscribe(localeSupported -> {
                Boolean isSignedIn = AlexaApp.from(context).getRootComponent().getAuthController().isAuthenticated();
                if (isSignedIn) {
                    if (localeSupported) {
                        Log.d(TAG, "Locale is supported by Alexa, don't popup Alexa language selection screen.");
                        if (isSignedIn) {
                            Log.d(TAG, "persist Alexa locale to " + currentLocale);
                            LocaleUtil.persistAlexaLocale(context, currentLocale);
                        }
                    } else {
                        Log.d(TAG,
                                "Locale is not supported by Alexa, popup Alexa language selection screen if logged in.");
                        startPopup(context);
                    }
                } else {
                    LocaleUtil.updateLocaleConfigurationWithSystemLocale(context, currentLocale);
                    LocaleUtil.resetPersistentAlexaLocale(context);
                }
            });
        }
    }

    private void startPopup(Context context) {
        Log.d(TAG, "System Language not supported by Alexa, bring up popup");
        Intent intent = new Intent(context, UpdateLanguageActivity.class);
        context.startActivity(intent);
    }
}
