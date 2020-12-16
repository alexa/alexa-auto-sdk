package com.amazon.alexa.auto.voiceinteraction.config

import android.content.Context
import androidx.preference.PreferenceManager
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_AACS_START_ON_BOOT
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_AVS_DEVICE_CLIENT_ID
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_AVS_DEVICE_DSN
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_AVS_DEVICE_MANUFACTURER
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_AVS_DEVICE_PRODUCT_ID
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_CONFIG_USE_AACS_AUDIO_INPUT
import com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.AACS_OVERRIDE_CONFIG
import org.json.JSONObject
import java.lang.ref.WeakReference

/**
 * Provides functions to sync the AACS Configuration from shared preferences
 * and vice-versa.
 */
class AACSConfigurationPreferences(val contextWk: WeakReference<Context>) {
    companion object {
        val TAG = AACSConfigurationPreferences::class.qualifiedName

        val AACS_ALEXA = "aacs.alexa"
        val AACS_ALEXA_DEVICE_INFO = "deviceInfo"
        val CLIENT_ID = "clientId"
        val PRODUCT_ID = "productId"
        val DEVICE_SERIAL_NUMBER = "deviceSerialNumber"
        val DEVICE_MANUFACTURER = "manufacturerName"
        val DEVICE_DESCRIPTION = "description"

        val AACS_GENERAL = "aacs.general"
        val AACS_START_ON_BOOT_ENABLED = "startServiceOnBootEnabled"

        val AACS_PLATFORM_HANDLERS = "aacs.defaultPlatformHandlers"
        val USE_DEFAULT_LOCATION_PROVIDER = "useDefaultLocationProvider"
        val USE_DEFAULT_NETWORK_INFO_PROVIDER = "useDefaultNetworkInfoProvider"
        val USE_DEFAULT_EXTERNAL_MEDIA_ADAPTER = "useDefaultExternalMediaAdapter"

        val AUDIO_INPUT = "audioInput"
        val AUDIO_TYPE = "audioType"
        val AUDIO_TYPE_VOICE = "VOICE"
        val USE_DEFAULT = "useDefault"
        val AUDIO_SOURCE = "audioSource"
        val EXTERNAL_SOURCE = "externalSource"
        val AUDIO_SOURCE_AACS_MICROPHONE = "MediaRecorder.AudioSource.MIC"
        val AUDIO_SOURCE_EXTERNAL = "EXTERNAL"

        val INTENT_HANDLER_TYPE = "type"
        val INTENT_HANDLER_TYPE_SERVICE = "SERVICE"
        val INTENT_HANDLER_PACKAGE = "package"
        val INTENT_HANDLER_PACKAGE_SELF = "com.amazon.alexa.auto.app"
        val INTENT_HANDLER_CLASS = "class"
        val INTENT_HANDLER_CLASS_AUDIO_IO = ".audio.AudioIOService"
    }

    private val mSharedPreference = PreferenceManager.getDefaultSharedPreferences(contextWk.get())

    /**
     * Updates the configuration with preferences that are explicitly set
     * by the user.
     *
     * @param config Configuration object that is updated from preferences.
     * @retyrb Updated JSON object.
     */
    fun updateConfigFromPreference(configJson: JSONObject): JSONObject {
        val overrideAACSConfig = mSharedPreference.getBoolean(AACS_OVERRIDE_CONFIG, false)
        if (overrideAACSConfig) {
            val aacsConfig = parseConfig(configJson)

            updateAVSDeviceConfig(aacsConfig)
            updateAACSGeneralConfig(aacsConfig)
            updatePlatformHandlersConfig(aacsConfig)

            saveConfigToJson(aacsConfig, configJson)
        }

        return configJson
    }

    /**
     * Updates the preference with config. Only those preferences are updated
     * which aren't explicitly set by the user.
     *
     * @param config Configuration JSON Object from where preferences are updated.
     */
    fun updatePreferenceFromConfig(config: JSONObject) {
        val aacsConfig = parseConfig(config)

        updateAVSDevicePreference(aacsConfig)
        updateAACSGeneralPreference(aacsConfig)
        updatePlatformHandlersPreference(aacsConfig)
    }

    private fun updateAVSDeviceConfig(config: AACSConfiguration) {
        if (mSharedPreference.contains(AACS_CONFIG_AVS_DEVICE_CLIENT_ID)) {
            config.deviceInfo.let {
                it.clientId = mSharedPreference.getString(AACS_CONFIG_AVS_DEVICE_CLIENT_ID, "")!!
                it.deviceSerialNumber =
                    mSharedPreference.getString(AACS_CONFIG_AVS_DEVICE_DSN, "")!!
                it.productId = mSharedPreference.getString(AACS_CONFIG_AVS_DEVICE_PRODUCT_ID, "")!!
                it.manufacturerName =
                    mSharedPreference.getString(AACS_CONFIG_AVS_DEVICE_MANUFACTURER, "")!!
            }
        }
    }

