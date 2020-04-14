/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_PROPERTY_ALEXA_PROPERTIES_H
#define AACE_ALEXA_PROPERTY_ALEXA_PROPERTIES_H

/** @file */

namespace aace {
namespace alexa {
namespace property {

/**
 * This read-only property is used with
 * aace::propertyManager::PropertyManager::getProperty() to check if the Engine
 * has wake word support. If wake word is not supported in the Engine, attempts
 * to enable wake word detection by the @c SpeechRecognizer will fail.
 */
static const std::string WAKEWORD_SUPPORTED = "aace.alexa.wakewordSupported";

/**
 * This property is used with
 * aace::propertyManager::PropertyManager::setProperty() to change the firmware
 * version that is reported to AVS. The value must be a positive, 32-bit signed
 * integer represented as a string.
 */
static const std::string FIRMWARE_VERSION = "aace.alexa.system.firmwareVersion";

/**
 * This property is used with
 * aace::propertyManager::PropertyManager::setProperty() to change the current
 * locale setting for AVS. The value must be a valid locale accepted by AVS.
 * Calling aace::propertyManager::PropertyManager::getProperty() with the
 * SUPPORTED_LOCALES property provides the list of supported locales.
 */
static const std::string LOCALE = "aace.alexa.setting.locale";

/**
 * This read-only property is used with
 * aace::propertyManager::PropertyManager::getProperty() to see all 
 * AVS-supported locales. The return value is a comma-separated list, 
 * e.g. "de-DE,en-AU,..."
 */
static const std::string SUPPORTED_LOCALES = "aace.alexa.supportedLocales";

/**
 * This read-only property is used with
 * aace::propertyManager::PropertyManager::getProperty() to check if the
 * vehicle's country is supported.
 */
static const std::string COUNTRY_SUPPORTED = "aace.alexa.countrySupported";

/**
 * This property is used with
 * aace::propertyManager::PropertyManager::setProperty() to change the current
 * wake word enabled setting. The value must be a boolean represented as a
 * string, i.e. "true" or "false". Call
 * aace::propertyManager::PropertyManager::getProperty() with the
 * WAKEWORD_ENABLED property to check whether wake word is enabled.
 */
static const std::string WAKEWORD_ENABLED = "aace.alexa.wakewordEnabled";

/**
 * This property is used with
 * aace::propertyManager::PropertyManager::setProperty() to change the current
 * timezone setting of the device. The value must be a valid timezone accepted
 * by AVS. Calling aace::propertyManager::PropertyManager::getProperty()
 * with the TIMEZONE property provides the current timezone of the device.
 */
static const std::string TIMEZONE = "aace.alexa.timezone";

} // aace::alexa::property
} // aace::alexa
} // aace

#endif // AACE_ALEXA_PROPERTY_ALEXA_PROPERTIES_H
