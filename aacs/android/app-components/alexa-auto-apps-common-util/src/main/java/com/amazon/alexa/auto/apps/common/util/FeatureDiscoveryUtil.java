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
package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.aacsconstants.FeatureDiscoveryConstants.Domain;
import com.amazon.aacsconstants.FeatureDiscoveryConstants.EventType;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;

import org.json.JSONException;
import org.json.JSONStringer;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * Helper class that provides feature discovery utilities
 */
public class FeatureDiscoveryUtil {
    private static final String TAG = FeatureDiscoveryUtil.class.getSimpleName();
    public static final String FEATURE_DISCOVERY = "feature-discovery";
    public static final Integer DEFAULT_LIMIT = 5;
    public static final long GET_FEATURE_MIN_INTERVAL_IN_MS = 500;
    public static final Set<String> SUPPORTED_DOMAINS;
    static {
        SUPPORTED_DOMAINS = new HashSet<>();
        SUPPORTED_DOMAINS.add(Domain.GETTING_STARTED);
        SUPPORTED_DOMAINS.add(Domain.TALENTS);
        SUPPORTED_DOMAINS.add(Domain.ENTERTAINMENT);
        SUPPORTED_DOMAINS.add(Domain.COMMS);
        SUPPORTED_DOMAINS.add(Domain.WEATHER);
        SUPPORTED_DOMAINS.add(Domain.SMART_HOME);
        SUPPORTED_DOMAINS.add(Domain.NEWS);
        SUPPORTED_DOMAINS.add(Domain.NAVIGATION);
        SUPPORTED_DOMAINS.add(Domain.TRAFFIC);
        SUPPORTED_DOMAINS.add(Domain.SKILLS);
        SUPPORTED_DOMAINS.add(Domain.LISTS);
        SUPPORTED_DOMAINS.add(Domain.SHOPPING);
        SUPPORTED_DOMAINS.add(Domain.QUESTIONS_ANSWERS);
        SUPPORTED_DOMAINS.add(Domain.SPORTS);
        SUPPORTED_DOMAINS.add(Domain.CALENDAR);
    }

    /**
     * Create a tag for the feature discovery request from the locale, domain and eventType.
     *
     * @param locale The locale of the returned features. If the locale is a language
     *               combination, the primary language will be used in the tag. For example,
     *                if the locale is "en-US/es-ES", "en-US" will be used to create the tag.
     * @param domain The domain of the returned features. {@link Domain}
     * @param eventType The eventType of the returned features. {@link EventType}
     * @return String The tag for the returned features.
     */
    public static String createTag(String locale, String domain, String eventType) {
        try {
            Preconditions.checkNotNull(locale);
            Preconditions.checkNotNull(domain);
            Preconditions.checkNotNull(eventType);
            Preconditions.checkArgument(!locale.isEmpty());
            Preconditions.checkArgument(!domain.isEmpty());
            Preconditions.checkArgument(!eventType.isEmpty());

            String normalizedLocale = locale;
            if (locale.contains("/")) {
                normalizedLocale = locale.split("/")[0];
            }
            return String.format("%s#%s#%s", normalizedLocale, domain, eventType);
        } catch (Exception e) {
            Log.e(TAG, "failed to create tag. " + e.getMessage());
            return "";
        }
    }

    /**
     * Save the features in the shared preference by their tag.
     *
     * @param context The application context.
     * @param tag The tag of the feature discovery request. The tag can be created
     *            using {@link #createTag(String, String, String)} method.
     * @param features A set of utterance strings to save in the shared preference.
     */
    public static void saveFeaturesByTag(Context context, String tag, Set<String> features) {
        if (context == null) {
            Log.e(TAG, "context is null");
            return;
        }
        if (features == null || features.size() == 0) {
            Log.w(TAG, "No feature is saved. Tag: " + tag);
            return;
        }
        SharedPreferences.Editor editor = context.getSharedPreferences(FEATURE_DISCOVERY, 0).edit();
        editor.putStringSet(tag, features);
        editor.apply();
    }

    /**
     * Retrieve the features from the shared preference by their tag.
     *
     * @param context The application context.
     * @param tag The tag of the feature discovery request. The tag can be created
     *            using {@link #createTag(String, String, String)} method.
     * @return The set of utterance strings retrieved from the shared preference.
     */
    public static Set<String> getFeaturesByTag(Context context, String tag) {
        try {
            Preconditions.checkNotNull(context);
            Preconditions.checkNotNull(tag);
            SharedPreferences sharedPreferences = context.getSharedPreferences(FEATURE_DISCOVERY, 0);
            Set<String> featureSet = sharedPreferences.getStringSet(tag, null);
            if (featureSet == null) {
                throw new NullPointerException("No feature found for tag: " + tag);
            }
            return featureSet;
        } catch (Exception e) {
            Log.e(TAG, "failed to get features by tag. Error: " + e.getMessage());
            return new HashSet<>();
        }
    }

