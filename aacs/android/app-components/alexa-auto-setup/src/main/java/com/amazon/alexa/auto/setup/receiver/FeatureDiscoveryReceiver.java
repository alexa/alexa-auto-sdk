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
package com.amazon.alexa.auto.setup.receiver;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.util.Log;

import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.HashSet;
import java.util.Set;

public class FeatureDiscoveryReceiver extends BroadcastReceiver {
    private static final String TAG = FeatureDiscoveryReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive.");
        if (intent == null || intent.getAction() == null) {
            Log.e(TAG, "intent received is not valid.");
            return;
        }
        final PendingResult result = goAsync();
        new FeatureProcessingTask(context, result, intent).execute();
    }

    private static class FeatureProcessingTask extends AsyncTask<Void, Void, Void> {
        private final WeakReference<Context> mContext;
        private final PendingResult mPendingResult;
        private final Intent mIntent;

        FeatureProcessingTask(Context context, PendingResult result, Intent intent) {
            mContext = new WeakReference<>(context);
            mPendingResult = result;
            mIntent = intent;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            AACSMessageBuilder.parseEmbeddedIntent(mIntent).ifPresent(message -> {
                try {
                    JSONObject payload = new JSONObject(message.payload);
                    String responsesString = payload.getString(FeatureDiscoveryConstants.DISCOVERY_RESPONSES);
                    JSONArray responsesArray = new JSONArray(responsesString);
                    for (int i = 0; i < responsesArray.length(); i++) {
                        try {
                            JSONObject response = responsesArray.getJSONObject(i);
                            String tag =
                                    FeatureDiscoveryUtil.createTag(response.getString(FeatureDiscoveryConstants.LOCALE),
                                            response.getString(FeatureDiscoveryConstants.DOMAIN),
                                            response.getString(FeatureDiscoveryConstants.EVENT_TYPE));
                            JSONArray featuresArray =
                                    response.getJSONArray(FeatureDiscoveryConstants.LOCALIZED_CONTENT);
                            Set<String> featuresSet = new HashSet<>();
                            for (int j = 0; j < featuresArray.length(); j++) {
                                JSONObject feature = featuresArray.getJSONObject(j);
                                String utterance = feature.getString(FeatureDiscoveryConstants.UTTERANCE_TEXT);
                                if (!utterance.isEmpty())
                                    featuresSet.add(String.format("\"%s\"", utterance));
                            }
                            FeatureDiscoveryUtil.saveFeaturesByTag(mContext.get(), tag, featuresSet);
                            Log.i(TAG,
                                    String.format("The size of features is %s. Saving to shared preference by tag %s.",
                                            featuresSet.size(), tag));
                        } catch (JSONException e) {
                            Log.e(TAG,
                                    "Failed to parse the element in the discoveryResponses JSON array. Error: "
                                            + e.getMessage());
                        }
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "GetFeaturesReply message cannot be parsed. Error: " + e.getMessage());
                }
            });
            mPendingResult.finish();
            return null;
        }
    }
}
