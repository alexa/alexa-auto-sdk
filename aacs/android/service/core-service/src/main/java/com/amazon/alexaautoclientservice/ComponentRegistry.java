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

package com.amazon.alexaautoclientservice;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.aacs.common.AACSComponentRegistryUtil;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ComponentRegistry {
    private static final String TAG = AACSConstants.AACS + "-" + ComponentRegistry.class.getSimpleName();

    private static final ComponentRegistry COMPONENT_REGISTRY_INSTANCE = new ComponentRegistry();

    private Map<String, List<TargetComponent>> mTargetCache;

    public static ComponentRegistry getInstance() {
        return COMPONENT_REGISTRY_INSTANCE;
    }

    private ComponentRegistry() {
        mTargetCache = new HashMap<>();
    }

    List<TargetComponent> findAASBMessageTargets(Context context, String topic, String action) {
        return findTargets(context, topic, action, IPCConstants.AASB_INTENT_PREFIX);
    }

    /**
     * Finds components (Service, Activity, Broadcast Receiver) that listen to the intent with
     * intentPrefix+shortCategory as the intent category and intentPrefix+shortAction as the intent action.
     *
     * @param context An Android context used to query the package manager.
     * @param shortCategory A short version of the intent category without the intent prefix.
     * @param shortAction A short version of the intent action without the intent prefix.
     * @param intentPrefix A prefix used to formalize and identify the type of the intent.
     * @return A list of targets as TargetComponents that listen to the intent.
     */
    public List<TargetComponent> findTargets(
            Context context, String shortCategory, String shortAction, String intentPrefix) {
        List<TargetComponent> targets;

        // Query config
        targets = queryConfig(context, shortCategory);
        if (targets != null) {
            return targets;
        }

        String cacheKey = shortCategory + "+" + shortAction;

        // Query mTargetCache
        Log.v(TAG,
                String.format(
                        "No target specified in config for shortCategory=%s, checking mTargetCache", shortCategory));
        targets = mTargetCache.get(cacheKey);
        if (targets != null) {
            Log.i(TAG,
                    String.format("Found targets=%s for shortCategory=%s shortAction=%s in mTargetcache", targets,
                            shortCategory, shortAction));
            return targets;
        }

        // Query package manager
        Log.v(TAG,
                String.format(
                        "No targets found in mTargetCache for shortCategory=%s shortAction=%s, querying package manager",
                        shortCategory, shortAction));
        final Intent queryIntent = new Intent(intentPrefix + shortAction);
        queryIntent.addCategory(intentPrefix + shortCategory);
        targets = AACSComponentRegistryUtil.queryPackageManager(context, queryIntent, AACSConstants.AACS_PERMISSION);
        if (targets == null) {
            Log.e(TAG,
                    String.format("No targets found for shortCategory=%s shortAction=%s ", shortCategory, shortAction));
            return null;
        }

        Log.v(TAG,
                String.format("Caching shortCategory=%s, shortAction=%s with targets=%s", shortCategory, shortAction,
                        targets));
        mTargetCache.put(cacheKey, targets);
        return targets;
    }

    private List<TargetComponent> queryConfig(Context context, String topic) {
        JSONArray packageNames = FileUtil.getIntentTargets(topic, "package");
        JSONArray classNames = FileUtil.getIntentTargets(topic, "class");
        JSONArray types = FileUtil.getIntentTargets(topic, "type");

        if (packageNames == null) {
            Log.e(TAG, String.format("Target package names for topic=%s is null", topic));
            return null;
        }

        if (classNames == null) {
            Log.e(TAG, String.format("Target class names for topic=%s is null", topic));
            return null;
        }

        if (types == null) {
            Log.e(TAG, String.format("Target types for topic=%s is null", topic));
            return null;
        }

        if (!(packageNames.length() == classNames.length() && packageNames.length() == types.length())) {
            Log.e(TAG,
                    String.format("Sizes of lists packageNames, classNames, types in config do not match for topic=%s",
                            topic));
            return null;
        }

        int size = packageNames.length();
        List<TargetComponent> targets = new ArrayList<>();

        for (int i = 0; i < size; i++) {
            TargetComponent target;
            try {
                String className = classNames.getString(i);
                if (className.charAt(0) == '.') {
                    className = packageNames.getString(i) + className;
                }
                target = TargetComponent.withComponent(
                        new ComponentName(packageNames.getString(i), className), getTargetType(types.getString(i)));
            } catch (JSONException ex) {
                Log.e(TAG, "Error reading JSONArray's from config");
                break;
            }
            if (AACSComponentRegistryUtil.checkPermission(context, AACSConstants.AACS_PERMISSION, target)) {
                Log.v(TAG,
                        String.format(
                                "Target for topic=%s found in config: %s", topic, target.component.getClassName()));
                targets.add(target);
            }
        }

        if (targets.size() > 0) {
            return targets;
        } else {
            Log.w(TAG,
                    String.format(
                            "None of the specified targets in config for topic=%s have the AACS permission", topic));
            return null;
        }
    }

    public static TargetComponent.Type getTargetType(String type) {
        switch (type) {
            case "ACTIVITY":
                return TargetComponent.Type.ACTIVITY;
            case "RECEIVER":
                return TargetComponent.Type.RECEIVER;
            case "SERVICE":
                return TargetComponent.Type.SERVICE;
            default:
                Log.w(TAG, String.format("Unknown target type: %s", type));
                return TargetComponent.Type.UNKNOWN;
        }
    }

    public void cleanUp() {
        if (mTargetCache != null) {
            mTargetCache.clear();
            Log.v(TAG, "Target Cache in Component Registry is cleaned");
        }
    }
}