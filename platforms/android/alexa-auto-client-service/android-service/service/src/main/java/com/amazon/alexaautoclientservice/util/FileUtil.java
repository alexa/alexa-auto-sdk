/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.media.MediaRecorder;
import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;
import com.amazon.aace.storage.config.StorageConfiguration;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexaautoclientservice.constants.AudioSourceConstants;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Objects;

public class FileUtil {
    private static final String TAG = AACSConstants.AACS + "-" + FileUtil.class.getSimpleName();

    public static final String APPDATA_DIR = "appdata";
    public static final String CERTS_DIR = "certs";
    public static final String EXTERNAL_FILE_DIR = "externalFiles";
    public static final String MODEL_DIR = "aacs.amazonLite";
    public static final String CONFIG_KEY = "aacs-configuration";
    public static final String AACS_GENERAL_CONFIG = "aacs-general";
    public static final String AACS_EXTRAS_CONFIG = "aacs-extras";

    public static final String AACS_CONFIG_NETWORK_INFO_PROVIDER = "NetworkInfoProvider";
    public static final String AACS_CONFIG_LOCATION_PROVIDER = "LocationProvider";
    public static final String AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER = "ExternalMediaAdapter";
    public static final String AACS_CONFIG_PROPERTY_MANAGER = "PropertyManager";

    public static final HashMap<String, String> mInterfaceToModuleNames = new HashMap<String, String>() {
        {
            put(AACS_CONFIG_NETWORK_INFO_PROVIDER, "network");
            put(AACS_CONFIG_LOCATION_PROVIDER, "location");
            put(AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER, "alexa");
            put(AACS_CONFIG_PROPERTY_MANAGER, "propertyManager");
        }
    };

    private static boolean copyFileFromAssetPath(
            String assetPath, File destFile, boolean force, AssetManager assetManager) {
        if (!destFile.exists() || force) {
            if (destFile.getParentFile().exists() || destFile.getParentFile().mkdirs()) {
                // Copy the asset to the dest path
                try (InputStream is = assetManager.open(assetPath); OutputStream os = new FileOutputStream(destFile)) {
                    byte[] buf = new byte[1024];
                    int len;
                    while ((len = is.read(buf)) > 0) {
                        os.write(buf, 0, len);
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Could not copy file " + assetPath);
                    return false;
                }
            } else {
                Log.e(TAG, "Could not create directory: " + destFile.getParentFile());
                return false;
            }
        } else {
            Log.w(TAG, String.format("Skipping existing file in : %s to: %s", assetPath, destFile));
        }
        return true;
    }

    public static void createDirectoriesAndCopyCertificates(
            File certsDir, File appDataDir, File externalFileDir, File modelsDir, AssetManager assetManager) {
        if (!appDataDir.exists()) {
            appDataDir.mkdir();
        }

        if (!certsDir.exists()) {
            certsDir.mkdir();
        }

        if (!externalFileDir.exists()) {
            externalFileDir.mkdir();
        }

        if (!modelsDir.exists()) {
            modelsDir.mkdir();
        }

        if (!appDataDir.exists() || !certsDir.exists() || !externalFileDir.exists() || !modelsDir.exists()) {
            Log.w(TAG, "Failed to create internal storage directories.");
            return;
        }

        try {
            String[] certAssets = assetManager.list(CERTS_DIR);
            for (String next : certAssets) {
                if (copyFileFromAssetPath(CERTS_DIR + "/" + next, new File(certsDir, next), false, assetManager))
                    continue;
                return;
            }
        } catch (IOException e) {
            Log.w(TAG, "Cannot copy certs to cache directory. Error: " + e.getMessage());
        }
    }

    public static ArrayList<EngineConfiguration> getEngineConfiguration(@NonNull Context context) {
        File certsDir = new File(context.getCacheDir(), CERTS_DIR);
        File appDataDir = new File(context.getFilesDir(), APPDATA_DIR);

        InputStream configStream = new ByteArrayInputStream(
                getSavedConfigurationForEngine(context).toString().getBytes(StandardCharsets.UTF_8));

        // Construct and return EngineConfigurations list
        return new ArrayList<EngineConfiguration>(Arrays.asList(AlexaConfiguration.createCurlConfig(certsDir.getPath()),
                AlexaConfiguration.createMiscStorageConfig(appDataDir.getPath() + "/miscStorage.sqlite"),
                AlexaConfiguration.createCertifiedSenderConfig(appDataDir.getPath() + "/certifiedSender.sqlite"),
                AlexaConfiguration.createCapabilitiesDelegateConfig(
                        appDataDir.getPath() + "/capabilitiesDelegate.sqlite"),
                AlexaConfiguration.createAlertsConfig(appDataDir.getPath() + "/alerts.sqlite"),
                AlexaConfiguration.createNotificationsConfig(appDataDir.getPath() + "/notifications.sqlite"),
                AlexaConfiguration.createDeviceSettingsConfig(appDataDir.getPath() + "/deviceSettings.sqlite"),
                StorageConfiguration.createLocalStorageConfig(appDataDir.getPath() + "/localStorage.sqlite"),
                StreamConfiguration.create(configStream)));
    }

    private static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
    }

