package com.amazon.alexa.auto.apps.common.util;

import java.util.HashMap;
import java.util.Locale;

/**
 * Class for Locale related utilities
 */
public class LocaleUtil {

    public static final String DEFAULT_DOMAIN = "amazon.com";
    private static HashMap<String, String> localeAmazonDomainMap;

    public static void init() {
        if (localeAmazonDomainMap == null) {
            localeAmazonDomainMap = new HashMap<>();
            localeAmazonDomainMap.put("en_US", DEFAULT_DOMAIN);
            localeAmazonDomainMap.put("en_GB", "amazon.co.uk");
            localeAmazonDomainMap.put("en_AU", "amazon.co.uk");
            localeAmazonDomainMap.put("en_CA", "amazon.co.uk");
            localeAmazonDomainMap.put("en_IN", "amazon.co.uk");
            localeAmazonDomainMap.put("de", "amazon.de");
            localeAmazonDomainMap.put("es_ES", "amazon.es");
            localeAmazonDomainMap.put("es_MX", "amazon.com.mx");
            localeAmazonDomainMap.put("it", "amazon.it");
            localeAmazonDomainMap.put("pt_BR", DEFAULT_DOMAIN);
            localeAmazonDomainMap.put("jp", "amazon.co.jp");
            localeAmazonDomainMap.put("fr_FR", "amazon.fr");
            localeAmazonDomainMap.put("fr_CA", "amazon.ca");
            localeAmazonDomainMap.put("hi_IN", "amazon.in");
        }
    }

    public static String getLocalizedDomain(Locale locale) {
        init();
        String domain = localeAmazonDomainMap.get(locale.toString());
        if (domain != null) {
            return domain;
        } else if (localeAmazonDomainMap.get(locale.getLanguage()) != null) {
            return localeAmazonDomainMap.get(locale.getLanguage());
        }
        return DEFAULT_DOMAIN;
    }
}
