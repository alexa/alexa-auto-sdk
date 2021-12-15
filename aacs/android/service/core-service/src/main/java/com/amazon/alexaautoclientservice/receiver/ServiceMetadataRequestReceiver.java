/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.receiver;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.lang.ref.WeakReference;

public class ServiceMetadataRequestReceiver extends BroadcastReceiver {
    private final static String TAG = AACSConstants.AACS + "-" + ServiceMetadataRequestReceiver.class.getSimpleName();

    private static final String _AACS_PREFIX = "aacs-";
    private static final String EXTRAS_MODULE_FACTORY_JSON_EXT = ".json";
    private static final String EMPTY_STRING = "";
    private static final String ALEXAAUTOCLIENTSERVICE = "alexaautoclientservice";

    private String mPayload;

    public ServiceMetadataRequestReceiver(final WeakReference<Context> contextWeakReference) {
        super();
        AsyncTask.execute(() -> {
            JSONObject payload = new JSONObject();
            JSONObject metaData = new JSONObject();
            JSONArray extrasModuleList = new JSONArray();
            try {
                String[] fileList = contextWeakReference.get().getAssets().list(ALEXAAUTOCLIENTSERVICE);
                if (fileList == null) {
                    mPayload = EMPTY_STRING;
                    return;
                }
                for (String file : fileList) {
                    // Cleaning up file names to extract module name only
                    extrasModuleList.put(file.replace(_AACS_PREFIX, EMPTY_STRING)
                                                 .replace(EXTRAS_MODULE_FACTORY_JSON_EXT, EMPTY_STRING));
                }
                metaData.put(AACSConstants.ServiceMetadata.EXTRA_MODULE_LIST, extrasModuleList);
                payload.put(AACSConstants.ServiceMetadata.METADATA, metaData);
                mPayload = payload.toString();
            } catch (IOException e) {
                Log.e(TAG, String.format("Error in extra modules file parsing: %s", e.getMessage()));
            } catch (JSONException e) {
                e.printStackTrace();
            }
        });
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent reply = new Intent();

        reply.setAction(AACSConstants.IntentAction.GET_SERVICE_METADATA_REPLY);
        reply.putExtra(AACSConstants.PAYLOAD, mPayload);

        if (intent.hasExtra(AACSConstants.REPLY_TO_PACKAGE) && intent.hasExtra(AACSConstants.REPLY_TO_CLASS)) {
            String pkg = intent.getStringExtra(AACSConstants.REPLY_TO_PACKAGE);
            String cls = intent.getStringExtra(AACSConstants.REPLY_TO_CLASS);
            reply.setComponent(new ComponentName(pkg, cls));
            Log.i(TAG, String.format("Replying extras to package: %s class: %s", pkg, cls));
        }

        if (intent.hasExtra(AACSConstants.REPLY_TYPE)) {
            String type = intent.getStringExtra(AACSConstants.REPLY_TYPE);
            switch (type) {
                case "SERVICE":
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                        context.startForegroundService(reply);
                    } else {
                        context.startService(reply);
                    }
                    break;
                case "ACTIVITY":
                    context.startActivity(reply);
                    break;
                case "RECEIVER":
                    context.sendBroadcast(reply);
                    break;
            }
        } else {
            Log.w(TAG, "replyType is not mentioned.");
        }
    }
}
