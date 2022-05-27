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
package com.amazon.aacsconstants;

import java.util.HashSet;
import java.util.Set;

public class FeatureDiscoveryConstants {
    // fields used in AASB FeatureDiscovery messages
    public static final String LOCALE = "locale";
    public static final String DOMAIN = "domain";
    public static final String EVENT_TYPE = "eventType";

    // constants used to construct GetFeatures message
    public static final String DISCOVERY_REQUESTS = "discoveryRequests";
    public static final String LIMIT = "limit";

    // constants used to construct GetFeaturesReply message
    public static final String DISCOVERY_RESPONSES = "discoveryResponses";
    public static final String LOCALIZED_CONTENT = "localizedContent";
    public static final String UTTERANCE_TEXT = "utteranceText";
    public static final String DESCRIPTION_TEXT = "descriptionText";

    public static class Domain {
        public static final String GETTING_STARTED = "GETTING_STARTED";
        public static final String TALENTS = "TALENTS";
        public static final String ENTERTAINMENT = "ENTERTAINMENT";
        public static final String COMMS = "COMMS";
        public static final String WEATHER = "WEATHER";
        public static final String SMART_HOME = "SMART_HOME";
        public static final String NEWS = "NEWS";
        public static final String NAVIGATION = "NAVIGATION";
        public static final String TRAFFIC = "TRAFFIC";
        public static final String SKILLS = "SKILLS";
        public static final String LISTS = "LISTS";
        public static final String SHOPPING = "SHOPPING";
        public static final String QUESTIONS_ANSWERS = "QUESTIONS_ANSWERS";
        public static final String SPORTS = "SPORTS";
        public static final String CALENDAR = "CALENDAR";
    }

    public static class EventType {
        public static final String THINGS_TO_TRY = "THINGS_TO_TRY";
        public static final String SETUP = "SETUP";
    }
}