    private fun updateAVSDevicePreference(config: AACSConfiguration) {
        if (!mSharedPreference.contains(AACS_CONFIG_AVS_DEVICE_CLIENT_ID) ||
            mSharedPreference.getString(AACS_CONFIG_AVS_DEVICE_CLIENT_ID, "")?.isEmpty() == true) {
            val deviceInfo = config.deviceInfo
            mSharedPreference.edit()
                .putString(AACS_CONFIG_AVS_DEVICE_CLIENT_ID, deviceInfo.clientId)
                .putString(AACS_CONFIG_AVS_DEVICE_DSN, deviceInfo.deviceSerialNumber)
                .putString(AACS_CONFIG_AVS_DEVICE_PRODUCT_ID, deviceInfo.productId)
                .putString(AACS_CONFIG_AVS_DEVICE_MANUFACTURER, deviceInfo.manufacturerName)
                .apply()
        }
    }

    private fun updateAACSGeneralConfig(config: AACSConfiguration) {
        if (mSharedPreference.contains(AACS_CONFIG_AACS_START_ON_BOOT)) {
            config.generalConfig.startServiceOnBootEnabled =
                mSharedPreference.getBoolean(AACS_CONFIG_AACS_START_ON_BOOT, false)
        }
    }

    private fun updateAACSGeneralPreference(config: AACSConfiguration) {
        if (!mSharedPreference.contains(AACS_CONFIG_AACS_START_ON_BOOT)) {
            mSharedPreference.edit()
                .putBoolean(
                    AACS_CONFIG_AACS_START_ON_BOOT,
                    config.generalConfig.startServiceOnBootEnabled)
                .apply()
        }
    }

    private fun updatePlatformHandlersConfig(config: AACSConfiguration) {
        if (mSharedPreference.contains(AACS_CONFIG_USE_AACS_AUDIO_INPUT)) {
            val aacsAudioInput = mSharedPreference.getBoolean(
                AACS_CONFIG_USE_AACS_AUDIO_INPUT, true)

            config.platformHandlers.audioInput.voice.useExternalSource = !aacsAudioInput;
            if (aacsAudioInput) {
                config.platformHandlers.audioInput.voice.audioSource = AUDIO_SOURCE_AACS_MICROPHONE
                config.platformHandlers.audioInput.voice.externalSource = null
            } else {
                config.platformHandlers.audioInput.voice.audioSource = AUDIO_SOURCE_EXTERNAL
                config.platformHandlers.audioInput.voice.externalSource = makeAudioIOIntentHandler()
            }

        }
    }

    private fun updatePlatformHandlersPreference(config: AACSConfiguration) {
        if (!mSharedPreference.contains(AACS_CONFIG_USE_AACS_AUDIO_INPUT)) {
            mSharedPreference.edit()
                .putBoolean(
                    AACS_CONFIG_USE_AACS_AUDIO_INPUT,
                    !config.platformHandlers.audioInput.voice.useExternalSource)
                .apply();
        }
    }

    private fun saveConfigToJson(config: AACSConfiguration, configJson: JSONObject) {
        saveGeneralConfigToJson(config, configJson)
        saveDeviceInfoToJson(config, configJson)
        savePlatformHandlersToJson(config, configJson)
    }

    private fun parseConfig(json : JSONObject) =
        AACSConfiguration(
            parseGeneralConfig(json),
            parseDeviceInfo(json),
            parsePlatformHandlers(json)
        )

    private fun parseDeviceInfo(json: JSONObject) : AACSConfigurationDeviceInfo {
        val alexaObject = json.getJSONObject(AACS_ALEXA)
        val deviceInfo = alexaObject.getJSONObject(AACS_ALEXA_DEVICE_INFO)

        return AACSConfigurationDeviceInfo(
            deviceInfo.getString(CLIENT_ID),
            deviceInfo.getString(PRODUCT_ID),
            deviceInfo.getString(DEVICE_SERIAL_NUMBER),
            deviceInfo.getString(DEVICE_MANUFACTURER),
            deviceInfo.getString(DEVICE_DESCRIPTION)
        )
    }

    private fun saveDeviceInfoToJson(aacsConfig: AACSConfiguration, configJson: JSONObject) {
        val alexaObject = configJson.getJSONObject(AACS_ALEXA)
        val deviceInfoObj = alexaObject.getJSONObject(AACS_ALEXA_DEVICE_INFO)

        deviceInfoObj.put(CLIENT_ID, aacsConfig.deviceInfo.clientId)
            .put(PRODUCT_ID, aacsConfig.deviceInfo.productId)
            .put(DEVICE_SERIAL_NUMBER, aacsConfig.deviceInfo.deviceSerialNumber)
            .put(DEVICE_MANUFACTURER, aacsConfig.deviceInfo.manufacturerName)
            .put(DEVICE_DESCRIPTION, aacsConfig.deviceInfo.description);
    }