    public static JSONObject getSavedConfigurationForEngine(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        JSONObject configJson = null;
        try {
            configJson = new JSONObject(config);
            configJson.remove("aacs.general");
            configJson.remove("aacs.defaultPlatformHandlers");
        } catch (JSONException e) {
            Log.e(TAG, "Error while constructing InputStream from stored configuration.");
        }
        return configJson;
    }

    private static void updateEngineConfigurationInSharedPref(Context context, JSONObject engineConfig) {
        SharedPreferences sp = getSharedPreferences(context);
        try {
            JSONObject fullConfig = new JSONObject(sp.getString(CONFIG_KEY, ""));
            engineConfig.put("aacs.general", fullConfig.get("aacs.general"));
            engineConfig.put("aacs.defaultPlatformHandlers", fullConfig.get("aacs.defaultPlatformHandlers"));
            sp.edit().putString(CONFIG_KEY, engineConfig.toString()).apply();
        } catch (JSONException e) {
            Log.e(TAG, "Error while saving engine configuration: " + e.getMessage());
        }
    }

    public static boolean isConfigurationSaved(@NonNull Context context) {
        return getSharedPreferences(context).contains(CONFIG_KEY);
    }

    public static boolean saveConfiguration(
            @NonNull Context context, @NonNull JSONArray configFilepaths, @NonNull JSONArray configs) {
        Log.i(TAG, "Constructing configuration and saving into SharedPreferences.");

        JSONObject fullConfig = constructFullConfiguration(configFilepaths, configs);
        fullConfig = translateToEngineConfiguration(context, fullConfig);

        if (fullConfig == null || !validateConfig(fullConfig))
            return false;

        getSharedPreferences(context).edit().putString(CONFIG_KEY, fullConfig.toString()).apply();
        try {
            getSharedPreferences(context)
                    .edit()
                    .putString(AACS_GENERAL_CONFIG, fullConfig.get("aacs.general").toString())
                    .apply();
        } catch (JSONException e) {
            Log.e(TAG, "Error while storing aacs.general config into shared preferences.");
        }

        // Delete extras config stored in SharedPref
        getSharedPreferences(context).edit().remove(AACS_EXTRAS_CONFIG).apply();

        // Add config to deregister AASB platform interfaces that have default impl
        for (String pi : mInterfaceToModuleNames.keySet()) {
            if (isDefaultImplementationEnabled(context, pi)) {
                deregisterAASBPlatformInterface(context, mInterfaceToModuleNames.get(pi), pi);
            }
        }
        return true;
    }