    /**
     * Clear all the discovered features in the shared preference.
     *
     * @param context The application context.
     */
    public static void clearDiscoveredFeatures(Context context) {
        try {
            Preconditions.checkNotNull(context);
            SharedPreferences.Editor editor = context.getSharedPreferences(FEATURE_DISCOVERY, 0).edit();
            editor.clear();
            editor.apply();
        } catch (Exception e) {
            Log.e(TAG, "Failed to clear cache. Error: " + e.getMessage());
        }
    }

    /**
     * Get the cached features and assign the text to the list of text views displayed in setup flow.
     *
     * @param textViews A list of {@link TextView} to be assigned with the retrieved features.
     * @param propertyManager The {@link AlexaPropertyManager} used to get the Alexa locale.
     * @param context The application context.
     */
    public static void setFeaturesInSetupFlow(
            List<TextView> textViews, AlexaPropertyManager propertyManager, Context context) {
        if (propertyManager == null) {
            Log.e(TAG, "propertyManager is null");
            return;
        }
        propertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                .filter(Optional::isPresent)
                .map(Optional::get)
                .subscribe(alexaLocale -> {
                    Set<String> featureSet = FeatureDiscoveryUtil.getFeaturesByTag(context,
                            FeatureDiscoveryUtil.createTag(alexaLocale,
                                    FeatureDiscoveryConstants.Domain.GETTING_STARTED,
                                    FeatureDiscoveryConstants.EventType.SETUP));
                    int i = 0;
                    for (String feature : featureSet) {
                        if (i == textViews.size())
                            break;
                        textViews.get(i).setText(feature);
                        i++;
                    }
                    Log.i(TAG, "Dynamic hints set for setup flow. Number of hints: " + i);
                });
    }

    /**
     * Construct and publish the GetFeatures message to the engine if the network is connected.
     *
     * @param context The application context.
     * @param requests The string of the feature discovery requests.
     */
    public static void checkNetworkAndPublishGetFeaturesMessage(Context context, String requests) {
        if (NetworkUtil.TYPE_NOT_CONNECTED == NetworkUtil.getConnectivityStatus(context)) {
            Log.e(TAG, "Network is not connected. Aborting the GetFeatures request.");
            return;
        }
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(FeatureDiscoveryConstants.DISCOVERY_REQUESTS)
                                     .value(requests)
                                     .endObject()
                                     .toString();
            new AACSMessageSender(new WeakReference<>(context), new AACSSender())
                    .sendMessage(Topic.FEATURE_DISCOVERY, Action.FeatureDiscovery.GET_FEATURES, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate GetFeatures message.");
        }
    }

    /**
     * Construct and publish the GetFeatures message to the engine if the network is connected.
     *
     * @param context The application context.
     * @param domains The set of domains of the requests.
     * @param eventType The eventType of the requests.
     */
    public static void checkNetworkAndPublishGetFeaturesMessage(
            Context context, Set<String> domains, String eventType) {
        try {
            JSONStringer requestStringer = new JSONStringer();
            requestStringer.array();
            for (String domain : domains) {
                requestStringer.object()
                        .key(FeatureDiscoveryConstants.DOMAIN)
                        .value(domain)
                        .key(FeatureDiscoveryConstants.EVENT_TYPE)
                        .value(eventType)
                        .key(FeatureDiscoveryConstants.LIMIT)
                        .value(DEFAULT_LIMIT)
                        .endObject();
            }
            requestStringer.endArray();
            checkNetworkAndPublishGetFeaturesMessage(context, requestStringer.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate discoveryRequests.");
        }
    }

    /**
     * Construct and publish the GetFeatures message to the engine if the network is connected.
     *
     * @param context The application context.
     * @param domain The domain of the single request.
     * @param eventType The eventType of the single request.
     */
    public static void checkNetworkAndPublishGetFeaturesMessage(Context context, String domain, String eventType) {
        checkNetworkAndPublishGetFeaturesMessage(context, new HashSet<>(Arrays.asList(domain)), eventType);
    }
}
