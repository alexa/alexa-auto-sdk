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
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.alexa.config.AlexaConfiguration;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;
import com.amazon.aace.storage.config.StorageConfiguration;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexaautoclientservice.constants.AudioSourceConstants;
import com.amazon.alexaautoclientservice.modules.mediaManager.MediaSource;

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
import java.util.List;
import java.util.Objects;

public class FileUtil {
    private static final String TAG = AACSConstants.AACS + "-" + FileUtil.class.getSimpleName();

    public static final String APPDATA_DIR = "appdata";
    public static final String CERTS_DIR = "certs";
    public static final String EXTERNAL_FILE_DIR = "externalFiles";
    public static final String MODEL_DIR = "aacs.amazonLite";

    public static final String CONFIG_KEY = "aacs-configuration";
    public static final String EXTRAS_CONFIG_KEY = "aacs-extras";

    public static final String AACS_GENERAL_CONFIG = "aacs.general";
    public static final String AACS_DEFAULT_PLATFORM_IMPL_CONFIG = "aacs.defaultPlatformHandlers";

    public static final String AACS_CONFIG_NETWORK_INFO_PROVIDER = "NetworkInfoProvider";
    public static final String AACS_CONFIG_LOCATION_PROVIDER = "LocationProvider";
    public static final String AACS_CONFIG_LOCAL_MEDIA_SOURCE = "LocalMediaSource";
    public static final String AACS_CONFIG_LOCAL_MEDIA_SOURCE_METADATA = "localMediaSourceMetadata";
    public static final String AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER = "ExternalMediaAdapter";
    public static final String AACS_CONFIG_PROPERTY_MANAGER = "PropertyManager";
    public static final String AACS_CONFIG_CUSTOM_DOMAIN_MESSAGE_DISPATCHER = "CustomDomainMessageDispatcher";

    public static final HashMap<String, String> mInterfaceToModuleNames = new HashMap<String, String>() {
        {
            put(AACS_CONFIG_NETWORK_INFO_PROVIDER, "network");
            put(AACS_CONFIG_LOCATION_PROVIDER, "location");
            put(AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER, "alexa");
            put(AACS_CONFIG_PROPERTY_MANAGER, "propertyManager");
            put(AACS_CONFIG_LOCAL_MEDIA_SOURCE, "alexa");
        }
    };

    private static JSONObject mEngineConfiguration = new JSONObject();
    private static JSONObject mGeneralConfiguration = new JSONObject();
    private static JSONObject mDefaultHandlerConfiguration = new JSONObject();
    private static JSONObject mExtrasConfiguration = new JSONObject();

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

