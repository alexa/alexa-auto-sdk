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
package com.amazon.alexa.auto.settings;

import static com.amazon.aacsconstants.AACSConstants.IntentAction.GET_SERVICE_METADATA_REPLY;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Receives intents broadcasted by AACS containing Service Metadata
 */
public class AACSMetadataReceiver extends BroadcastReceiver {
    private static final String TAG = AACSMetadataReceiver.class.getCanonicalName();
    public static final String AACS_MODULE_GEOLOCATION = "geolocation";
    public static final String AACS_MODULE_LVC = "lvc";

    /**
     * Scans through list of AACS extra modules in metadata object and adds modules
     * to the list of extra modules in shared preferences
     *
     * @param context
     * @param intent
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (intent == null || intent.getAction() == null) {
            return;
        }
        Log.d(TAG, intent.getAction());

        switch (intent.getAction()) {
            case GET_SERVICE_METADATA_REPLY:
                String payload = intent.getStringExtra(AACSConstants.PAYLOAD);
                if (payload == null || payload.isEmpty()) {
                    Log.w(TAG, "Received null list of extras");
                    return;
                }

                try {
                    JSONObject payloadJSON = new JSONObject(payload);
                    if (!payloadJSON.has(AACSConstants.ServiceMetadata.METADATA))
                        return;
                    JSONObject metaDataJSON = payloadJSON.getJSONObject(AACSConstants.ServiceMetadata.METADATA);
                    if (!metaDataJSON.has(AACSConstants.ServiceMetadata.EXTRA_MODULE_LIST))
                        return;
                    JSONArray extrasModuleListJSON =
                            metaDataJSON.getJSONArray(AACSConstants.ServiceMetadata.EXTRA_MODULE_LIST);
                    for (int i = 0; i < extrasModuleListJSON.length(); i++) {
                        if (AACS_MODULE_GEOLOCATION.equals(extrasModuleListJSON.get(i).toString())) {
                            ModuleProvider.addModule(context, ModuleProvider.ModuleName.GEOLOCATION.name());
                            Log.d(TAG, "Added AACS Geolocation module to list of extra modules");
                        }
                        if (AACS_MODULE_LVC.equals(extrasModuleListJSON.get(i).toString())) {
                            ModuleProvider.addModule(context, ModuleProvider.ModuleName.LVC.name());
                            Log.d(TAG, "Added AACS LVC module to list of extra modules");
                        }
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "Error while parsing aacs metadata reply intent" + e);
                }
                break;
            default:
                Log.d(TAG, "Intent action not handled. Action:" + intent.getAction());
        }
    }
}
