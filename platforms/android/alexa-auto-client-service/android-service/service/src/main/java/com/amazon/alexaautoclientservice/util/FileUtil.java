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
import android.support.annotation.NonNull;
import android.util.Log;

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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

public class FileUtil {
    private static final String TAG = AACSConstants.AACS + "-" + FileUtil.class.getSimpleName();

    private static final String APPDATA_DIR = "appdata";
    private static final String CERTS_DIR = "certs";
    public static final String CONFIG_KEY = "aacs-configuration";
    public static final String AACS_GENERAL_CONFIG = "aacs-general";

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

    private static boolean createDirectoriesAndCopyCertificates(
            File certsDir, File appDataDir, AssetManager assetManager) {
        if (!appDataDir.exists()) {
            appDataDir.mkdir();
        }

        if (!certsDir.exists()) {
            certsDir.mkdir();
        }

        if (!appDataDir.exists() || !certsDir.exists()) {
            Log.w(TAG, " appData or certsDir is missing");
            return false;
        }

        try {
            String[] certAssets = assetManager.list(CERTS_DIR);
            for (String next : certAssets) {
                if (copyFileFromAssetPath(CERTS_DIR + "/" + next, new File(certsDir, next), false, assetManager))
                    continue;
                return false;
            }
        } catch (IOException e) {
            Log.w(TAG, "Cannot copy certs to cache directory. Error: " + e.getMessage());
            return false;
        }
        return true;
    }

    public static ArrayList<EngineConfiguration> getEngineConfiguration(@NonNull Context context) {
        File certsDir = new File(context.getCacheDir(), CERTS_DIR);
        File appDataDir = new File(context.getFilesDir(), APPDATA_DIR);

        // Copy certificates and configuration onto the device location accessible to the layer which handles AACE
        if (!createDirectoriesAndCopyCertificates(certsDir, appDataDir, context.getAssets()))
            return null;

        // Construct EngineConfigurations list
        ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
                AlexaConfiguration.createCurlConfig(certsDir.getPath()),
                AlexaConfiguration.createMiscStorageConfig(appDataDir.getPath() + "/miscStorage.sqlite"),
                AlexaConfiguration.createCertifiedSenderConfig(appDataDir.getPath() + "/certifiedSender.sqlite"),
                AlexaConfiguration.createCapabilitiesDelegateConfig(
                        appDataDir.getPath() + "/capabilitiesDelegate.sqlite"),
                AlexaConfiguration.createAlertsConfig(appDataDir.getPath() + "/alerts.sqlite"),
                AlexaConfiguration.createNotificationsConfig(appDataDir.getPath() + "/notifications.sqlite"),
                AlexaConfiguration.createDeviceSettingsConfig(appDataDir.getPath() + "/deviceSettings.sqlite"),
                StorageConfiguration.createLocalStorageConfig(appDataDir.getPath() + "/localStorage.sqlite"),
                StreamConfiguration.create(getSavedConfigurationForEngine(context))));

        return configuration;
    }

    private static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
    }

    public static InputStream getSavedConfigurationForEngine(@NonNull Context context) {
        InputStream configStream = null;

        String config = getSharedPreferences(context).getString(CONFIG_KEY, "");

        try {
            JSONObject configJson = new JSONObject(config);
            configJson.remove("aacs.general");
            configJson.remove("aacs.defaultPlatformHandlers");
            configStream = new ByteArrayInputStream(configJson.toString().getBytes(StandardCharsets.UTF_8));
        } catch (JSONException e) {
            Log.e(TAG, "Error while constructing InputStream from stored configuration.");
        }
        return configStream;
    }

    public static boolean isConfigurationSaved(@NonNull Context context) {
        return getSharedPreferences(context).contains(CONFIG_KEY);
    }

    public static boolean saveConfiguration(
            @NonNull Context context, @NonNull JSONArray configFilepaths, @NonNull JSONArray configs) {
        Log.i(TAG, "Constructing configuration and saving into SharedPreferences.");

        JSONObject fullConfig = constructFullConfiguration(configFilepaths, configs);
        fullConfig = translateToEngineConfiguration(fullConfig);

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

    private static JSONObject translateToEngineConfiguration(JSONObject config) {
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

    public static String getCarControlAssetsPath(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "localVoiceControl", "carControlAssetsPath");
            if (!(leafNodeValue instanceof String)) {
                Log.w(TAG, "Defaulting to carControlAssets being empty, since the leaf node value was not valid");
                return "";
            }
            return (String) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG, "Defaulting to carControlAssets being empty, since it was not specified in config.");
            return "";
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
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            Object leafNodeValue = getLeafNodeValueFromJson(configJson, "localVoiceControl");
            if (leafNodeValue instanceof JSONObject) {
                return leafNodeValue.toString();
            } else {
                Log.e(TAG, "Invalid leaf node value config for localVoiceControl");
                return null;
            }
        } catch (JSONException e) {
            Log.w(TAG, "Could not find localVoiceControl in config file");
        }
        return null;
    }

    public static boolean lvcConfigurationAvailable(@NonNull Context context) {
        String config = getSharedPreferences(context).getString(AACS_GENERAL_CONFIG, "");
        try {
            JSONObject configJson = new JSONObject(config);
            return configJson.has("localVoiceControl");
        } catch (JSONException e) {
            Log.w(TAG, "Could not find localVoiceControl in config file");
        }
        return false;
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
}
