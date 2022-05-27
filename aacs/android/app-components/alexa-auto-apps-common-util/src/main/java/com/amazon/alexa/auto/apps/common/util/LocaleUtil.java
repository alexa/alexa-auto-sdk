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
package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.util.DisplayMetrics;
import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Locale;

/**
 * Class for Locale related utilities
 */
public class LocaleUtil {
    private static final String TAG = LocaleUtil.class.getSimpleName();

    public static final String DEFAULT_DOMAIN = "amazon.com";
    private static HashMap<String, String> localeAmazonDomainMap;

    public static final String ALEXA_LOCALE_KEY = "alexaLocale-key";
    public static final String SYSTEM_LOCALE_KEY = "systemLocale-key";

    public static final String LOCALE = "locale";
    public static final String LANGUAGE = "language";

    public static void init() {
        if (localeAmazonDomainMap == null) {
            localeAmazonDomainMap = new HashMap<>();
            localeAmazonDomainMap.put("en_US", DEFAULT_DOMAIN);
            localeAmazonDomainMap.put("en_GB", "amazon.co.uk");
            localeAmazonDomainMap.put("en_AU", "amazon.co.uk");
            localeAmazonDomainMap.put("en_CA", "amazon.co.uk");
            localeAmazonDomainMap.put("en_IN", "amazon.co.uk");
            localeAmazonDomainMap.put("de_DE", "amazon.de");
            localeAmazonDomainMap.put("es_ES", "amazon.es");
            localeAmazonDomainMap.put("es_MX", "amazon.com.mx");
            localeAmazonDomainMap.put("it_IT", "amazon.it");
            localeAmazonDomainMap.put("pt_BR", DEFAULT_DOMAIN);
            localeAmazonDomainMap.put("ja_JP", "amazon.co.jp");
            localeAmazonDomainMap.put("fr_FR", "amazon.fr");
            localeAmazonDomainMap.put("fr_CA", "amazon.ca");
            localeAmazonDomainMap.put("hi_IN", "amazon.in");
        }
    }

    public static String getLocalizedDomain(Locale locale) {
        init();
        String domain = localeAmazonDomainMap.get(locale.toString());
        if (domain != null) {
            return domain;
        } else if (localeAmazonDomainMap.get(locale.getLanguage()) != null) {
            return localeAmazonDomainMap.get(locale.getLanguage());
        }
        return DEFAULT_DOMAIN;
    }

    /**
     * Update device locale configuration to Alexa locale, and return the updated Android context,
     * if device locale configuration has been updated.
     *
     * @param context     Android context
     * @param alexaLocale Alexa locale string
     * @return updated Android context, if device locale configuration has been updated
     */
    public static Context updateLocaleConfigurationWithAlexaLocale(Context context, String alexaLocale) {
        alexaLocale = parseAlexaLocaleToAndroidLocale(alexaLocale);
        Locale[] locales = Locale.getAvailableLocales();

        for (Locale locale : locales) {
            if (alexaLocale.equals(locale.toString())) {
                Resources res = context.getResources();
                DisplayMetrics dm = res.getDisplayMetrics();
                Configuration conf = res.getConfiguration();

                conf.setLocale(locale);
                Locale.setDefault(locale);
                conf.setLayoutDirection(locale);

                Log.d(TAG, "Updating device locale configuration to " + alexaLocale);
                res.updateConfiguration(conf, dm);
                return context;
            }
        }

        Log.w(TAG, alexaLocale + "is not available on device, return original Android locale configuration.");
        return context;
    }

    /**
     * Update device locale configuration to system locale, and return the updated Android context.
     *
     * @param context      Android context
     * @param systemLocale system locale string
     * @return updated Android context
     */
    public static Context updateLocaleConfigurationWithSystemLocale(Context context, String systemLocale) {
        Locale[] locales = Locale.getAvailableLocales();

        for (Locale locale : locales) {
            if (systemLocale.equals(locale.toString())) {
                Resources res = context.getResources();
                DisplayMetrics dm = res.getDisplayMetrics();
                Configuration conf = res.getConfiguration();

                conf.setLocale(locale);
                Locale.setDefault(locale);
                conf.setLayoutDirection(locale);

                Log.d(TAG, "Updating device locale configuration to " + systemLocale);
                res.updateConfiguration(conf, dm);
                break;
            }
        }
        return context;
    }

