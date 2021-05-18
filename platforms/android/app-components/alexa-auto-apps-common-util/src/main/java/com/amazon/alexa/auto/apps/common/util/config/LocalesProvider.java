package com.amazon.alexa.auto.apps.common.util.config;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.util.Pair;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apps.common.util.FileUtil;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;

import io.reactivex.rxjava3.core.Single;

/**
 * A helper object to provide device locale and Alexa supported locale values.
 */
public class LocalesProvider {
    private static final String TAG = LocalesProvider.class.getSimpleName();

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final ExecutorService mExecutorService;
    @NonNull
    private final Handler mMainThreadHandler;

    public LocalesProvider(@NonNull WeakReference<Context> contextWk, @NonNull ExecutorService executorService) {
        mContextWk = contextWk;
        mExecutorService = executorService;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Fetch Alexa locale's country and language based on locale id.
     * @param localeId locale id.
     * @return Locale's {language, country} pair.
     */
    public Single<Optional<Pair<String, String>>> fetchAlexaSupportedLocaleWithId(String localeId) {
        return fetchAlexaSupportedLocales().map(localeMap -> {
            Pair<String, String> languageCountryPair = localeMap.get(localeId);
            if (languageCountryPair == null)
                return Optional.empty();
            return Optional.of(languageCountryPair);
        });
    }

    /**
     * Fetch all Alexa locale's {country and language}.
     *
     * @return Map of Locale's id <-> pair {language and country}.
     */
    public Single<Map<String, Pair<String, String>>> fetchAlexaSupportedLocales() {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                Log.v(TAG, "Fetching alexa supported locales");
                String locales = FileUtil.readLocales(mContextWk.get());

                Map<String, Pair<String, String>> localeMap = new HashMap<>();
                try {
                    JSONObject jsonObject = new JSONObject(locales);
                    JSONArray array = jsonObject.getJSONArray("locales");

                    for (int i = 0; i < array.length(); i++) {
                        JSONObject objectInArray = array.getJSONObject(i);
                        String localeId = objectInArray.getString("id");
                        JSONObject locale = objectInArray.getJSONObject("locale");
                        String language = locale.getString("language");
                        String country = locale.getString("country");
                        localeMap.put(localeId, new Pair<>(language, country));
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "Failed to parse locale values from locales json file " + e);
                    mMainThreadHandler.post(() -> emitter.onError(e));
                    return;
                }

                mMainThreadHandler.post(() -> emitter.onSuccess(localeMap));
            });
        });
    }

    /**
     * Helper method to identify if current device locale is supported by Alexa or not.
     * @param currentLocale current device locale.
     * @return true if it is supported by Alexa, otherwise return false.
     */
    public Single<Boolean> isCurrentLocaleSupportedByAlexa(String currentLocale) {
        return fetchAlexaSupportedLocaleList().map(supportedLocaleList -> {
            for (String supportedLocale : supportedLocaleList) {
                if (currentLocale.equals(supportedLocale)) {
                    return true;
                }
            }

            return false;
        });
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
     * Fetch current device local display name.
     */
    public String getCurrentDeviceLocaleDisplayName() {
        Locale currentLocale = mContextWk.get().getResources().getConfiguration().getLocales().get(0);
        Preconditions.checkNotNull(currentLocale);

        return currentLocale.getDisplayName();
    }

    /**
     * Fetch all Alexa supported locales and put into locale list.
     */
    private Single<List<String>> fetchAlexaSupportedLocaleList() {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                List<String> supportedLocaleList = new ArrayList<>();
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
                        Log.e(TAG, "Failed to parse locale values from locales json file " + e);
                        mMainThreadHandler.post(() -> emitter.onError(e));
                        return;
                    }
                }

                mMainThreadHandler.post(() -> emitter.onSuccess(supportedLocaleList));
            });
        });
    }
}
