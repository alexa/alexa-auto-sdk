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

package com.amazon.aacstts;

import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.MissingResourceException;
import java.util.Optional;

/**
 * This is Utility class to vend out the mapping of IOS-639-2  to ISO2-639-1 language codes
 * and mapping of ISO 3166-1 alpha-2 to ISO 3166-1 alpha-3 country codes.
 *
 * Reference Code :
 * https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/speech/tts/TtsEngines.java#83
 */
public class ISO3CodeUtil {
    /**
     * This is a list of ISO language codes from:
     * https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
     */
    private static Map<String, String> ISO2_TO_ISO3_LANG_CODES;

    /**
     * This is a list of ISO country codes from:
     * https://en.wikipedia.org/wiki/ISO_3166-1
     */
    private static Map<String, String> ISO2_TO_ISO3_COUNTRY_CODES;

    // Populate the ISO2_TO_ISO3* maps
    static {
        HashMap<String, String> normalizeLanguage = new HashMap<String, String>();
        for (String language : Locale.getISOLanguages()) {
            try {
                normalizeLanguage.put(new Locale(language).getISO3Language(), language);
            } catch (MissingResourceException e) {
                continue;
            }
        }
        ISO2_TO_ISO3_LANG_CODES = Collections.unmodifiableMap(normalizeLanguage);

        HashMap<String, String> normalizeCountry = new HashMap<String, String>();
        for (String country : Locale.getISOCountries()) {
            try {
                normalizeCountry.put(new Locale("", country).getISO3Country(), country);
            } catch (MissingResourceException e) {
                continue;
            }
        }
        ISO2_TO_ISO3_COUNTRY_CODES = Collections.unmodifiableMap(normalizeCountry);
    }

    public static Optional<String> getISO2LangCode(String iso3LangCode) {
        if (ISO2_TO_ISO3_LANG_CODES.containsKey(iso3LangCode)) {
            return Optional.of(ISO2_TO_ISO3_LANG_CODES.get(iso3LangCode));
        }
        return Optional.empty();
    }

    public static Optional<String> getISO2CountryCode(String iso3CountryCode) {
        if (ISO2_TO_ISO3_COUNTRY_CODES.containsKey(iso3CountryCode)) {
            return Optional.of(ISO2_TO_ISO3_COUNTRY_CODES.get(iso3CountryCode));
        }
        return Optional.empty();
    }
}