    private fun parseGeneralConfig(json: JSONObject) : AACSConfigurationGeneral {
        val generalObject = json.getJSONObject(AACS_GENERAL)

        return AACSConfigurationGeneral(
            generalObject.getBoolean(AACS_START_ON_BOOT_ENABLED)
        )
    }

    private fun saveGeneralConfigToJson(aacsConfig: AACSConfiguration, configJson: JSONObject) {
        val generalObject = configJson.getJSONObject(AACS_GENERAL)

        generalObject.put(
            AACS_START_ON_BOOT_ENABLED,
            aacsConfig.generalConfig.startServiceOnBootEnabled)
    }

    private fun parsePlatformHandlers(json: JSONObject): AACSConfigurationPlatformHandlers {
        val platformHandlersObject = json.getJSONObject(AACS_PLATFORM_HANDLERS)

        return AACSConfigurationPlatformHandlers(
            platformHandlersObject.getBoolean(USE_DEFAULT_LOCATION_PROVIDER),
            platformHandlersObject.getBoolean(USE_DEFAULT_NETWORK_INFO_PROVIDER),
            platformHandlersObject.getBoolean(USE_DEFAULT_EXTERNAL_MEDIA_ADAPTER),
            parseAudioInputTypes(platformHandlersObject)
        )
    }

    private fun savePlatformHandlersToJson(aacsConfig: AACSConfiguration, configJson: JSONObject) {
        val platformHandlersObject = configJson.getJSONObject(AACS_PLATFORM_HANDLERS)

        platformHandlersObject
            .put(
                USE_DEFAULT_LOCATION_PROVIDER,
                aacsConfig.platformHandlers.useDefaultLocationProvider)
            .put(
                USE_DEFAULT_NETWORK_INFO_PROVIDER,
                aacsConfig.platformHandlers.useDefaultNetworkInfoProvider)
            .put(
                USE_DEFAULT_EXTERNAL_MEDIA_ADAPTER,
                aacsConfig.platformHandlers.useDefaultExternalMediaAdapter);
        saveAudioInputTypesToJson(aacsConfig, platformHandlersObject)
    }

    private fun parseAudioInputTypes(platHandlersObj: JSONObject): AACSConfigurationAudioInputTypes {
        val audioInputObj = platHandlersObj.getJSONObject(AUDIO_INPUT)
        val audioTypeObj = audioInputObj.getJSONObject(AUDIO_TYPE)
        val voiceObj = audioTypeObj.getJSONObject(AUDIO_TYPE_VOICE)

        val useDefault = voiceObj.getBoolean(USE_DEFAULT)
        return AACSConfigurationAudioInputTypes(
            AACSConfigurationAudioInputType(
                useDefault,
                voiceObj.getString(AUDIO_SOURCE),
                if (useDefault) null else makeAudioIOIntentHandler()
            )
        )
    }

    private fun saveAudioInputTypesToJson(
            aacsConfig: AACSConfiguration, platHandlersObj: JSONObject) {
        val audioInputObj = platHandlersObj.getJSONObject(AUDIO_INPUT)
        val audioTypeObj = audioInputObj.getJSONObject(AUDIO_TYPE)
        val voiceObj = audioTypeObj.getJSONObject(AUDIO_TYPE_VOICE)

        voiceObj
            .put(USE_DEFAULT, true)
            .put(AUDIO_SOURCE, aacsConfig.platformHandlers.audioInput.voice.audioSource)

        voiceObj.remove(EXTERNAL_SOURCE)

        if (aacsConfig.platformHandlers.audioInput.voice.useExternalSource) {
            val audioIntentHandlerObj = JSONObject()
            audioIntentHandlerObj
                .put(INTENT_HANDLER_TYPE, INTENT_HANDLER_TYPE_SERVICE)
                .put(INTENT_HANDLER_PACKAGE, INTENT_HANDLER_PACKAGE_SELF)
                .put(INTENT_HANDLER_CLASS, INTENT_HANDLER_CLASS_AUDIO_IO)
            voiceObj.put(EXTERNAL_SOURCE, audioIntentHandlerObj)
        }
    }

    private fun makeAudioIOIntentHandler() =
        AACSConfigurationIntentHandler(
            INTENT_HANDLER_TYPE_SERVICE,
            INTENT_HANDLER_PACKAGE_SELF,
            INTENT_HANDLER_CLASS_AUDIO_IO
        )
}