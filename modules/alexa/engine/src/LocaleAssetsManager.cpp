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

#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <Settings/SettingStringConversion.h>

#include "AACE/Engine/Alexa/LocaleAssetsManager.h"
#include "AACE/Engine/Core/EngineMacros.h"

/// String to identify log entries originating from this file.
static const std::string TAG("LocaleAssetsManager");

namespace aace {
namespace engine {
namespace alexa {

/// The key in our config file to find the root of settings for this database.
static const std::string SETTING_CONFIGURATION_ROOT_KEY = "deviceSettings";

/// The key to find the locales configuration.
static const std::string LOCALES_CONFIGURATION_KEY = "locales";

/// The key to find the locales configuration.
static const std::string DEFAULT_LOCALE_CONFIGURATION_KEY = "defaultLocale";

/// The key to find the locales combinations configuration.
static const std::string LOCALE_COMBINATION_CONFIGURATION_KEY = "localeCombinations";

/// The default locale value used if the locale configuration is not present.
static const std::string DEFAULT_LOCALE_VALUE = "en-US";

/// The default supported wake word.
static const std::string DEFAULT_SUPPORTED_WAKEWORD = "ALEXA";

std::shared_ptr<LocaleAssetsManager> LocaleAssetsManager::create(bool enableWakeWord) {
    auto manager = std::shared_ptr<LocaleAssetsManager>(new LocaleAssetsManager());
    if (!manager->initialize(enableWakeWord)) {
        return nullptr;
    }
    return manager;
}

bool LocaleAssetsManager::changeAssets(const Locales& locales, const WakeWords& wakeWords) {
    // Changing assets is not currently supported, however this needs to return true for AVS Device SDK
    //  to function correctly, as it will always call this during the initialization sequence.
    AACE_VERBOSE(LX(__func__)
                     .d("Locale", alexaClientSDK::settings::toSettingString<Locales>(locales).second)
                     .d("WakeWords", alexaClientSDK::settings::toSettingString<WakeWords>(wakeWords).second));
    return true;
}

void LocaleAssetsManager::cancelOngoingChange() {
    // No work is done by changeAssets.
}

bool LocaleAssetsManager::initialize(bool enableWakeWord) {
    auto settingsConfig =
        alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot()[SETTING_CONFIGURATION_ROOT_KEY];
    if (!settingsConfig) {
        AACE_ERROR(LX(TAG, "initializeFailed")
                       .d("reason", "configurationKeyNotFound")
                       .d("configurationKey", SETTING_CONFIGURATION_ROOT_KEY));
        return false;
    }

    if (!settingsConfig.getStringValues(LOCALES_CONFIGURATION_KEY, &m_supportedLocales)) {
        AACE_ERROR(LX(TAG, "initializeFailed")
                       .d("reason", "configurationKeyNotFound")
                       .d("configurationKey", LOCALES_CONFIGURATION_KEY));
        return false;
    }

    if (!settingsConfig.getString(DEFAULT_LOCALE_CONFIGURATION_KEY, &m_defaultLocale)) {
        AACE_ERROR(LX(TAG, "initializeFailed")
                       .d("reason", "configurationKeyNotFound")
                       .d("configurationKey", SETTING_CONFIGURATION_ROOT_KEY + "." + DEFAULT_LOCALE_CONFIGURATION_KEY));
        return false;
    }
    auto combinationArray = settingsConfig.getArray(LOCALE_COMBINATION_CONFIGURATION_KEY);
    if (combinationArray) {
        for (std::size_t arrayIndex = 0; arrayIndex < combinationArray.getArraySize(); ++arrayIndex) {
            auto stringVector =
                alexaClientSDK::avsCommon::utils::json::jsonUtils::retrieveStringArray<std::vector<std::string>>(
                    combinationArray[arrayIndex].serialize());
            // Make sure the combination is more than one locale.
            if (stringVector.size() <= 1) {
                AACE_ERROR(LX(TAG).d("reason", "LocaleCombinationSizeError").d("size", stringVector.size()));
                return false;
            }
            // Make sure all the locales in the combination are supported.
            for (const auto& locale : stringVector) {
                if (m_supportedLocales.count(locale) == 0) {
                    AACE_ERROR(LX(TAG).d("reason", "notSupportedLocalesInCombination").d("locale", locale));
                    return false;
                }
            }
            m_supportedLocalesCombinations.insert(stringVector);
        }
    }

    if (m_supportedLocales.empty()) {
        AACE_ERROR(LX(TAG, "initializeFailed").d("reason", "noSupportedLocalesInConfiguration"));
        return false;
    }

    if (m_defaultLocale.empty()) {
        AACE_ERROR(LX(TAG, "initializeFailed").d("reason", "noDefaultLocaleInConfiguration"));
        return false;
    }

    // Check if the default is in the supported locales
    if (m_supportedLocales.find(m_defaultLocale) == m_supportedLocales.end()) {
        AACE_ERROR(LX(TAG, "initializeFailed").d("reason", "defaultLocaleNotInSupportedLocaleList"));
        return false;
    }

    WakeWords wakeWords;
    if (enableWakeWord) {
        m_supportedWakeWords.insert({DEFAULT_SUPPORTED_WAKEWORD});
    } else {
        AACE_INFO(LX(TAG, __func__).d("supportedWakeWords", "NONE"));
    }

    return true;
}

LocaleAssetsManager::WakeWordsSets LocaleAssetsManager::getSupportedWakeWords(const Locale& locale) const {
    return m_supportedWakeWords;
}

LocaleAssetsManager::WakeWordsSets LocaleAssetsManager::getDefaultSupportedWakeWords() const {
    return m_supportedWakeWords;
}

std::map<LocaleAssetsManager::Locale, LocaleAssetsManager::WakeWordsSets> LocaleAssetsManager::
    getLocaleSpecificWakeWords() const {
    return std::map<LocaleAssetsManager::Locale, LocaleAssetsManager::WakeWordsSets>();
}

std::map<LocaleAssetsManager::LanguageTag, LocaleAssetsManager::WakeWordsSets> LocaleAssetsManager::
    getLanguageSpecificWakeWords() const {
    return std::map<LocaleAssetsManager::LanguageTag, LocaleAssetsManager::WakeWordsSets>();
}

std::set<LocaleAssetsManager::Locale> LocaleAssetsManager::getSupportedLocales() const {
    return m_supportedLocales;
}

LocaleAssetsManager::LocaleCombinations LocaleAssetsManager::getSupportedLocaleCombinations() const {
    return m_supportedLocalesCombinations;
}

LocaleAssetsManager::Locale LocaleAssetsManager::getDefaultLocale() const {
    return m_defaultLocale;
}

LocaleAssetsManager::LocaleAssetsManager() : m_defaultLocale{DEFAULT_LOCALE_VALUE} {
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
