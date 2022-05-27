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
package com.amazon.alexa.auto.settings.config;

import static com.amazon.aacsconstants.AACSConstants.AACS_AMAZONLITE_CONFIG;
import static com.amazon.aacsconstants.AACSConstants.AACS_COASSISTANT;
import static com.amazon.alexa.auto.apps.common.Constants.MODELS;
import static com.amazon.alexa.auto.apps.common.Constants.PATH;

import android.content.ComponentName;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.apis.alexaCustomAssistant.AssistantManager;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.FileUtil;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

import io.reactivex.rxjava3.core.Single;

/**
 * A helper object to configure AACS.
 */
public class AACSConfigurator {
    private static final String TAG = AACSConfigurator.class.getSimpleName();

    // AACS Configs
    private static final String AACS_CONFIG_FILE_PATH = "configFilepaths";
    private static final String AACS_CONFIG_STRINGS = "configStrings";

    private static final String ASSISTANTS = "assistants";

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final TargetComponent mAACSTarget;
    @NonNull
    private final AACSSender mAACSSender;
    @Nullable
    private final AACSConfigurationPreferences mConfigOverrider;

    private static String[] AMAZONLITE_MODEL_FILES;

    /**
     * Constructs an instance of {@link AACSConfigurator}.
     *
     * @param contextWk Android Context.
     * @param aacsSender Helper for sending messages to AACS.
     * @param configOverrider Optional object to override AACS config before it is
     *                        sent to AACS.
     */
    public AACSConfigurator(@NonNull WeakReference<Context> contextWk, @NonNull AACSSender aacsSender,
            @Nullable AACSConfigurationPreferences configOverrider) {
        mContextWk = contextWk;
        mAACSSender = aacsSender;
        mConfigOverrider = configOverrider;

        mAACSTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.getAACSPackageName(contextWk), AACSConstants.AACS_CLASS_NAME),
                TargetComponent.Type.SERVICE);
    }

    /**
     * Configure AACS with default app config.
     */
    public void configureAACSUsingDefaultAppConfig() {
        Log.i(TAG, "Configuring Alexa Client Service with default app config.");

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        FileUtil.readAACSConfigurationAsync(context).subscribe(this::sendConfigurationMessage);
    }

    /**
     * Configure AACS with preference overrides.
     */
    public void configureAACSWithPreferenceOverrides() {
        Log.i(TAG, "Configuring Alexa Client Service with preference overrides.");

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        FileUtil.readAACSConfigurationAsync(context).subscribe(
                config -> { mergeConfigurationWithPreferences(config).subscribe(this::sendConfigurationMessage); });
    }

    /**
     * Sharing files with AACS so it can have read access for processing in its own thread.
     * For example, if AACS Sample App provides `aacs.amazonlite` configuration node in
     * `aacs_config.json` and wake word models in different locales in the `assets/models` folder
     * to enable runtime switching the wake word model based on locale selected, AACS Sample App
     * would need to share those wake word models with AACS for it to successfully load during runtime.
     */
    public void shareFilesWithAACS(@NonNull Context context) {
        Log.i(TAG, "sharing files with AACS");
        File fileDir = context.getFilesDir();

        // Sharing wake word models
        FileUtil.copyModelsToFilesDir(context);
        File modelsDir = new File(fileDir, MODELS);

        FileUtil.readAACSConfigurationAsync(context).subscribe(config -> {
            try {
                JSONObject configJson = new JSONObject(config);
                JSONObject aacsAmazonLite = configJson.getJSONObject(AACS_AMAZONLITE_CONFIG);
                JSONArray modelsJson = aacsAmazonLite.getJSONArray(MODELS);
                ArrayList<String> modelFiles = new ArrayList<>();
                for (int i = 0; i < modelsJson.length(); i++) {
                    String modelPath = modelsJson.getJSONObject(i).getString(PATH);
                    modelFiles.add(modelPath);
                }
                AMAZONLITE_MODEL_FILES = new String[modelFiles.size()];
                modelFiles.toArray(AMAZONLITE_MODEL_FILES);
                AACSServiceController.shareFilePermissionsOfSameType(
                        context, modelsDir, AMAZONLITE_MODEL_FILES, AACS_AMAZONLITE_CONFIG);
            } catch (JSONException e) {
                Log.w(TAG, "Error occurs while sharing wake word models with AACS" + e);
            }
        });
    }

    /**
     * Send configuration message to AACS with configs JSON.
     *
     * @param configs Configuration to to be sent to AACS.
     */
    private void sendConfigurationMessage(@NonNull String configs) {
        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);
        if (ModuleProvider.isAlexaCustomAssistantEnabled(context)) {
            Log.d(TAG, "Updating aacs config for assistant settings");
            try {
                JSONObject configJson = new JSONObject(configs);
                AlexaApp mApp = AlexaApp.from(context);
                if (mApp.getRootComponent().getComponent(AssistantManager.class).isPresent()) {
                    AssistantManager assistantManager =
                            mApp.getRootComponent().getComponent(AssistantManager.class).get();
                    JSONObject settings = assistantManager.getAssistantsSettings();
                    if (settings != null && !settings.toString().isEmpty()) {
                        JSONObject coAssistantConfig = configJson.getJSONObject(AACS_COASSISTANT);
                        coAssistantConfig.put(ASSISTANTS, settings.getJSONObject(ASSISTANTS));
                        configs = configJson.toString();
                    }
                }
            } catch (JSONException e) {
                Log.w(TAG, "Error occurred updating the config for assistant settings");
            }
        }

        String configMessage = "{\n"
                + "  \"" + AACS_CONFIG_FILE_PATH + "\" : [],"
                + "  \"" + AACS_CONFIG_STRINGS + "\" : [" + configs + "]"
                + "}";

        mAACSSender.sendConfigMessageAnySize(configMessage, mAACSTarget, context);
        Log.i(TAG, "Alexa Client Service configured.");
    }

    private Single<String> mergeConfigurationWithPreferences(@NonNull final String config) {
        return Single.create(emitter -> {
            if (mConfigOverrider == null) {
                emitter.onSuccess(config);
                return;
            }

            Handler handler = new Handler(Looper.getMainLooper());
            Thread ioThread = new Thread(() -> {
                try {
                    JSONObject configObject = new JSONObject(config);
                    mConfigOverrider.updatePreferenceFromConfig(configObject);
                    String updatedConfig = mConfigOverrider.updateConfigFromPreference(configObject).toString();
                    handler.post(() -> emitter.onSuccess(updatedConfig));
                } catch (JSONException exception) {
                    Log.w(TAG, "Error updating configuration from preferences: " + exception);
                    handler.post(() -> emitter.onError(exception));
                }
            });

            ioThread.setDaemon(true);
            ioThread.start();
        });
    }
}