        InputStream configStream =
                new ByteArrayInputStream(mEngineConfiguration.toString().getBytes(StandardCharsets.UTF_8));

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
                AlexaConfiguration.createDuckingConfig(getDuckingConfig()), StreamConfiguration.create(configStream)));
    }

    /**
     * Expecting following in the aacs.alexa
     * section of the config file
     * "audio" : {
     *   "audioOutputType.music": {
     *     "ducking": {
     *      "enabled" : true
     *    }
     *  }
     *}
     * @return ducking is enabled or not, default is false
     */
    private static boolean getDuckingConfig() {
        try {
            return mEngineConfiguration.getJSONObject("aace.alexa")
                    .getJSONObject("avsDeviceSDK")
                    .getJSONObject("audio")
                    .getJSONObject("audioOutputType.music")
                    .getJSONObject("ducking")
                    .getBoolean("enabled");
        } catch (JSONException exe) {
            Log.e(TAG, "Ducking configuration Error: " + exe.getMessage());
        }
        return false;
    }

    private static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
    }

    public static boolean isConfigurationSaved(@NonNull Context context) {
        return getSharedPreferences(context).contains(CONFIG_KEY);
    }

    public static void saveConfigToSharedPref(@NonNull Context context) {
        SharedPreferences.Editor sp = getSharedPreferences(context).edit();

        try {
            if (mEngineConfiguration == null) {
                Log.e(TAG, "Not saving shared preferences because engine configuration is null.");
                return;
            }
            JSONObject config = new JSONObject(mEngineConfiguration.toString());
            config.put(AACS_GENERAL_CONFIG, mGeneralConfiguration);
            config.put(AACS_DEFAULT_PLATFORM_IMPL_CONFIG, mDefaultHandlerConfiguration);

            sp.putString(CONFIG_KEY, config.toString()).apply();
            sp.putString(EXTRAS_CONFIG_KEY, mExtrasConfiguration.toString()).apply();
        } catch (JSONException e) {
            Log.e(TAG, "Error while saving config to SharedPref. Error: " + e.getMessage());
        }
    }

    public static void setConfiguration(@NonNull Context context) {
        Log.i(TAG, "Setting configuration using SharedPreferences.");
        try {
            JSONObject config = new JSONObject(getSharedPreferences(context).getString(CONFIG_KEY, ""));
            mDefaultHandlerConfiguration = (JSONObject) config.remove(AACS_DEFAULT_PLATFORM_IMPL_CONFIG);
            mGeneralConfiguration = (JSONObject) config.remove(AACS_GENERAL_CONFIG);
            mEngineConfiguration = config;

            mExtrasConfiguration = new JSONObject(getSharedPreferences(context).getString(EXTRAS_CONFIG_KEY, ""));
        } catch (JSONException e) {
            Log.e(TAG, "Setting config with SharedPref failed. Error: " + e.getMessage());
        }
    }

    public static boolean setConfiguration(
            @NonNull Context context, @NonNull JSONArray configFilepaths, @NonNull JSONArray configs) {
        Log.i(TAG, "Setting configuration using config message.");

        JSONObject fullConfig = constructFullConfiguration(configFilepaths, configs);
        fullConfig = translateToEngineConfiguration(context, fullConfig);

        if (fullConfig == null || !validateConfig(fullConfig))
            return false;

        mDefaultHandlerConfiguration = (JSONObject) fullConfig.remove(AACS_DEFAULT_PLATFORM_IMPL_CONFIG);
        mGeneralConfiguration = (JSONObject) fullConfig.remove(AACS_GENERAL_CONFIG);
        mEngineConfiguration = fullConfig;
        mExtrasConfiguration = new JSONObject();

        // Add config to deregister AASB platform interfaces that have default impl
        for (String pi : mInterfaceToModuleNames.keySet()) {
            if (isDefaultImplementationEnabled(pi)) {
                deregisterAASBPlatformInterface(mInterfaceToModuleNames.get(pi), pi);
            }
        }

        // Check the enablement for modules disabled by default
        checkEnablementForAASBModulesDisabledByDefault(mEngineConfiguration, "apl", "APL");
        checkEnablementForAASBModulesDisabledByDefault(mEngineConfiguration, "customDomain", "CustomDomain");
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
                    config, AACS_DEFAULT_PLATFORM_IMPL_CONFIG, "audioInput", "audioType", "VOICE", "audioSource");
            Object commsAudioSource = getLeafNodeValueFromJson(config, AACS_DEFAULT_PLATFORM_IMPL_CONFIG, "audioInput",
                    "audioType", "COMMUNICATION", "audioSource");

            Object voiceIsDefault = getLeafNodeValueFromJson(
                    config, AACS_DEFAULT_PLATFORM_IMPL_CONFIG, "audioInput", "audioType", "VOICE", "useDefault");
            Object commsIsDefault = getLeafNodeValueFromJson(config, AACS_DEFAULT_PLATFORM_IMPL_CONFIG, "audioInput",
                    "audioType", "COMMUNICATION", "useDefault");

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
                if (key.length() < 4) {
                    translatedConfig.put(key, config.getJSONObject(key));
                    continue;
                }
                if (key.substring(0, 4).equals("aacs")
                        && (!key.contains("modules") && !key.contains("general")
                                && !key.contains("defaultPlatformHandlers"))) {
                    if (key.contains("alexa")) {
                        JSONObject aacsAlexaConfig = config.getJSONObject("aacs.alexa");
                        populateDSN(context, aacsAlexaConfig);
                        JSONObject lmsConfig = (JSONObject) aacsAlexaConfig.remove("localMediaSource");
                        JSONObject mediaResumeConfig = (JSONObject) aacsAlexaConfig.remove("requestMediaPlayback");
                        JSONObject aasbAlexaConfig = new JSONObject();
                        aasbAlexaConfig.put("LocalMediaSource", lmsConfig);
                        translatedConfig.put("aasb.alexa", aasbAlexaConfig);
                        JSONObject aace_alexa = new JSONObject();
                        aace_alexa.put("avsDeviceSDK", aacsAlexaConfig);
                        if (mediaResumeConfig != null) {
                            aace_alexa.put("requestMediaPlayback", mediaResumeConfig);
                        }
                        translatedConfig.put("aace.alexa", aace_alexa);
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
                    JSONObject modulesConfig = config.getJSONObject(key);
                    Iterator<String> modulesIter = modulesConfig.keys();
                    while (modulesIter.hasNext()) {
                        String modulesKey = modulesIter.next();
                        if (modulesKey.length() >= 4 && modulesKey.substring(0, 4).equals("aacs")) {
                            translatedConfig.put(
                                    "aasb" + modulesKey.substring(4), modulesConfig.getJSONObject(modulesKey));
                        }
                    }
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

    private static void populateDSN(Context context, JSONObject aacsAlexaConfig) throws JSONException {
        if (aacsAlexaConfig.has("deviceInfo")) {
            JSONObject deviceInfo = aacsAlexaConfig.getJSONObject("deviceInfo");
            if (!deviceInfo.has("deviceSerialNumber") || "".equals(deviceInfo.optString("deviceSerialNumber"))) {
                String androidId = Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
                deviceInfo.put("deviceSerialNumber", androidId);
            }
        }
    }

    public static JSONObject removeExtrasModuleConfiguration(@NonNull Context context, @NonNull String configKey) {
        JSONObject engineConfig = mEngineConfiguration;
        JSONObject moduleConfig = new JSONObject();

        // Support both aacs and aace in config key
        if (configKey.substring(0, 4).equals("aacs")) {
            configKey = "aace" + configKey.substring(4);
        }

        // Check engine configuration first for module
        if (engineConfig.has(configKey)) {
            try {
                moduleConfig = (JSONObject) engineConfig.remove(configKey);
            } catch (Exception e) {
                Log.w(TAG, String.format("Failed to remove and retrieve config for %s", configKey));
            }

            // Save config for extras module factory
            try {
                mExtrasConfiguration.put(configKey, moduleConfig);
            } catch (JSONException e) {
                Log.w(TAG,
                        String.format(
                                "Error while constructing JSON object for extras config. Error: %s", e.getMessage()));
            }
        } else {
            // If not in engine configuration, check extras config
            try {
                if (mExtrasConfiguration.has(configKey))
                    moduleConfig = new JSONObject(mExtrasConfiguration.getString(configKey));
            } catch (JSONException e) {
                Log.w(TAG,
                        String.format("Error while getting JSON object for extras config. Error: %s", e.getMessage()));
            }
        }

        return moduleConfig;
    }

    /**
     * Checks boolean config fields in "aacs.general"
     * @param field
     * @return
     */
    public static boolean isEnabledInAACSGeneralConfig(@NonNull String field) {
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(mGeneralConfiguration, field);
            if (!(leafNodeValue instanceof Boolean)) {
                Log.w(TAG, String.format("Defaulting to %s=false, since the leaf node value was not valid", field));
                return false;
            }
            return (Boolean) leafNodeValue;
        } catch (Exception e) {
            Log.w(TAG, String.format("Defaulting to %s=false, since it was not specified in config.", field));
            return false;
        }
    }

    public static boolean isDefaultImplementationEnabled(@NonNull String platformInterfaceName) {
        try {
            Object leafNodeValue =
                    getLeafNodeValueFromJson(mDefaultHandlerConfiguration, "useDefault" + platformInterfaceName);
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

    public static boolean isAudioInputTypeEnabled(@NonNull String AudioInputType) {
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(
                    mDefaultHandlerConfiguration, "audioInput", "audioType", AudioInputType, "useDefault");
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

    public static boolean isAudioOutputTypeEnabled(@NonNull String AudioOutputType) {
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(
                    mDefaultHandlerConfiguration, "audioOutput", "audioType", AudioOutputType, "useDefault");
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

    public static int getAudioSourceForAudioType(@NonNull String audioType) {
        String audioSourceString = getAudioSourceStringForAudioType(audioType);
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

    public static boolean isAudioSourceExternal(@NonNull String audioType) {
        String audioSourceString = getAudioSourceStringForAudioType(audioType);
        return AudioSourceConstants.EXTERNAL.equals(audioSourceString);
    }

    private static String getAudioSourceStringForAudioType(@NonNull String audioType) {
        String audioSourceString = AudioSourceConstants.MIC;
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(
                    mDefaultHandlerConfiguration, "audioInput", "audioType", audioType, "audioSource");
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

    public static boolean handleAudioInputFocus(@NonNull String audioType) {
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(
                    mDefaultHandlerConfiguration, "audioInput", "audioType", audioType, "handleAudioFocus");
            if (leafNodeValue instanceof Boolean) {
                return (Boolean) leafNodeValue;
            } else {
                return false;
            }
        } catch (Exception e) {
            Log.w(TAG,
                    String.format("Defaulting isAudioInputFocusHandle false for %s, exception encountered: %s",
                            audioType, e.getMessage()));
            return false;
        }
    }

    public static JSONObject getAudioExternalSourceForAudioType(@NonNull String audioType) {
        try {
            Object leafNodeValue = getLeafNodeValueFromJson(
                    mDefaultHandlerConfiguration, "audioInput", "audioType", audioType, "externalSource");
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

    public static JSONArray getIntentTargets(@NonNull String topic, @NonNull String target) {
        Object leafNodeValue = getLeafNodeValueFromJson(mGeneralConfiguration, "intentTargets", topic, target);
        if (leafNodeValue instanceof JSONArray) {
            return (JSONArray) leafNodeValue;
        } else {
            Log.w(TAG,
                    String.format(
                            "Returning null due to invalid leaf node value retrieved from config for topic=%s, target=%s",
                            topic, target));
            return null;
        }
    }

    public static Object getLeafNodeValueFromJson(@NonNull JSONObject root, @NonNull String... jsonObjectTree) {
        if (root == null) {
            Log.w(TAG, "getLeafNodeValueFromJson: root is null.");
            return null;
        }
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

    public static String getLVCConfiguration() {
        String lvcModule = "aace.localVoiceControl";

        if (mEngineConfiguration.has(lvcModule)) {
            // Remove LVC config and store in SharedPref; Used for LVC apk, rather than Auto SDK
            JSONObject lvcConfig = (JSONObject) mEngineConfiguration.remove(lvcModule);
            try {
                mExtrasConfiguration.put(lvcModule, lvcConfig);
            } catch (JSONException e) {
                Log.w(TAG,
                        String.format(
                                "Error while constructing JSON object for extras config. Error: %s", e.getMessage()));
            }
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

    public static double getVersionNumber() {
        Object leafNodeValue = getLeafNodeValueFromJson(mGeneralConfiguration, "version");
        if (leafNodeValue instanceof String) {
            return Double.parseDouble((String) leafNodeValue);
        } else {
            Log.e(TAG, "Invalid value for version number");
        }
        return 0.0;
    }

    public static int getIPCCacheCapacity() {
        Object leafNodeValue = getLeafNodeValueFromJson(mGeneralConfiguration, "ipc", "cacheCapacity");
        if (leafNodeValue instanceof Integer) {
            return (int) leafNodeValue;
        } else {
            Log.e(TAG, "invalid leaf node cacheCapacity, using default=20");
            return IPCConstants.DEFAULT_CACHE_CAPACITY;
        }
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

    public static void deregisterAASBPlatformInterface(@NonNull String module, @NonNull String interfaceName) {
        String aasbModule = "aasb." + module;
        try {
            Log.d(TAG, String.format("Deregistering %s.%s PI from AASB", module, interfaceName));
            JSONObject enabled = new JSONObject();
            enabled.put("enabled", false);
            JSONObject moduleJson = new JSONObject();
            if (mEngineConfiguration.has(aasbModule)) {
                moduleJson = mEngineConfiguration.getJSONObject(aasbModule);
            }
            moduleJson.put(interfaceName, enabled);
            mEngineConfiguration.put(aasbModule, moduleJson);
        } catch (JSONException e) {
            Log.e(TAG, String.format("Error while de-registering AASB PI. Error: %s", e.getMessage()));
        }
    }

    private static void checkEnablementForAASBModulesDisabledByDefault(
            @NonNull JSONObject fullConfig, String module, String interfaceName) {
        try {
            Object isModuleEnabled = getLeafNodeValueFromJson(fullConfig, "aasb." + module, interfaceName, "enabled");
            if (isModuleEnabled instanceof Boolean) {
                if ((Boolean) isModuleEnabled) {
                    Log.i(TAG, String.format("AASB %s module is enabled, not de-registering the PI.", interfaceName));
                    return;
                }
            } else {
                Log.e(TAG,
                        String.format("invalid AASB module enablement configuration. %s will be disabled by default.",
                                interfaceName));
            }
            // Adding config for disabling AASB module by default
            deregisterAASBPlatformInterface(module, interfaceName);
        } catch (Exception e) {
            Log.e(TAG,
                    String.format("Error while checking AASB %s module configuration. Error: %s", interfaceName,
                            e.getMessage()));
        }
    }

    public static void cleanup() {
        mInterfaceToModuleNames.clear();
        mGeneralConfiguration = null;
        mEngineConfiguration = null;
        mDefaultHandlerConfiguration = null;
        mExtrasConfiguration = null;
    }

    /**
     * @return List of MediaSource objects populated from the config file
     */
    public static List<MediaSource> getLocalMediaSourceList() {
        final List<MediaSource> mLocalMediaSources = new ArrayList<>();
        try {
            Object leafNodeValue = mDefaultHandlerConfiguration.opt(AACS_CONFIG_LOCAL_MEDIA_SOURCE_METADATA);
            if (leafNodeValue == null) {
                Log.w(TAG,
                        String.format("getLeafNodeValueFromJson: key=%s did not exist in the JSONObject",
                                AACS_CONFIG_LOCAL_MEDIA_SOURCE));
                return mLocalMediaSources;
            }
            if (!(leafNodeValue instanceof JSONArray)) {
                Log.w(TAG, "Defaulting to empty list for \"sources\", since the leaf node value was not valid.");
                return mLocalMediaSources;
            }
            JSONArray list = (JSONArray) leafNodeValue;
            for (int i = 0; i < list.length(); i++) {
                JSONObject item = list.getJSONObject(i);
                boolean isSupported = item.optBoolean("supported", false);
                if (!isSupported)
                    continue;
                MediaSource source = new MediaSource(item.getString("sourceType"), item.getString("mediaPackageName"),
                        item.getString("mediaServiceClass"), item.optBoolean("supportsSetChannel", false),
                        item.optBoolean("supportsSetFrequency", false), item.optBoolean("supportsSetPreset", false));
                Log.v(TAG, "Adding local media source " + source.getSourceType());
                if (item.has("metadataTitleKey"))
                    source.putMetadataTitleKey(item.getString("metadataTitleKey"));
                if (item.has("metadataTrackIdKey"))
                    source.putMetadataTrackIdKey(item.getString("metadataTrackIdKey"));
                if (item.has("metadataTrackNumberKey"))
                    source.putMetadataTrackNumberKey(item.getString("metadataTrackNumberKey"));
                if (item.has("metadataArtistKey"))
                    source.putMetadataArtistKey(item.getString("metadataArtistKey"));
                if (item.has("metadataAlbumKey"))
                    source.putMetadataAlbumKey(item.getString("metadataAlbumKey"));
                if (item.has("metadataDurationKey"))
                    source.putMetadataDurationKey(item.getString("metadataDurationKey"));
                mLocalMediaSources.add(source);
            }
            return mLocalMediaSources;
        } catch (Exception e) {
            Log.e(TAG,
                    String.format(
                            "Defaulting to isDefaultImplementationEnabled=false for %s, since the configuration was not valid. Exception= %s",
                            AACS_CONFIG_LOCAL_MEDIA_SOURCE, e.getMessage()));
            return mLocalMediaSources;
        }
    }
}