    public static String parseAlexaLocaleToAndroidLocale(String locale) {
        locale = locale.split("/")[0];
        return locale.replace("-", "_");
    }

    /**
     * Persist Alexa locale that is selected by user in either setup or settings screen.
     *
     * @param context     Android context
     * @param alexaLocale Alexa locale string
     */
    public static void persistAlexaLocale(@NonNull Context context, String alexaLocale) {
        SharedPreferences preferences = context.getSharedPreferences(ALEXA_LOCALE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(ALEXA_LOCALE_KEY, alexaLocale);
        editor.apply();
    }

    /**
     * Get Alexa locale that is selected by user in either setup or settings screen.
     *
     * @param context Android context
     * @return Alexa locale string
     */
    public static String getPersistentAlexaLocale(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(ALEXA_LOCALE_KEY, Context.MODE_PRIVATE);
        return preferences.getString(ALEXA_LOCALE_KEY, "");
    }

    /**
     * Remove the persistent Alexa locale.
     * @param context Android context
     */
    public static void resetPersistentAlexaLocale(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(ALEXA_LOCALE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.remove(ALEXA_LOCALE_KEY);
        editor.apply();
    }

    /**
     * Persist system locale data that is selected by user in system settings screen.
     *
     * @param context        Android context
     * @param systemLocale   system locale string
     * @param systemLanguage system language string
     */
    public static void persistSystemLocaleData(@NonNull Context context, String systemLocale, String systemLanguage) {
        JSONObject locale = new JSONObject();
        try {
            locale.put(LOCALE, systemLocale);
            locale.put(LANGUAGE, systemLanguage);
        } catch (JSONException e) {
            Log.d(TAG, "Fail to create system locale json");
        }

        SharedPreferences preferences = context.getSharedPreferences(SYSTEM_LOCALE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(SYSTEM_LOCALE_KEY, locale.toString());
        editor.apply();
    }

    /**
     * Get system locale data that is selected by user in system settings screen. If system locale is empty, store
     * current system locale.
     *
     * @param context Android context
     * @return system locale json string that contains locale code and language
     */
    public static String getPersistentSystemLocaleData(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(SYSTEM_LOCALE_KEY, Context.MODE_PRIVATE);
        String systemLocaleObj = preferences.getString(SYSTEM_LOCALE_KEY, "");

        if (systemLocaleObj.isEmpty()) {
            Locale systemLocale = context.getResources().getConfiguration().getLocales().get(0);
            persistSystemLocaleData(context, systemLocale.getLanguage() + "-" + systemLocale.getCountry(),
                    systemLocale.getDisplayLanguage());
        }
        return systemLocaleObj;
    }

    /**
     * Get persistent system locale code name.
     *
     * @param context Android context
     * @return system locale code name
     */
    public static String getPersistentSystemLocale(@NonNull Context context) {
        try {
            JSONObject systemLocaleObj = new JSONObject(getPersistentSystemLocaleData(context));
            return systemLocaleObj.optString(LocaleUtil.LOCALE);
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse persistent system locale. Error: " + e);
            return "";
        }
    }

    public static String parseLanguageFromLocale(String locale) {
        return locale.split("-")[0];
    }

    /**
     * Fetch current device locale.
     */
    public static String getCurrentDeviceLocale(Context context) {
        Locale currentLocale = context.getResources().getConfiguration().getLocales().get(0);
        Preconditions.checkNotNull(currentLocale);

        return currentLocale.getLanguage() + "-" + currentLocale.getCountry();
    }

    /**
     * Fetch current device local display name: language-country.
     */
    public static String getCurrentDeviceLocaleDisplayName(Context context) {
        Locale currentLocale = context.getResources().getConfiguration().getLocales().get(0);
        Preconditions.checkNotNull(currentLocale);

        return currentLocale.getDisplayName();
    }

    /**
     * Fetch current device local display language.
     */
    public static String getCurrentDeviceLocaleDisplayLanguage(Context context) {
        Locale currentLocale = context.getResources().getConfiguration().getLocales().get(0);
        Preconditions.checkNotNull(currentLocale);

        return currentLocale.getDisplayLanguage();
    }
}
