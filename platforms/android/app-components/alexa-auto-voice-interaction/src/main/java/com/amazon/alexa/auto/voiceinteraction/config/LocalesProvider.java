package com.amazon.alexa.auto.voiceinteraction.config;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.voiceinteraction.util.FileUtil;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * A helper object to provide device locale and Alexa supported locale values.
 */
public class LocalesProvider {
    private static final String TAG = LocalesProvider.class.getSimpleName();

    @NonNull
    private final WeakReference<Context> mContextWk;

    private List<String> supportedLocaleList;

    public LocalesProvider(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
    }

    /**
     * Fetch Alexa locale's country and language based on locale id.
     * @param localeId locale id.
     * @return Locale's country and language.
     */
    public List<String> fetchAlexaSupportedLocaleWithId(String localeId) {
        List<String> localeValues = new ArrayList<>();

        String locales = FileUtil.readLocales(mContextWk.get());
        if (!locales.equals("")) {
            try {
                JSONObject jsonObject = new JSONObject(locales);
                JSONArray array = jsonObject.getJSONArray("locales");

                for (int i = 0; i < array.length(); i++) {
                    JSONObject objectInArray = array.getJSONObject(i);
                    if (objectInArray.getString("id").equals(localeId)) {
                        JSONObject locale = objectInArray.getJSONObject("locale");
                        String language = locale.getString("language");
                        String country = locale.getString("country");
                        localeValues.add(language);
                        localeValues.add(country);
                        return localeValues;
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "Failed to parse locale values from locales json file");
            }
        }
        return null;
    }

    /**
     * Helper method to identify if current device locale is supported by Alexa or not.
     * @param currentLocale current device locale.
     * @return true if it is supported by Alexa, otherwise return false.
     */
    public boolean isCurrentLocaleSupportedByAlexa(String currentLocale) {
        fetchAlexaSupportedLocaleList();

        for (String supportedLocale : supportedLocaleList) {
            if (currentLocale.equals(supportedLocale)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Fetch current device locale.
     */
    public String getCurrentDeviceLocale() {
        Locale currentLocale = mContextWk.get().getResources().getConfiguration().getLocales().get(0);
        Preconditions.checkNotNull(currentLocale);

        return currentLocale.getLanguage() + "-" + currentLocale.getCountry();
    }

    /**
     * Fetch all Alexa supported locales and put into locale list.
     */
    private void fetchAlexaSupportedLocaleList() {
        supportedLocaleList = new ArrayList<>();

        String locales = FileUtil.readLocales(mContextWk.get());

        if (!locales.equals("")) {
            try {
                JSONObject jsonObject = new JSONObject(locales);
                JSONArray array = jsonObject.getJSONArray("locales");
                for (int i = 0; i < array.length(); i++) {
                    JSONObject objectInArray = array.getJSONObject(i);
                    supportedLocaleList.add(objectInArray.getString("id"));
                }
            } catch (JSONException e) {
                Log.e(TAG, "Failed to parse locale values from locales json file");
            }
        }
    }
}
