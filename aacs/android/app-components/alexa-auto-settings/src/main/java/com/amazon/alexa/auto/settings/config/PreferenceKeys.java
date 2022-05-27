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

public class PreferenceKeys {
    public static final String AACS_OVERRIDE_CONFIG = "aacs-override-base-config";

    public static final String AACS_CONFIG_AVS_DEVICE_CLIENT_ID = "avs-device-clientid";
    public static final String AACS_CONFIG_AVS_DEVICE_PRODUCT_ID = "avs-device-productid";
    public static final String AACS_CONFIG_AVS_DEVICE_DSN = "avs-device-dsn";
    public static final String AACS_CONFIG_AVS_DEVICE_MANUFACTURER = "avs-device-manufacturer";

    public static final String AACS_CONFIG_AACS_START_ON_BOOT = "aacs-startonboot";

    public static final String AACS_CONFIG_USE_AACS_AUDIO_INPUT = "aacs-audio-input-internal";

    public static final String AACS_CONFIG_CONTROL_RESTART_APPLY_CONFIG = "aacs-reboot-apply-config";
    public static final String AACS_CONFIG_CONTROL_SHUTDOWN = "aacs-shutdown";
    public static final String AACS_CONFIG_CONTROL_START = "aacs-start";

    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA = "voice-assistance-nonalexa";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA = "voice-assistance-alexa";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_DISABLE_NONALEXA = "voice-assistance-disable-nonalexa";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_NONALEXA = "voice-assistance-enable-nonalexa";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_ENABLE_ALEXA = "voice-assistance-enable-alexa";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_PUSH_TO_TALK = "voice-assistance-push-to-talk";
    public static final String ALEXA_SETTINGS_VOICE_ASSISTANCE_PUSH_TO_TALK_SELECTION =
            "voice-assistance-push-to-talk-selection";
    public static final String ALEXA_SETTINGS_ACA_ADDRESSBOOK_CONSENT = "aca-addressbook-consent";
    public static final String ALEXA_SETTINGS_ADDRESSBOOK_CONSENT = "alexa-addressbook-consent";
    public static final String ALEXA_SETTINGS_HANDS_FREE = "alexa-hands-free-settings";
    public static final String ALEXA_SETTINGS_LOCATION_CONSENT = "alexa-location-consent-setting";
    public static final String ALEXA_SETTINGS_LANGUAGES = "alexa-languages-settings";
    public static final String ALEXA_SETTINGS_THINGS_TO_TRY = "alexa-things-to-try";
    public static final String ALEXA_SETTINGS_SIGNIN = "alexa-signin";
    public static final String ALEXA_SETTINGS_SIGNOUT = "alexa-signout";
    public static final String ALEXA_SETTINGS_DISABLE = "alexa-disable";
    public static final String ALEXA_SETTINGS_DISABLE_NON_ALEXA = "non-alexa-disable";
    public static final String ALEXA_SETTINGS_AACS = "alexa-auto-client-service-settings";
    public static final String ALEXA_SETTINGS_DO_NOT_DISTURB = "alexa-dnd-setting";
    public static final String ALEXA_SETTINGS_NAVI_FAVORITES = "alexa-nav_favorite-consent-setting";
    public static final String ALEXA_SETTINGS_COMMUNICATION = "alexa-communication";
    public static final String ALEXA_SETTINGS_SOUNDS = "alexa-sounds";

    public static final String ALEXA_SETTINGS_SOUND_START = "alexa-sound-start";
    public static final String ALEXA_SETTINGS_SOUND_END = "alexa-sound-end";

    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US = "en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_ES_US = "es-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_DE_DE = "de-DE";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_AU = "en-AU";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_CA = "en-CA";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_GB = "en-GB";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_IN = "en-IN";
    public static final String ALEXA_SETTINGS_LANGUAGES_ES_ES = "es-ES";
    public static final String ALEXA_SETTINGS_LANGUAGES_ES_MX = "es-MX";
    public static final String ALEXA_SETTINGS_LANGUAGES_FR_CA = "fr-CA";
    public static final String ALEXA_SETTINGS_LANGUAGES_FR_FR = "fr-FR";
    public static final String ALEXA_SETTINGS_LANGUAGES_HI_IN = "hi-IN";
    public static final String ALEXA_SETTINGS_LANGUAGES_IT_IT = "it-IT";
    public static final String ALEXA_SETTINGS_LANGUAGES_JA_JP = "ja-JP";
    public static final String ALEXA_SETTINGS_LANGUAGES_PT_BR = "pt-BR";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_ES_US = "en-US/es-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_ES_US_EN_US = "es-US/en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_CA_FR_CA = "en-CA/fr-CA";
    public static final String ALEXA_SETTINGS_LANGUAGES_FR_CA_EN_CA = "fr-CA/en-CA";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_IN_HI_IN = "en-IN/hi-IN";
    public static final String ALEXA_SETTINGS_LANGUAGES_HI_IN_EN_IN = "hi-IN/en-IN";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_FR_FR = "en-US/fr-FR";
    public static final String ALEXA_SETTINGS_LANGUAGES_FR_FR_EN_US = "fr-FR/en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_DE_DE = "en-US/de-DE";
    public static final String ALEXA_SETTINGS_LANGUAGES_DE_DE_EN_US = "de-DE/en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_JA_JP = "en-US/ja-JP";
    public static final String ALEXA_SETTINGS_LANGUAGES_JA_JP_EN_US = "ja-JP/en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_IT_IT = "en-US/it-IT";
    public static final String ALEXA_SETTINGS_LANGUAGES_IT_IT_EN_US = "it-IT/en-US";
    public static final String ALEXA_SETTINGS_LANGUAGES_EN_US_ES_ES = "en-US/es-ES";
    public static final String ALEXA_SETTINGS_LANGUAGES_ES_ES_EN_US = "es-ES/en-US";
}
