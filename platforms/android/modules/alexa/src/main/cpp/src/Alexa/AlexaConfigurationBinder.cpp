/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Alexa/AlexaConfigurationBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.config.AlexaConfigurationBinder";

// type aliases
using TemplateRuntimeTimeout = aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeout;
using TemplateRuntimeTimeoutType = aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType;
using EqualizerBand = aace::jni::alexa::EqualizerControllerHandler::EqualizerBand;

namespace aace {
namespace jni {
namespace alexa {

//
// JTemplateRuntimeTimeout
//

TemplateRuntimeTimeout JTemplateRuntimeTimeout::getTemplateRuntimeTimeout() {
    try_with_context {
        jobject timeoutTypeObj;
        ThrowIfNot(
            invoke(
                "getType",
                "()Lcom/amazon/aace/alexa/config/AlexaConfiguration$TemplateRuntimeTimeoutType;",
                &timeoutTypeObj),
            "getType:invokeMethodFailed");

        TemplateRuntimeTimeoutType checkedTimeoutTypeObj;
        ThrowIfNot(
            JTemplateRuntimeTimeoutType::checkType(timeoutTypeObj, &checkedTimeoutTypeObj), "invalidTimeoutType");

        jint checkedInt;
        ThrowIfNot(invoke("getValue", "()I", &checkedInt), "getValue:invokeMethodFailed");

        return {checkedTimeoutTypeObj, std::chrono::milliseconds(checkedInt)};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getTemplateRuntimeTimeout", ex.what());
        return {};
    }
}

std::vector<TemplateRuntimeTimeout> JTemplateRuntimeTimeout::convert(jobjectArray timeoutArrObj) {
    try_with_context {
        std::vector<TemplateRuntimeTimeout> runtimeTimeouts;
        JObjectArray arr(timeoutArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            runtimeTimeouts.push_back(aace::jni::alexa::JTemplateRuntimeTimeout(next).getTemplateRuntimeTimeout());
        }

        return runtimeTimeouts;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

// JNI
extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createDeviceInfoConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring deviceSerialNumber,
    jstring clientId,
    jstring productId,
    jstring manufacturerName,
    jstring description) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createDeviceInfoConfig(
            JString(deviceSerialNumber).toStdStr(),
            JString(clientId).toStdStr(),
            JString(productId).toStdStr(),
            JString(manufacturerName).toStdStr(),
            JString(description).toStdStr());
        ThrowIfNull(config, "createDeviceInfoConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfigurationBinder_createDeviceInfoConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAlertsConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createAlertsConfig(JString(databaseFilePath).toStdStr());
        ThrowIfNull(config, "createAlertsConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAlertsConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createNotificationsConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath) {
    try {
        auto config =
            aace::alexa::config::AlexaConfiguration::createNotificationsConfig(JString(databaseFilePath).toStdStr());
        ThrowIfNull(config, "createNotificationsConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createNotificationsConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCertifiedSenderConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath) {
    try {
        auto config =
            aace::alexa::config::AlexaConfiguration::createCertifiedSenderConfig(JString(databaseFilePath).toStdStr());
        ThrowIfNull(config, "createCertifiedSenderConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCertifiedSenderConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCapabilitiesDelegateConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createCapabilitiesDelegateConfig(
            JString(databaseFilePath).toStdStr());
        ThrowIfNull(config, "createCapabilitiesDelegateConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCapabilitiesDelegateConfigBinder",
            ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCurlConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring certsPath,
    jstring iface) {
    try {
        auto config = iface != nullptr
                          ? aace::alexa::config::AlexaConfiguration::createCurlConfig(
                                JString(certsPath).toStdStr(), JString(iface).toStdStr())
                          : aace::alexa::config::AlexaConfiguration::createCurlConfig(JString(certsPath).toStdStr());

        ThrowIfNull(config, "createCurlConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCurlConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createMiscStorageConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath) {
    try {
        auto config =
            aace::alexa::config::AlexaConfiguration::createMiscStorageConfig(JString(databaseFilePath).toStdStr());
        ThrowIfNull(config, "createMiscStorageConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createMiscStorageConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createDeviceSettingsConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring databaseFilePath,
    jobjectArray locales,
    jstring defaultLocale,
    jstring defaultTimezone,
    jobjectArray localeCombinations = {}) {
    try {
        std::vector<std::string> localesVector;
        std::vector<std::vector<std::string>> localeCombinationsVector;
        int localesSize = env->GetArrayLength(locales);
        int localeCombinationsSize = env->GetArrayLength(localeCombinations);
        jstring locale;

        for (int j = 0; j < localesSize; j++) {
            locale = (jstring)env->GetObjectArrayElement(locales, j);
            localesVector.push_back(JString(locale).toStdStr());
        }
        for (int i = 0; i < localeCombinationsSize; i++) {
            int localeCombinationSize =
                env->GetArrayLength((jobjectArray)env->GetObjectArrayElement(localeCombinations, i));
            std::vector<std::string> localeCombinationVector;
            for (int j = 0; j < localeCombinationSize; j++) {
                locale = (jstring)env->GetObjectArrayElement(
                    (jobjectArray)env->GetObjectArrayElement(localeCombinations, i), j);
                localeCombinationVector.push_back(JString(locale).toStdStr());
            }
            localeCombinationsVector.push_back(localeCombinationVector);
        }

        auto config = aace::alexa::config::AlexaConfiguration::createDeviceSettingsConfig(
            JString(databaseFilePath).toStdStr(),
            localesVector,
            JString(defaultLocale).toStdStr(),
            JString(defaultTimezone).toStdStr(),
            localeCombinationsVector);
        ThrowIfNull(config, "createDeviceSettingsConfigConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createDeviceSettingsConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeakerManagerConfigBinder(
    JNIEnv* env,
    jobject obj,
    jboolean enabled) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createSpeakerManagerConfig(enabled);
        ThrowIfNull(config, "createSpeakerManagerConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeakerManagerConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSystemConfigBinder(
    JNIEnv* env,
    jobject obj,
    jint firmwareVersion) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createSystemConfig(firmwareVersion);
        ThrowIfNull(config, "createSystemConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSystemConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeechRecognizerConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring encoderName) {
    try {
        auto config =
            aace::alexa::config::AlexaConfiguration::createSpeechRecognizerConfig(JString(encoderName).toStdStr());
        ThrowIfNull(config, "createSpeechRecognizerConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeechRecognizerConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createTemplateRuntimeTimeoutConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray timeoutList) {
    try {
        auto config = aace::alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig(
            aace::jni::alexa::JTemplateRuntimeTimeout::convert(timeoutList));
        ThrowIfNull(config, "createTemplateRuntimeConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createTemplateRuntimeTimeoutConfigBinder",
            ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createEqualizerControllerConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray supportedBands,
    jint minLevel,
    jint maxLevel,
    jobjectArray defaultBandLevels) {
    try {
        std::vector<EqualizerBand> equalizerBands;
        JObjectArray bandsObjArr(supportedBands);

        jobject bandObj;
        EqualizerBand band;

        for (int j = 0; j < bandsObjArr.size(); j++) {
            ThrowIfNot(bandsObjArr.getAt(j, &bandObj), "getArrayValueFailed");
            ThrowIfNot(aace::jni::alexa::JEqualizerBand::checkType(bandObj, &band), "invalidEqualizerBandType");

            equalizerBands.push_back(band);
        }
        auto config = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
            equalizerBands, minLevel, maxLevel, aace::jni::alexa::JEqualizeBandLevel::convert(defaultBandLevels));
        ThrowIfNull(config, "createEqualizerControllerConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createEqualizerControllerConfigBinder",
            ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createExternalMediaPlayerConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring agent) {
    try {
        auto config =
            aace::alexa::config::AlexaConfiguration::createExternalMediaPlayerConfig(JString(agent).toStdStr());
        ThrowIfNull(config, "createExternalMediaPlayerConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createExternalMediaPlayerConfigBinder",
            ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAuthProviderConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray providerNames) {
    try {
        std::vector<std::string> providerNamesVector;
        int providerNamesSize = env->GetArrayLength(providerNames);
        jstring providerName;

        for (int j = 0; j < providerNamesSize; j++) {
            providerName = (jstring)env->GetObjectArrayElement(providerNames, j);
            providerNamesVector.push_back(JString(providerName).toStdStr());
        }

        auto config = aace::alexa::config::AlexaConfiguration::createAuthProviderConfig(providerNamesVector);
        ThrowIfNull(config, "createAuthProviderConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAuthProviderConfigBinder", ex.what());
        return 0;
    }
}
}