    private static JSONObject constructFullConfiguration(JSONArray configFilepaths, JSONArray configs) {
        JSONObject fullConfig = new JSONObject();

        try {
            for (int i = 0; i < configFilepaths.length(); i++) {
                String filepath = configFilepaths.getString(i);
                configs.put(readConfigurationFromFile(filepath));
            }

            for (int i = 0; i < configs.length(); i++) {
                JSONObject node = new JSONObject(configs.getString(i));
                Iterator<String> iter = node.keys();
                while (iter.hasNext()) {
                    String key = iter.next();
                    fullConfig.put(key, node.get(key));
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error while constructing full configuration. Error: %s", e.getMessage()));
            return null;
        }

        return fullConfig;
    }

    private static JSONObject readConfigurationFromFile(String filepath) {
        JSONObject obj = null;
        InputStream inputStream = null;

        try {
            inputStream = new FileInputStream(new File(filepath));
        } catch (Exception e) {
            Log.e(TAG, String.format("Cannot read configuration from %s. Error: %s", filepath, e.getMessage()));
        }

        if (inputStream == null) {
            Log.e(TAG, "InputStream not available to read config.");
            return null;
        }

        try {
            byte[] buffer = new byte[inputStream.available()];
            inputStream.read(buffer);
            String json = new String(buffer, "UTF-8");
            obj = new JSONObject(json);
        } catch (Exception e) {
            Log.e(TAG, String.format("Cannot read from %s due to Error: %s", filepath, e.getMessage()));
        } finally {
            try {
                if (inputStream != null)
                    inputStream.close();
            } catch (IOException e) {
                Log.w(TAG,
                        String.format("Failed to close InputStream after reading from %s. Error: %s", filepath,
                                e.getMessage()));
            }
        }
        return obj;
    }

    private static boolean validateConfig(JSONObject config) {
        return validateAudioSource(config);
    }

    private static boolean validateAudioSource(JSONObject config) {
        try {
            Object voiceAudioSource = getLeafNodeValueFromJson(
                    config, "aacs.defaultPlatformHandlers", "audioInput", "audioType", "VOICE", "audioSource");
            Object commsAudioSource = getLeafNodeValueFromJson(
                    config, "aacs.defaultPlatformHandlers", "audioInput", "audioType", "COMMUNICATION", "audioSource");

            Object voiceIsDefault = getLeafNodeValueFromJson(
                    config, "aacs.defaultPlatformHandlers", "audioInput", "audioType", "VOICE", "useDefault");
            Object commsIsDefault = getLeafNodeValueFromJson(
                    config, "aacs.defaultPlatformHandlers", "audioInput", "audioType", "COMMUNICATION", "useDefault");

            // Fail if AudioSources are unequal and non-external. If missing, config is still considered valid
            if (!(voiceAudioSource instanceof String) || !(commsAudioSource instanceof String)) {
                Log.w(TAG, "AudioSources not found for AudioInput while validating config.");
                return true;
            } else if (!(voiceIsDefault instanceof Boolean) || !(commsIsDefault instanceof Boolean)) {
                Log.w(TAG, "useDefault not found for AudioInput while validating config");
                return true;
            }
            boolean notAllDefaultImpl = !((boolean) voiceIsDefault) || !((boolean) commsIsDefault);
            boolean notAllInternalSource = voiceAudioSource.equals(AudioSourceConstants.EXTERNAL)
                    || commsAudioSource.equals(AudioSourceConstants.EXTERNAL);
            if (notAllDefaultImpl || notAllInternalSource)
                return true;

            boolean sameAudioSource;
            if (!(sameAudioSource = voiceAudioSource.equals(commsAudioSource))) {
                Log.e(TAG,
                        "Config validation failed for AudioInput. Non-external AudioSources must be equal because Android"
                                + " only supports having one AudioRecord instance.");
            }
            return sameAudioSource;
        } catch (Exception e) {
            Log.e(TAG, String.format("Error while validating config: %s.", e.getMessage()));
            return false;
        }
    }

    private static JSONObject translateToEngineConfiguration(Context context, JSONObject config) {
        if (config == null)
            return null;
        JSONObject translatedConfig = new JSONObject();
        try {
            Iterator<String> iter = config.keys();
            if (!iter.hasNext()) {
                Log.e(TAG, "Provided configuration is empty.");
                return null;
            }
            while (iter.hasNext()) {
                String key = iter.next();
                if (key.substring(0, 4).equals("aacs")
                        && (!key.contains("modules") && !key.contains("general")
                                && !key.contains("defaultPlatformHandlers"))) {
                    if (key.contains("alexa")) {
                        JSONObject aacsAlexaConfig = config.getJSONObject("aacs.alexa");
                        JSONObject lmsConfig = (JSONObject) aacsAlexaConfig.remove("localMediaSource");
                        JSONObject aasbAlexaConfig = new JSONObject();
                        aasbAlexaConfig.put("LocalMediaSource", lmsConfig);
                        translatedConfig.put("aasb.alexa", aasbAlexaConfig);
                        translatedConfig.put("aace.alexa", new JSONObject().put("avsDeviceSDK", aacsAlexaConfig));
                    } else {
                        if (key.contains("dcm")) {
                            JSONObject dcmConfig = config.getJSONObject("aacs.dcm");
                            String filePath = dcmConfig.optString("metricsFilePath");
                            if (filePath == null || filePath.equals("")) {
                                File appDataDir = new File(context.getFilesDir(), APPDATA_DIR);
                                dcmConfig.put("metricsFilePath", appDataDir.getPath());
                            }
                        }
                        translatedConfig.put("aace" + key.substring(4), config.getJSONObject(key));
                    }
                } else if (key.contains("modules")) {
                    translatedConfig.put("aasb" + key.substring(4), config.getJSONObject(key));
                } else {
                    translatedConfig.put(key, config.getJSONObject(key));
                }
            }
        } catch (Exception e) {
            Log.e(TAG, String.format("Translation of configuration failed. Error: %s", e.getMessage()));
            return null;
        }

        Log.d(TAG, "Configuration successfully translated");
        return translatedConfig;
    }

    public static JSONObject removeExtrasModuleConfiguration(@NonNull Context context, @NonNull String configKey) {
        JSONObject engineConfig = getSavedConfigurationForEngine(context);
        JSONObject moduleConfig = new JSONObject();
        SharedPreferences sp = getSharedPreferences(context);
        SharedPreferences.Editor editor = sp.edit();

        // Support both aacs and aace in config key
        if (configKey.substring(0, 4).equals("aacs")) {
            configKey = "aace" + configKey.substring(4);
        }

        // Check full configuration in SharedPref for extras config first
        if (engineConfig.has(configKey)) {
            try {
                moduleConfig = (JSONObject) engineConfig.remove(configKey);
                updateEngineConfigurationInSharedPref(context, engineConfig);
            } catch (Exception e) {
                Log.w(TAG, String.format("Failed to remove and retrieve config for %s", configKey));
            }

            // Save config for extras module factory in separate SharedPreferences field
            JSONObject extrasJson = new JSONObject();
            if (sp.contains(AACS_EXTRAS_CONFIG)) {
                String extrasConfig = sp.getString(AACS_EXTRAS_CONFIG, "");
                try {
                    extrasJson = new JSONObject(extrasConfig);
                } catch (JSONException e) {
                    Log.w(TAG,
                            String.format(
                                    "Error while creating JSON object for extras config. Error: %s", e.getMessage()));
                }
            }

            try {
                extrasJson.put(configKey, moduleConfig);
            } catch (JSONException e) {
                Log.w(TAG,
                        String.format(
                                "Error while constructing JSON object for extras config. Error: %s", e.getMessage()));
            }
            editor.putString(AACS_EXTRAS_CONFIG, extrasJson.toString()).apply();
        } else {
            // If not in full configuration, check SharedPref for extras (Using stored config)
            if (sp.contains(AACS_EXTRAS_CONFIG)) {
                String extrasConfig = sp.getString(AACS_EXTRAS_CONFIG, "");
                try {
                    JSONObject extrasJson = new JSONObject(extrasConfig);
                    if (extrasJson.has(configKey))
                        moduleConfig = new JSONObject(extrasJson.getString(configKey));
                } catch (JSONException e) {
                    Log.w(TAG,
                            String.format(
                                    "Error while getting JSON object for extras config. Error: %s", e.getMessage()));
                }
            }
        }

        return moduleConfig;
    }

    public static boolean isPersistentSystemService(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "persistentSystemService");
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG, "Defaulting to persistentSystemService=false, since the leaf node value was not valid");
                return false;
            }
            return (Boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG, "Defaulting to persistentSystemService=false, since it was not specified in config.");
            return false;
        }
    }

    public static boolean isStartServiceOnBootEnabled(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "startServiceOnBootEnabled");
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG, "Defaulting to startServiceOnBootEnabled=false, since the leaf node value was not valid");
                return false;
            }
            return (Boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG, "Defaulting to startServiceOnBootEnabled=false, since it was not specified in config.");
            return false;
        }
    }

    public static boolean isDefaultImplementationEnabled(
            @NonNull Context context, @NonNull String platformInterfaceName) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(
                    configJson, "aacs.defaultPlatformHandlers", "useDefault" + platformInterfaceName);
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG,
                        String.format(
                                "Defaulting to isDefaultImplementationEnabled=false for %s, since the leaf node value was not valid.",
                                platformInterfaceName));
                return false;
            }
            return (boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG,
                    String.format(
                            "Defaulting to isDefaultImplementationEnabled=false for %s, since the configuration was not valid.",
                            platformInterfaceName));
            return false;
        }
    }

    public static boolean isAudioInputTypeEnabled(@NonNull Context context, @NonNull String AudioInputType) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "audioInput",
                    "audioType", AudioInputType, "useDefault");
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG,
                        String.format(
                                "Defaulting to isAudioInputTypeEnabled=false for %s, since the leaf node value was not valid",
                                AudioInputType));
                return false;
            }
            return (boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG,
                    String.format(
                            "Defaulting to isAudioInputTypeEnabled=false for %s, since it was not specified in the config.",
                            AudioInputType));
            return false;
        }
    }

    public static boolean isAudioOutputTypeEnabled(@NonNull Context context, @NonNull String AudioOutputType) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "aacs.defaultPlatformHandlers", "audioOutput",
                    "audioType", AudioOutputType, "useDefault");
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG,
                        String.format(
                                "Defaulting to isAudioOutputTypeEnabled=false for %s, since the leaf node value was not valid",
                                AudioOutputType));
                return false;
            }
            return (boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG,
                    String.format("Defaulting to isAudioOutputTypeEnabled=false for %s, Exception encountered: %s.",
                            AudioOutputType, e.getMessage()));
            return false;
        }
    }

    public static int getAudioSourceForAudioType(@NonNull Context context, @NonNull String audioType) {
        String audioSourceString = getAudioSourceStringForAudioType(context, audioType);
        int audioSource;
        switch (audioSourceString) {
            case AudioSourceConstants.MIC:
                audioSource = MediaRecorder.AudioSource.MIC;
                break;
            case AudioSourceConstants.VOICE_RECOGNITION:
                audioSource = MediaRecorder.AudioSource.VOICE_RECOGNITION;
                break;
            case AudioSourceConstants.DEFAULT:
                audioSource = MediaRecorder.AudioSource.DEFAULT;
                break;
            case AudioSourceConstants.VOICE_COMMUNICATION:
                audioSource = MediaRecorder.AudioSource.VOICE_COMMUNICATION;
                break;
            default:
                Log.w(TAG,
                        String.format(
                                "Defaulting to use audioSource: MediaRecorder.AudioSource.MIC for %s, since audioSource found in configuration was not valid.",
                                audioType));
                return MediaRecorder.AudioSource.MIC;
        }
        return audioSource;
    }

    public static boolean isAudioSourceExternal(@NonNull Context context, @NonNull String audioType) {
        String audioSourceString = getAudioSourceStringForAudioType(context, audioType);
        return AudioSourceConstants.EXTERNAL.equals(audioSourceString);
    }

    private static String getAudioSourceStringForAudioType(@NonNull Context context, @NonNull String audioType) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        String audioSourceString = AudioSourceConstants.MIC;
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(
                    configJson, "aacs.defaultPlatformHandlers", "audioInput", "audioType", audioType, "audioSource");
            if (!(leafNodeValue instanceof String)) {
                Log.w(TAG,
                        String.format(
                                "Defaulting to use audioSource: MediaRecorder.AudioSource.MIC for %s, since the leaf node value was not valid.",
                                audioType));
                return audioSourceString;
            }
            audioSourceString = (String) leafNodeValue;
            return audioSourceString;
        } catch (Exception e) {
            Log.w(TAG,
                    String.format(
                            "Defaulting to use audioSource: MediaRecorder.AudioSource.MIC for %s, exception encountered: %s",
                            audioType, e.getMessage()));
            return audioSourceString;
        }
    }

    public static JSONObject getAudioExternalSourceForAudioType(@NonNull Context context, @NonNull String audioType) {
        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(
                    configJson, "aacs.defaultPlatformHandlers", "audioInput", "audioType", audioType, "externalSource");
            if (!(leafNodeValue instanceof JSONObject)) {
                Log.e(TAG,
                        String.format(
                                "Failed to retrieve external source for %s, since the leaf node value was not valid.",
                                audioType));
                return null;
            }
            JSONObject externalSourceTarget = (JSONObject) leafNodeValue;
            if (externalSourceTarget.optString("type") != null && externalSourceTarget.optString("package") != null
                    && externalSourceTarget.optString("class") != null) {
                return externalSourceTarget;
            } else {
                Log.e(TAG,
                        String.format("externalSource JSONObject for %s does not contain all the required fields.",
                                audioType));
                return null;
            }
        } catch (Exception e) {
            Log.w(TAG,
                    String.format("Failed to retrieve external source for %s, exception encountered: %s", audioType,
                            e.getMessage()));
            return null;
        }
    }

    public static JSONArray getIntentTargets(@NonNull Context context, @NonNull String topic, @NonNull String target) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "intentTargets", topic, target);
            if (leafNodeValue instanceof JSONArray) {
                return (JSONArray) leafNodeValue;
            } else {
                Log.w(TAG,
                        String.format(
                                "Returning null due to invalid leaf node value retrieved from config for topic=%s, target=%s",
                                topic, target));
                return null;
            }
        } catch (JSONException e) {
            Log.w(TAG, String.format("Could not find a target for topic=%s from config", topic));
        }
        return null;
    }

    public static Object getLeafNodeValueFromJson(@NonNull JSONObject root, @NonNull String... jsonObjectTree) {
        JSONObject node = root;
        for (String key : jsonObjectTree) {
            Object nextNode = node.opt(key);
            if (nextNode == null) {
                Log.w(TAG, String.format("getLeafNodeValueFromJson: key=%s did not exist in the JSONObject", key));
                return null;
            }
            if (nextNode instanceof JSONObject) {
                node = (JSONObject) nextNode;
            } else {
                return nextNode;
            }
        }
        return node;
    }

    public static String getLVCConfiguration(@NonNull Context context) {
        JSONObject config = getSavedConfigurationForEngine(context);
        String lvcModule = "aace.localVoiceControl";

        if (config.has(lvcModule)) {
            // Remove LVC config and store in SharedPref; Used for LVC apk, rather than Auto SDK
            JSONObject lvcConfig = (JSONObject) config.remove(lvcModule);

            JSONObject extrasJson = new JSONObject();
            SharedPreferences sp = getSharedPreferences(context);
            if (sp.contains(AACS_EXTRAS_CONFIG)) {
                String extrasConfig = sp.getString(AACS_EXTRAS_CONFIG, "");
                try {
                    extrasJson = new JSONObject(extrasConfig);
                } catch (JSONException e) {
                    Log.w(TAG,
                            String.format(
                                    "Error while creating JSON object for extras config. Error: %s", e.getMessage()));
                }
            }

            try {
                extrasJson.put(lvcModule, lvcConfig);
            } catch (JSONException e) {
                Log.w(TAG,
                        String.format(
                                "Error while constructing JSON object for extras config. Error: %s", e.getMessage()));
            }
            sp.edit().putString(AACS_EXTRAS_CONFIG, extrasJson.toString()).apply();

            return lvcConfig.toString();
        } else {
            Log.d(TAG, "The optional LVC Configuration is not included.");
        }
        return null;
    }

    public static boolean lvcEnabled() {
        try {
            Class.forName("com.amazon.alexaautoclientservice.lvc.LvcModuleFactory");
            return true;
        } catch (ClassNotFoundException e) {
            return false;
        }
    }

    public static double getVersionNumber(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "version");
            if (leafNodeValue instanceof String) {
                return Double.parseDouble((String) leafNodeValue);
            } else {
                Log.e(TAG, "Invalid value for version number");
            }
        } catch (JSONException e) {
            Log.e(TAG, "Exception encountered: " + e.getMessage());
        }
        return 0.0;
    }

    public static int getIPCCacheCapacity(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "ipc", "cacheCapacity");
            if (leafNodeValue instanceof Integer) {
                return (int) leafNodeValue;
            } else {
                Log.e(TAG, "invalid leaf node cacheCapacity, using default=20");
                return IPCConstants.DEFAULT_CACHE_CAPACITY;
            }
        } catch (JSONException e) {
            Log.w(TAG, "Could not find capacity from config");
        }
        return IPCConstants.DEFAULT_CACHE_CAPACITY;
    }

    public static void copyExternalFileToAACS(
            @NonNull Context context, @NonNull Uri fileUri, @NonNull String module, @NonNull String field) {
        // Create copy file in externalFiles directory
        File externalFilesDir = new File(context.getFilesDir(), "externalFiles");
        if (!externalFilesDir.exists())
            externalFilesDir.mkdir();

        File newModuleDir = externalFilesDir;
        if (!module.equals("")) {
            newModuleDir = new File(externalFilesDir, module);
            if (!newModuleDir.exists()) {
                newModuleDir.mkdir();
            }
        }

        File externalFileCopy = new File(newModuleDir, field);
        if (externalFileCopy.exists())
            externalFileCopy.delete();

        if (!externalFilesDir.exists() || !newModuleDir.exists() || externalFileCopy.exists()) {
            Log.e(TAG, "Error while setting up externalFiles directory.");
            return;
        }

        try {
            if (!externalFileCopy.createNewFile()) {
                Log.e(TAG, String.format("Failed to create new file %s for module %s.", field, module));
            }
        } catch (IOException e) {
            Log.e(TAG,
                    String.format("Error while creating new file %s for module %s. Error: %s", field, module,
                            e.getMessage()));
        }

        // Copy contents of file descriptor to copy file
        try {
            FileDescriptor fd = Objects.requireNonNull(context.getContentResolver().openFileDescriptor(fileUri, "r"))
                                        .getFileDescriptor();
            InputStream inputStream = new FileInputStream(fd);
            OutputStream outputStream = new FileOutputStream(externalFileCopy);
            byte[] buf = new byte[1024];
            int len;
            while ((len = inputStream.read(buf)) > 0) {
                outputStream.write(buf, 0, len);
            }
        } catch (FileNotFoundException e) {
            Log.w(TAG,
                    String.format("File associated with URI (%s) provided not found. Error: %s", fileUri.toString(),
                            e.getMessage()));
        } catch (IOException e) {
            Log.e(TAG,
                    String.format("Error while copying files locally for URI: %s. Error: %s", fileUri.toString(),
                            e.getMessage()));
        } catch (SecurityException e) {
            Log.e(TAG,
                    String.format("AACS does not have permissions to access URI (%s). Error: %s", fileUri.toString(),
                            e.getMessage()));
        }
    }

    public static void deregisterAASBPlatformInterface(
            @NonNull Context context, @NonNull String module, @NonNull String interfaceName) {
        JSONObject config = getSavedConfigurationForEngine(context);
        String aasbModule = "aasb." + module;
        try {
            Log.d(TAG, String.format("Deregistering %s.%s PI from AASB", module, interfaceName));
            JSONObject enabled = new JSONObject();
            enabled.put("enabled", false);
            JSONObject moduleJson = new JSONObject();
            if (config.has(aasbModule)) {
                moduleJson = config.getJSONObject(aasbModule);
            }
            moduleJson.put(interfaceName, enabled);
            config.put(aasbModule, moduleJson);
            updateEngineConfigurationInSharedPref(context, config);
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error while deregistering AASB PI. Error: %s", e.getMessage()));
        }
    }
}
